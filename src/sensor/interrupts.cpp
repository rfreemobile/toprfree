
#include "sensor/interrupts.hpp"
#include "sensor/interrupts-api.hpp"

#include "mylib/string.hpp"
#include "mylib/memory.hpp"
#include "mylib/iostream.hpp"
#include "mylib/regex.hpp"

#include "mylib/string_trim.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <iomanip>
#include <numeric>

// ===========================================================================================================

cInterruptOptions::cInterruptOptions()
: m_showifsum{0}
{
}

// ===========================================================================================================

cSensorInterruptsError::cSensorInterruptsError(const string & err)
	: std::runtime_error(
		err.size() ?
			("Sensors error: " + err) :
			("Sensors error.")
	)
{ }

// ===========================================================================================================

unique_ptr<cSensorInterrupts> factory_cSensorInterrupts() {
	auto obj = make_unique<cSensorInterrupts>();
	return obj;
}

// ===========================================================================================================

cOneInterruptCounter::cOneInterruptCounter(std::vector<cOneInterruptCounter::t_count> && per_cpu_call)
:
m_per_cpu_call( std::move(per_cpu_call) )
,m_sum_call{0}
{
	recalc_sum();
}

void cOneInterruptCounter::recalc_sum() {
	m_sum_call = std::accumulate(m_per_cpu_call.begin(), m_per_cpu_call.end(), 0) ;
};

// ===========================================================================================================

cOneInterruptInfo::cOneInterruptInfo(const string & col0, const string & col1, const string & col2, vector<string> && devs)
: m_id_as_num(0), // will be overwritten possibly
m_standard( is_id_standard_and_parse(col0, this->m_id_as_num) ), // overwrite m_id_as_num, if ID is numerical
m_id(col0),
m_name1( !m_standard ? col1 : ""),
m_name2( !m_standard ? col2 : ""),
m_name(
	m_standard ?
	pfplib::ltrim_copy(col1) // for "NMI" the name is "Non-maskable interrupts"
	: (col1+" "+col2) // for ID e.g. 0 the name will be sum of collumns eg "IR-IO-APIC 2-edge timer"
)
,m_devs(devs)
,m_devs_str( make_dev_str() ) // using m_devs just set above
{ }

bool cOneInterruptInfo::is_id_standard_and_parse(const string & id_str, int & out_id_num) { ///< returns true for ID like "NMI", false for like "30"
	if (id_str.size()<=0) throw std::runtime_error("Empty ID of interrupt");
	std::istringstream iss(id_str);
	iss >> out_id_num;
	if (  (!iss.bad()) && (!iss.fail()) && (iss.eof()) ) return false; // OK, read exactly 1 number. Non-standard ID like "30".
	return true; // standard, named ID like "NMI"
}

bool cOneInterruptInfo::is_id_standard(const string & id_str) {
	int x;
	return is_id_standard_and_parse(id_str,x);
}

string cOneInterruptInfo::get_full_info() const {
	string ret = "name=" + m_name + " name1=" + m_name1 + " name2=" + m_name2+" " + m_devs_str;
	return ret;
}

string cOneInterruptInfo::make_dev_str() const {
	string ret;
	for (const string & dev : m_devs) ret += dev + ";";
	return ret;
}

// ===========================================================================================================


void cSensorInterrupts::calc_stats() {
	if (m_before_first) { // now doing first step, create the m_diff table
		// cerr << "calc: before first..." << endl;
		m_diff = m_current;
	}
	else {
		// cerr << "calc: normal" << endl;
		size_t size_inter = m_current.size();
		for (size_t i_inter=0; i_inter < size_inter; ++i_inter) {
			assert( m_current.at(i_inter).m_per_cpu_call.size() == this->m_num_cpu );
			for (size_t i_cpu=0; i_cpu < this->m_num_cpu; ++i_cpu) {
				auto & diff =     m_diff.at(i_inter).m_per_cpu_call.at(i_cpu);
				auto & curr =  m_current.at(i_inter).m_per_cpu_call.at(i_cpu);
				auto & prev = m_previous.at(i_inter).m_per_cpu_call.at(i_cpu);
				diff = curr - prev;
			} // all cpu-counter of interrupt
		} // all interrupt

		for (size_t i_inter=0; i_inter < size_inter; ++i_inter) m_diff.at(i_inter).recalc_sum();
	} // normal calc
}

void cSensorInterrupts::step() {
	// cerr << "Step... " << endl;
	m_previous = m_current;
	m_before_first = false;
}

