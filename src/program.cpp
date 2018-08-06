
#include "program.hpp"
#include "mylib/iostream.hpp"

#include "sensor/interrupts.hpp"

#include <boost/program_options.hpp>
#include <thread>
#include <chrono>

#include <ncurses.h>
#include <sstream>


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
		("showifsum", n_po::value<long int>()->default_value(1), "Show only interrupts with this or more events Sum.")
 	;

}

void cProgram::options(const int argc, const char * const * argv) {
	namespace n_po = boost::program_options;
	assert( m_pimpl->m_boostPO_desc );
	n_po::store(n_po::parse_command_line(argc, argv, *(m_pimpl->m_boostPO_desc)) , m_pimpl->m_argm);
}

void cProgram::early_startup() {
	initscr();
	bool has_col = has_colors();
	if (has_col) {
		start_color();
	}

	clear();

	noecho();
	cbreak();
	timeout(0);
	curs_set(0); // invisible

	for (short f=0; f<8; ++f) {
		for (short b=0; b<8; ++b) {
			init_pair(f+b*8,f,b);
		}
	}
	auto make_txt_col = [](short f, short b) -> short { return COLOR_PAIR( f + b*8 ); } ;

	long mmm=1000;
	for (int iii=0; iii<mmm; ++iii) {
		clear();
		float t = iii/100.0;
		float rr = 5 + iii/mmm*20;

		move(25 + sin(t*10)*rr/2, 40 + cos(t*10)*rr/2);
		addstr("X");

		move(25 + sin(t)*rr , 40 + cos(t)*rr);
		addstr("*rotfl");
		attron(A_BOLD);
		addstr("lol");
		attroff(A_BOLD);

		attron( make_txt_col(COLOR_RED, COLOR_BLUE) );
		addstr("foo");
		attroff( make_txt_col(COLOR_RED, COLOR_BLUE) );

		addstr("\n1\n2\n3.....press 'z' to exit.");

		std::ostringstream oss;
		oss << " colors=" << COLORS << " pairs=" << COLOR_PAIRS ;
		addstr(oss.str().c_str());

		string s(iii,'-');
		addstr(s.c_str());
		refresh();
		std::this_thread::sleep_for( std::chrono::milliseconds(10) );

		int keyb = getch();
		if (keyb == 'z') break;
	}

	endwin();

	throw std::runtime_error("test exit"); // XXX
}

void cProgram::run() {
	int sleep_time_ms = m_pimpl->m_argm["interval"].as<int>();
	int mainloops = m_pimpl->m_argm["mainloops"].as<int>();
	m_pimpl->m_sensor_interrupts->m_options.m_showifsum = m_pimpl->m_argm["showifsum"].as<long int>();

	cout << "Running the program. Interval time is: " << sleep_time_ms << "." << endl;

	bool exit_program=0;
	long int loop_counter=0;
	while (!exit_program) {
		++loop_counter;
		// cerr << "------- loop " << endl;

		m_pimpl->m_sensor_interrupts->gather();
		m_pimpl->m_sensor_interrupts->calc_stats();
		m_pimpl->m_sensor_interrupts->print();

		// cerr << "Sleep... " << endl;
		std::this_thread::sleep_for( std::chrono::milliseconds( sleep_time_ms  ));
		m_pimpl->m_sensor_interrupts->step();

		if ((mainloops!=0) && (loop_counter >= mainloops)) exit_program=1;
		int key=getch();
		if (key == 'z') exit_program=1;
	}
}

cProgram::~cProgram() { }

