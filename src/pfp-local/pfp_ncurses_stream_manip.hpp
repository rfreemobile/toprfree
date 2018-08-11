

#ifndef INCLUDE_pfp_ncurses_stream_manip
#define INCLUDE_pfp_ncurses_stream_manip

#include <ostream>
#include "pfp-local/pfp_ncurses.hpp" // cPairMaker

#include "pfp-local/pfp_ncurses_stream.hpp"

namespace nPfp_ncurses {

struct setcolor {
	explicit constexpr setcolor(short fg, short bg) : m_fg(fg), m_bg(bg) { }
	short m_fg, m_bg;
};

struct resetcol : setcolor {
	explicit constexpr resetcol() : setcolor(-2,-2) { }
};

struct setfg : setcolor {
	explicit constexpr setfg(short fg) : setcolor(fg,-1) { }
};

struct setbg : setcolor {
	explicit constexpr setbg(short bg) : setcolor(-1,bg) { }
};

struct resetfg : setfg {
	explicit constexpr resetfg() : setfg(-2) { }
};
struct resetbg : setbg {
	explicit constexpr resetbg() : setbg(-2) { }
};


template <typename CharT, typename Traits>
std::basic_ostream<CharT,Traits> & operator<<( std::basic_ostream<CharT,Traits> & os, const setcolor & col) {
	cNcursesOStream & stream_as_ncurses = dynamic_cast<cNcursesOStream &>( os );
	if (! stream_as_ncurses) throw std::runtime_error("cNcursesManipCol attribute was sent to a stream that is not compatible with it.");
	stream_as_ncurses.set_color(col.m_fg, col.m_bg);
	return os;
}


} // namespace

#endif