void cSensorInterrupts::gather() {
	/*
	            CPU0       CPU1       CPU2       CPU3       CPU4       CPU5
	               0:         46          0          0          0          0          0  IR-IO-APIC    2-edge      timer
	*/
	bool dbg=0;
	// cerr << "Gathering... " << endl;

	std::ifstream thefile("/proc/interrupts");

	m_info.clear();
	m_current.clear();

	size_t line_nr=1; // line number, human-numbering (starting at 1)

	while (thefile.good()) {
		string line;
		std::getline( thefile , line );
		if (thefile.fail()) break; // done

		if (line_nr==1) { // parse number of CPUs
			int num=0;
			{
				std::istringstream iss(line);
				std::string cpu_name;
				while(!iss.eof()) {
					iss>>cpu_name;
					if (! iss.fail()) ++num; // fail marks the inability to read next word, so we're done
				}
			}
			m_num_cpu = num;
			if (m_num_cpu < 1) throw cSensorInterruptsError("Can not find any CPU in the interrupts list");
			if (dbg) cerr << "CPU count: " << m_num_cpu << endl;
			m_info.reserve(m_num_cpu);
			m_current.reserve(m_num_cpu);
		}
		else
		{ // non-header normal line
			// line = "   9:         1          2          3          4          5          42  name1    name-two      nic[3]";
			vector<string> devices;
			std::regex expr_name_id = make_regex_C("^[[:blank:]]*([[:alnum:]]+):"); // get the ID

			std::smatch matched_name_id;
			if (!std::regex_search(line, matched_name_id, expr_name_id)) throw cSensorInterruptsError("Can not parse (for ID)");
			if (! (matched_name_id.size() == 1+1)) throw cSensorInterruptsError("Got more then exactly 1 ID");
			std::string data_id = matched_name_id[1];

			if ( (data_id != "ERR") && (data_id != "MIS") ) { // ERR (and MIS) does not have normal counters
				vector<cOneInterruptCounter::t_count> counter_per_cpu;
				counter_per_cpu.reserve(m_num_cpu);
				assert( matched_name_id.ready() );
				const auto & part = matched_name_id.suffix().str();
				std::regex expr_next_count = make_regex_C("[[:blank:]]*([[:digit:]]+)"); // get the ID
				string str_after_id = part; // why is this needed? TODO  why can't this be const :< ?
				if (dbg) cerr << "str_after_id (in id="<<data_id<<") [" << str_after_id << "]" << endl;
				std::regex_iterator<std::string::iterator> rit(str_after_id.begin() , str_after_id.end(), expr_next_count,
					std::regex_constants::match_continuous
				);
				const decltype(rit) rit_end;
				decltype( rit->position() ) last_suffix_pos{0}; // position in str_after_id - of suffix after per-CPU
					// (position of 1st character of unmatched after reading this per-CPU counters)

				while (rit != rit_end) {
					if (! (rit->size() >= 1+1)) throw cSensorInterruptsError("Cant match counter text regex");
					cOneInterruptCounter::t_count count=0;
					const string & matched_str = (*rit)[1];
					std::istringstream iss( matched_str );
					iss>>count;
					counter_per_cpu.push_back(count);
					if (iss.fail()) throw cSensorInterruptsError("Cant read counter as integer");
					last_suffix_pos = str_after_id.size() - rit->suffix().length(); // TODO idiotic workaround
					// cerr << "matched=[" << (*rit)[1] << "] , now suffix pos=" << last_suffix_pos << " giving string: [" << str_after_id.substr(last_suffix_pos,-1) << "]" << endl;
					++rit;
				} // all CPUs
				// cerr << "read " << counter_per_cpu.size() << " CPU(s) counters." << endl;

				if (( last_suffix_pos >= 1 )) { // CPUs ok, now parsing names of this interrupt
					// assert that last_suffix_pos is < size of str_after_id
					assert(last_suffix_pos >= 0);
					size_t last_suffix_pos_unsigned{ static_cast<size_t>(last_suffix_pos) };
					if (!  ( last_suffix_pos_unsigned < str_after_id.size()   ) ) {
						std::ostringstream oss;
						oss << "Can't continue after per-CPU counters (too long position) "
							<< last_suffix_pos_unsigned << " should be < than " << str_after_id.size() << ".";
						throw cSensorInterruptsError(oss.str());
					}

					std::array<std::string , 2> names;
					if (cOneInterruptInfo::is_id_standard(data_id)) { // standard like NMI
						// rest of the line is the one big name
						names.at(0) = str_after_id.substr(last_suffix_pos , str_after_id.size());
					} // standard like NMI
					else
					{ // custom-interrupt: parse names and devices

					std::regex expr_next_name = make_regex_C("[[:blank:]]*([[:graph:]]+)"); // get the text
					std::regex_iterator<std::string::iterator> rit(str_after_id.begin()+last_suffix_pos , str_after_id.end() , expr_next_name,
						std::regex_constants::match_continuous
					);
					const decltype(rit) rit_end;
					size_t ix=0;
					while (rit != rit_end) {
						if (dbg) cerr << "parsing names...  ix=" << ix << endl;
						const string value = (*rit)[1].str();
						// cerr << "parsing names...  ix=" << ix << " is [" << value << "]" << endl;
						names.at(ix) = std::move(value);
						++rit;
						++ix;
						if ((ix == 2) && (rit->ready())) {
							// now parse the name(s) of device(s), like:
							// "eth1" or like "ehci_hcd:usb3, ehci_hcd:usb6, ehci_hcd:usb7"
							//cerr << "Suffix now is: [" << string{ (*rit)[1].first , (*rit)[1].second } << "]" << endl;
							//cerr << "Suffix now is: [" << rit->suffix() <<  "]" << endl;
							auto pos_dev_1 = (*rit)[1].first; // end of current name
							auto pos_dev_2 = str_after_id.end(); // end of string
							if (dbg) cerr << "Device names string: [" << string{ pos_dev_1, pos_dev_2  } << "]" << endl;

							// tokenize the device(s) name(s)
							std::regex expr_next_dev = make_regex_C("[[:blank:]]*([[:graph:]^,]+),{0,1}");
							std::regex_iterator<std::string::iterator> rit_dev( pos_dev_1, pos_dev_2 , expr_next_dev,
								std::regex_constants::match_continuous
						);
							while (rit_dev != rit_end) {
								if (dbg) {
									cerr << "dev match 0=" << (*rit_dev)[0] << endl;
									cerr << "dev match 1=" << (*rit_dev)[1] << endl;
								}
								devices.push_back( (*rit_dev)[1].str() );
								++rit_dev;
							}
							if (dbg) {
								cerr<<"Device(s) : ";
								for (const string & n : devices) cerr<<n<<";";
								cerr<<endl;
							}

							break;
						} // last index matched
					}
					} // custom interrupt

					if (dbg) cerr << "names: 0=[" << names.at(0) << "] 1=[" << names.at(1) << "]" << endl;
					cOneInterruptInfo one_info(data_id, names.at(0), names.at(1), std::move(devices)); // TODO
					m_info.push_back( std::move(one_info) );
					cOneInterruptCounter one_interrupt(std::move(counter_per_cpu));
					m_current.push_back( std::move(one_interrupt) );
				} // ok CPUs
			} // not-ERR
		} // non-header

		++line_nr;
	}

	if (line_nr<=1) throw cSensorInterruptsError("No interrupt info could be read.");
}

