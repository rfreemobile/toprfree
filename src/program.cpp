
#include "program.hpp"
#include "mylib/iostream.hpp"

#include "sensor/interrupts.hpp"

#include <boost/program_options.hpp>
#include <thread>
#include <chrono>

#include <ncurses.h>
#include <sstream>

#include "ui/base.hpp"
#include "ui/ncurses.hpp"

class cProgram_pimpl {
	public:
		/// Boost program-options (for options from command-line and more)
		///@{
		boost::program_options::variables_map m_argm; ///< the arguments map (parsed, ready to use)
		shared_ptr<boost::program_options::options_description> m_boostPO_desc; ///< description/definition of the possible options, for parsing
		///@}

		shared_ptr<nToprfree::cUiBase> m_ui;
		shared_ptr<cSensorInterrupts> m_sensor_interrupts;
};

void cProgram_pimpl_delete::operator()(cProgram_pimpl *p) const { delete p; }


cProgram::cProgram()
: m_pimpl(new cProgram_pimpl)
{
	m_pimpl->m_ui = make_shared< nToprfree::cUiNcurses >();
	m_pimpl->m_ui->init();

	m_pimpl->m_sensor_interrupts = factory_cSensorInterrupts(); // might convert unique_ptr to shared_ptr here, ok

	namespace n_po = boost::program_options;
	unsigned line_length = 120;
 	m_pimpl->m_boostPO_desc = make_shared<n_po::options_description>("Options",line_length);

 	m_pimpl->m_boostPO_desc->add_options()
	 	("interval", n_po::value<int>()->default_value(2000), "Interval between displaying data, in msec.")
		("mainloops", n_po::value<int>()->default_value(0), "How many iterations of main loop to take. 0 = run forever")
		("showifsum", n_po::value<long int>()->default_value(1), "Show only interrupts with this or more events Sum.")
 	;

}

void cProgram::options(const int argc, const char * const * argv) {
	namespace n_po = boost::program_options;
	assert( m_pimpl->m_boostPO_desc );
	n_po::store(n_po::parse_command_line(argc, argv, *(m_pimpl->m_boostPO_desc)) , m_pimpl->m_argm);
}

void cProgram::early_startup() {
	assert(m_pimpl->m_ui);
}

void cProgram::run() {
	long mainloop_time_ms = m_pimpl->m_argm["interval"].as<int>();
	long mainloops = m_pimpl->m_argm["mainloops"].as<int>();
	m_pimpl->m_sensor_interrupts->m_options.m_showifsum = m_pimpl->m_argm["showifsum"].as<long int>();

	(m_pimpl->m_ui->write()) << "Running the program. Interval time is: " << mainloop_time_ms << "." << endl;

	bool exit_program=0;
	long int loop_counter=0;
	while (!exit_program) {
		++loop_counter;

		m_pimpl->m_ui->start_frame();
		m_pimpl->m_sensor_interrupts->gather();
		m_pimpl->m_sensor_interrupts->calc_stats();
		m_pimpl->m_sensor_interrupts->print( m_pimpl->m_ui );
		m_pimpl->m_ui->finish_frame();
		m_pimpl->m_sensor_interrupts->step();

		int key=-1;
		{
			const long sleep_parts=10;
			for (long sleep_i=0; sleep_i<sleep_parts; ++sleep_i) {
				long sleep_now = mainloop_time_ms / sleep_parts;
				if (sleep_now < 1) sleep_now = 1;
				std::this_thread::sleep_for( std::chrono::milliseconds( sleep_now ) );
				key = m_pimpl->m_ui->get_key();
				if (key != -1) break;
			}
		}

		if ((mainloops!=0) && (loop_counter >= mainloops)) exit_program=1;
		if (key == 'z') exit_program=1;
	}
}

cProgram::~cProgram() { }

