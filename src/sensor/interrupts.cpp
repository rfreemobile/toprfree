
#include "sensor/interrupts.hpp"
#include "sensor/interrupts.ipp"

#include "mylib/string.hpp"
#include "mylib/memory.hpp"
#include "mylib/iostream.hpp"
#include "mylib/regex.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <iomanip>

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
: m_per_cpu_call( std::move(per_cpu_call) )
{ };

// ===========================================================================================================

cOneInterruptInfo::cOneInterruptInfo(const string & col0, const string & col1, const string & col2, const string & col3)
: m_id_as_num(0), // will be overwritten possibly
m_standard( is_id_standard(col0, this->m_id_as_num) ), // overwrite m_id_as_num, if ID is numerical
m_id(col0),
m_name1(col1),
m_name2(col2),
m_name(
	m_standard ?
	col3 // for "NMI" the name is "Non-maskable interrupts"
	: (col1+" "+col2+" "+col3) // for ID e.g. 0 the name will be sum of collumns eg "IR-IO-APIC 2-edge timer"
)
{ }

bool cOneInterruptInfo::is_id_standard(const string & id_str, int & out_id_num) { ///< returns true for ID like "NMI", false for like "30"
	if (id_str.size()<=0) throw std::runtime_error("Empty ID of interrupt");
	std::istringstream iss(id_str);
	iss >> out_id_num;
	if (  (!iss.bad()) && (!iss.fail()) && (iss.eof()) ) return false; // OK, read exactly 1 number. Non-standard ID like "30".
	return true; // standard, named ID like "NMI"
}

// ===========================================================================================================

void cSensorInterrupts::gather() {
	/*
	            CPU0       CPU1       CPU2       CPU3       CPU4       CPU5
	               0:         46          0          0          0          0          0  IR-IO-APIC    2-edge      timer
	*/
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
			cout << "CPU count: " << m_num_cpu << endl;
			m_info.reserve(m_num_cpu);
			m_current.reserve(m_num_cpu);
		}
		else
		{ // non-header normal line
			// line = "   9:         1          2          3          4          5          42  name1    name-two      nic[3]";
			std::regex expr_name_id = make_regex_C("^[[:blank:]]*([[:alnum:]]+):"); // get the ID

			std::smatch matched_name_id;
			if (!std::regex_search(line, matched_name_id, expr_name_id)) throw cSensorInterruptsError("Can not parse (for ID)");
			if (! (matched_name_id.size() == 1+1)) throw cSensorInterruptsError("Got more then exactly 1 ID");
			std::string data_id = matched_name_id[1];

			if (data_id != "ERR") { // ERR does not have normal counters
				vector<cOneInterruptCounter::t_count> c_per_cpu;
				c_per_cpu.reserve(m_num_cpu);
				assert( matched_name_id.ready() );
				{ // non-ERR 2
					const auto & part = matched_name_id.suffix().str();
					std::regex expr_next_count = make_regex_C("[[:blank:]]*([[:digit:]]+)"); // get the ID
					string str_after_id = part; // why is this needed? TODO  why can't this be const :< ?
					// cout << "str_after_id (in id="<<data_id<<") [" << str_after_id << "]" << endl;
					std::regex_iterator<std::string::iterator> rit(str_after_id.begin() , str_after_id.end(), expr_next_count,
						std::regex_constants::match_continuous
					);
					const decltype(rit) rit_end;
					decltype( rit->position() ) last_suffix_pos{0}; // position in str_after_id - of suffix after per-CPU
						// (position of 1st character of unmatched after reading this per-CPU counters)

					while (rit != rit_end) {
						if (! (rit->size() >= 1+1)) throw cSensorInterruptsError("Cant match counter text regex");
						cOneInterruptCounter::t_count count=0;
						std::istringstream iss( (*rit)[1] );
						iss>>count;
						if (iss.fail()) throw cSensorInterruptsError("Cant read counter as integer");
						last_suffix_pos = rit->position(0); // update the last (so far) suffix
						++rit;
						c_per_cpu.push_back(count);
					} // all CPUs

					if (( last_suffix_pos >= 1 )) { // parsing per-CPU worked
						// assert that last_suffix_pos is < size of str_after_id
						assert(last_suffix_pos >= 0);
						size_t last_suffix_pos_unsigned{ static_cast<size_t>(last_suffix_pos) };
						if (!  ( last_suffix_pos_unsigned < str_after_id.size()   ) ) {
							std::ostringstream oss;
							oss << "Can't continue after per-CPU counters (too long position) "
								<< last_suffix_pos_unsigned << " should be < than " << str_after_id.size() << ".";
							throw cSensorInterruptsError(oss.str());
						}

						// kind of string-view, using str_after_id that must remain valid string
						//const char * part_names_char_start = & str_after_id.at(last_suffix_pos);
						//const char * part_names_char_end = & str_after_id.at(last_suffix_pos);

						cOneInterruptInfo one_info(data_id,"c1","c2","c3");
						m_info.push_back( std::move(one_info) );
						cOneInterruptCounter one_interrupt(std::move(c_per_cpu));
						m_current.push_back( std::move(one_interrupt) );

						//const auto & part = matched_name_id.suffix().str();
						//std::regex expr_next_name = make_regex_C("[[:blank:]]*([[:print:]]+)"); // get the text
						/*
						string x = part_names; // why is this needed? TODO
						std::regex_iterator<std::string::iterator> rit(x.begin() , x.end(), expr_next_count,
							std::regex_constants::match_continuous
						);
						const decltype(rit) rit_end;
						while (rit != rit_end) {
							++rit;
						*/

					} // per-CPU worked
				} // not-ERR2
			} // not-ERR
		} // non-header

		++line_nr;
	}

	if (line_nr<=1) throw cSensorInterruptsError("No interrupt info could be read.");
}

void cSensorInterrupts::print() const {
	cout << "CPU(s)=" << m_num_cpu << endl;
	size_t size_inter = m_info.size();
	assert( m_info.size() == m_current.size() );
	for (size_t ix_inter=0; ix_inter<size_inter; ++ix_inter) {
		cout << std::setw(4) << m_info.at(ix_inter).m_id << " ";
		cout << endl;
	}
}


