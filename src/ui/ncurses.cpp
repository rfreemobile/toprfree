
#include "ui/ncurses.hpp"

#include "ui/ncurses_stream.hpp"

#include <ncurses.h> // TODO remove this direct lib, use pfp_ wrapper

namespace nToprfree {

class cUiNcurses_impl {
	public:
		cNcursesOStream m_stream;
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

	for (short f=0; f<8; ++f) {
		for (short b=0; b<8; ++b) {
			init_pair(f+b*8,f,b);
		}
	}
	// auto make_txt_col = [](short f, short b) -> short { return COLOR_PAIR( f + b*8 ); } ;
	// bool has_col = has_colors();

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

