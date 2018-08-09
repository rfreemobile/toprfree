
#include "pfp_ncurses.hpp"

#include <ncurses.h> // system's library

namespace nPfp_ncurses {

namespace nCol {

const short black = COLOR_BLACK;
const short red = COLOR_RED;
const short green = COLOR_GREEN;
const short yellow = COLOR_YELLOW;
const short blue = COLOR_BLUE;
const short magenta = COLOR_MAGENTA;
const short cyan = COLOR_CYAN;
const short white = COLOR_WHITE;

} // namespace nCol


cPairMaker & cPairMaker::singleton() {
	static cPairMaker obj;
	return obj;
}

void cPairMaker::init_ncurses_colors() {
	for (short f=0; f<8; ++f) {
		for (short b=0; b<8; ++b) {
			init_pair(f + b*8,f,b);
		}
	}
}

short cPairMaker::colors_to_pair(int fg, int bg) const {
	return fg + bg*8;
}

} // namespace nPfp_ncurses