void cSensorInterrupts::print() const {
	cout << "CPU(s)=" << m_num_cpu << endl;
	size_t size_inter = m_info.size();
	size_t size_cpu = this->m_num_cpu;
	assert( m_info.size() == m_current.size() );

	if (m_before_first) {
		cout << "Can not show data yet (gathering more data to see speed)" << endl;
		return;
	}

	int wid_id = 5;
	int wid_sum = 5;
	int wid_cpu = wid_sum;

	cout << std::setw(wid_id) << "Inter" << " :" ;
	cout << std::setw(wid_sum) << "Sum" << ":" ;
	for (size_t ix_cpu=0; ix_cpu<size_cpu; ++ix_cpu) cout << std::setw(wid_cpu) << ix_cpu << "|";
	cout << "Device";
	cout << endl;

	cout << string(wid_id,'-') << " :" ;
	cout << string(wid_sum,'-') << ":" ;
	for (size_t ix_cpu=0; ix_cpu<size_cpu; ++ix_cpu) cout << string(wid_cpu,'-') << "|";
	cout << string(5,'-');
	cout << endl;

	size_t count_hidden{0};

	for (size_t ix_inter=0; ix_inter<size_inter; ++ix_inter) {
		const auto & diff = m_diff.at(ix_inter);
		const auto & info = m_info.at(ix_inter);

		bool show=true;
		if (diff.m_sum_call < m_options.m_showifsum) show=false;
		if (!show) {
			++count_hidden;
			continue;
		}

		cout << std::setw(wid_id) << info.m_id << " ";
		cout << ":";

		cout << std::setw(wid_sum) << diff.m_sum_call ;
		cout << ":" ;

		for (const auto & value : diff.m_per_cpu_call) {
			// 123
			// 1K
			// 123 K
			// 1M
			// 123 M
			// 5 char wide max (TODO)
			cout << std::setw(wid_cpu) << value ;
			cout << "|";
		}
		cout << " ";
		if (! info.m_standard) cout << info.m_devs_str ;
		else cout << "(" << info.m_name << ")";
		cout << endl;
	}
	if (count_hidden==0) {
		cout << "(All interrupts are displayed)" << endl;
	} else {
		cout << "(" << count_hidden << " interrupt(s) are hidden due to options)" << endl;
	}
	cout<<endl;
}


