
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

cOneInterruptCounter::cOneInterruptCounter(std::vector<int> && per_cpu_call)
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

	size_t line_nr=1; // line number, human-numbering (starting at 1)
	while (thefile.good()) {
		string line;
		std::getline( thefile , line );
		if (thefile.fail()) break; // done
		cout << "line [" << line << "]" << endl;

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
		}
		else
		{ // non-header normal line
			// line = "   9:         1          2          3          4          5          42  name1    name-two      nic[3]";
			cout << "line [" << line << "]" << endl;
			std::regex expr_name_id = make_regex_C("^[[:blank:]]*([[:alnum:]]+):"); // get the ID

			std::smatch matched_name_id;
			if (!std::regex_search(line, matched_name_id, expr_name_id)) throw cSensorInterruptsError("Can not parse (for ID)");
			if (! (matched_name_id.size() == 1+1)) throw cSensorInterruptsError("Got more then exactly 1 ID");
			std::string data_id = matched_name_id[1];

			if (data_id != "ERR") { // ERR does not have normal counters
				vector<int> counters;
				counters.reserve(m_num_cpu);
				assert( matched_name_id.ready() );
				{
					const auto & part = matched_name_id.suffix().str();
					std::regex expr_next_count = make_regex_C("[[:blank:]]*([[:digit:]]+)"); // get the ID
					string x = part; // why is this needed? TODO
					std::regex_iterator<std::string::iterator> rit(x.begin() , x.end(), expr_next_count,
						std::regex_constants::match_continuous
					);
					const decltype(rit) rit_end;
					while (rit != rit_end) {
						if (! (rit->size() >= 1+1)) throw cSensorInterruptsError("Cant match counter text regex");
						int count=0;
						std::istringstream iss( (*rit)[1] );
						iss>>count;
						if (iss.fail()) throw cSensorInterruptsError("Cant read counter as integer");
						cout << "got:" << count << endl;
						++rit;
					}
				} // parse counters
			} // not-ERR
		} // non-header

		++line_nr;
	}

	if (line_nr<=1) throw cSensorInterruptsError("No interrupt info could be read.");
}


