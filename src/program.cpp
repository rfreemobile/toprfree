
#include "program.hpp"
#include "mylib/iostream.hpp"

#include "sensor/interrupts.hpp"

#include <boost/program_options.hpp>
#include <thread>
#include <chrono>


class cProgram_pimpl {
	public:
		/// Boost program-options (for options from command-line and more)
		///@{
		boost::program_options::variables_map m_argm; ///< the arguments map (parsed, ready to use)
		shared_ptr<boost::program_options::options_description> m_boostPO_desc; ///< description/definition of the possible options, for parsing
		///@}

		shared_ptr<cSensorInterrupts> m_sensor_interrupts;
};

void cProgram_pimpl_delete::operator()(cProgram_pimpl *p) const { delete p; }


cProgram::cProgram()
: m_pimpl(new cProgram_pimpl)
{
	m_pimpl->m_sensor_interrupts = factory_cSensorInterrupts(); // might convert unique_ptr to shared_ptr here, ok

	namespace n_po = boost::program_options;
	unsigned line_length = 120;
 	m_pimpl->m_boostPO_desc = make_shared<n_po::options_description>("Options",line_length);

 	m_pimpl->m_boostPO_desc->add_options()
	 	("interval", n_po::value<int>()->default_value(2000), "Interval between displaying data, in msec.")
		("mainloops", n_po::value<int>()->default_value(5), "How many iterations of main loop to take. 0 = run forever")
 	;

}

void cProgram::options(const int argc, const char * const * argv) {
	namespace n_po = boost::program_options;
	assert( m_pimpl->m_boostPO_desc );
	n_po::store(n_po::parse_command_line(argc, argv, *(m_pimpl->m_boostPO_desc)) , m_pimpl->m_argm);
}

void cProgram::run() {
	int sleep_time_ms = m_pimpl->m_argm["interval"].as<int>();
	int mainloops = m_pimpl->m_argm["mainloops"].as<int>();
	cout << "Running the program. Interval time is: " << sleep_time_ms << "." << endl;

	bool exit_program=0;
	long int loop_counter=0;
	while (!exit_program) {
		++loop_counter;
		cerr << "------- loop " << endl;

		m_pimpl->m_sensor_interrupts->gather();
		m_pimpl->m_sensor_interrupts->calc_stats();
		m_pimpl->m_sensor_interrupts->print();

		cerr << "Sleep... " << endl;
		std::this_thread::sleep_for( std::chrono::milliseconds( sleep_time_ms  ));
		m_pimpl->m_sensor_interrupts->step();

		if ((mainloops!=0) && (loop_counter >= mainloops)) exit_program=1;
	}
}

cProgram::~cProgram() { }

