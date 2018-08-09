
#include "ui/ncurses.hpp"

#include "pfp-local/pfp_ncurses_stream.hpp"
#include "pfp-local/pfp_ncurses_stream_manip.hpp"

#include <ncurses.h> // TODO remove this direct lib, use pfp_ wrapper

#include "pfp-local/pfp_ncurses.hpp"

namespace nToprfree {

class cUiNcurses_impl {
	public:
		cNcursesOStream m_stream;
		nPfp_ncurses::cPairMaker m_pair_maker;
};

void cUiNcurses_impl_deleter::operator()(cUiNcurses_impl *p) const { delete p; }

cUiNcurses::cUiNcurses()
	: m_impl( new cUiNcurses_impl )
{ }

void cUiNcurses::init() {
	std::cout << "Starting ncurses..." << std::endl;

	initscr();
	start_color();
	// bool has_col = has_colors();
	intrflush(nullptr,false); // do not break logical screen when control-character is used
	keypad(nullptr,true); // special keys

	noecho();
	cbreak();
	timeout(0);
	curs_set(0); // invisible cursor

	clear();

	m_impl->m_stream.refresh_on_sync(false);

	m_impl->m_pair_maker = nPfp_ncurses::cPairMaker::singleton();

	addstr("Ncurses started.\n");
	refresh();
}

void cUiNcurses::start_frame() {
	clear();
}

std::ostream& cUiNcurses::write() {
	return m_impl->m_stream;
}

void cUiNcurses::finish_frame() {
	refresh();
}

cUiNcurses::t_keyboard_key cUiNcurses::get_key() {
	return getch();
}


} // namespace

