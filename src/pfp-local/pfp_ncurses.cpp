
#include "pfp_ncurses.hpp"

#include <ncurses.h> // system's library
#include <cassert>

namespace nPfp_ncurses {

/// @namespace: color constants
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
	for (short f=0; f<max_color_fg(); ++f) {
		for (short b=0; b<max_color_bg(); ++b) {
			init_pair(f + b*max_color_bg(),f,b);
		}
	}
}

short cPairMaker::colors_to_pair(int fg, int bg) const {
	assert( (fg>=0) && (fg<=max_color_fg() ) );
	assert( (bg>=0) && (bg<=max_color_bg() ) );
	return fg + bg*max_color_bg();
}

int cPairMaker::max_color_fg() const noexcept {
	return 8;
}

int cPairMaker::max_color_bg() const noexcept {
	return 8;
}

} // namespace nPfp_ncurses



