
#include "ui/ncurses.hpp"

#include "ui/ncurses_stream.hpp"

#include <ncurses.h>

namespace nToprfree {

class cUiNcurses_impl {
	public:
		MyOStream m_stream;
};

void cUiNcurses_impl_deleter::operator()(cUiNcurses_impl *p) const { delete p; }

void cUiNcurses::init() {
	initscr();
	start_color();
	// bool has_col = has_colors();

	clear();

	noecho();
	cbreak();
	timeout(0);
	curs_set(0); // invisible cursor

	for (short f=0; f<8; ++f) {
		for (short b=0; b<8; ++b) {
			init_pair(f+b*8,f,b);
		}
	}
	// auto make_txt_col = [](short f, short b) -> short { return COLOR_PAIR( f + b*8 ); } ;
	// bool has_col = has_colors();
}

void cUiNcurses::start_frame() {
	clear();
}

std::ostream& cUiNcurses::write() {
	throw std::runtime_error("not implemented");
}

void cUiNcurses::finish_frame() {
	refresh();
}

cUiNcurses::t_keyboard_key cUiNcurses::get_key() {
	return getch();
}


} // namespace

