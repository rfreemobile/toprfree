
/***
 * See header file for copyrights and notes.
*/


#include "pfp-local/pfp_ncurses_stream.hpp"
#include "pfp-local/pfp_ncurses_stream_manip.hpp" // to have look at details that we need to implement our apply() of various manips

#include <streambuf>
#include <ostream>
#include <iostream>
#include <cassert>
#include <cstddef>
#include <limits>

#include <ncurses.h>

namespace nPfp_ncurses {


cNcursesStreamBuf::cNcursesStreamBuf(cPairMaker & pairMaker)
: m_refresh_on_sync(true) , m_pairMaker( pairMaker )
	{
		setp(buf, buf + BUF_SIZE);
	}

int cNcursesStreamBuf::overflow(int c)
	{
		// Handle output
		putChars(pbase(), pptr());
		if (c != Traits::eof()) {
			char c2 = c;
			// Handle the one character that didn't fit to buffer
			putChars(&c2, &c2 + 1);
		}
		// This tells that buffer is empty again
		setp(buf, buf + BUF_SIZE);
		return c;
	}

	int cNcursesStreamBuf::sync(void)
	{
		// Handle output
		putChars(pbase(), pptr());
		// This tells that buffer is empty again
		setp(buf, buf + BUF_SIZE);
		if (m_refresh_on_sync) {
			refresh(); /// <--- ncurses
		}
		return 0;
	}

	void cNcursesStreamBuf::putChars(const char* begin, const char* end) {
		if (begin==end) return;
		if ( ! ( std::less<const char*>()(begin,end) )) throw std::runtime_error("invalid buffer in putChars");
		std::size_t diff = end-begin; // not using std::ptrdiff_t , because size_t has wider range, and we know result will be positive
		assert(diff>=1); // double checking

		// will it fit into "int" for addnstr?
		if ( ! ( diff < std::numeric_limits<int>::max()  ) ) throw std::runtime_error("too long buffer in putChars");
		int len { static_cast<int>( diff ) };

		addnstr(begin, len); // <--- ncurses write
	}


void cNcursesStreamBuf::refresh_on_sync(bool enable) {
	m_refresh_on_sync = enable;
}

cPairMaker & cNcursesStreamBuf::get_pairMaker() {
	return m_pairMaker;
}

// ===========================================================================================================

	cNcursesOStream::cNcursesOStream(cPairMaker & pairMaker) :
	std::basic_ostream< char, std::char_traits< char > >(&buf),
	buf(pairMaker)
	,m_fg(-1), m_bg(-1)
	,m_fg_normal(nCol::white), m_bg_normal(nCol::black)
	{
	}

void cNcursesOStream::refresh_on_sync(bool enable) {
	buf.refresh_on_sync(enable);
}

void cNcursesOStream::define_color_normal(short fg, short bg) {
	m_fg_normal = fg;
	m_bg_normal = bg;
}

void cNcursesOStream::set_color(short fg, short bg) {
	bool changed{false};
	if (fg == -1) fg = m_fg; // will tell ncurses to use the same fg color that I have saved
	if (bg == -1) bg = m_bg; // will tell ncurses to use the same bg color that I have saved

	// in same way, interpret -2 as reset to normal color:
	if (fg == -2) fg = m_fg_normal;
	if (bg == -2) bg = m_bg_normal;

	if (fg != m_fg) changed=true;
	if (bg != m_bg) changed=true;

	if (changed) {
		auto color_bits = this->buf.get_pairMaker().colors_to_pair(fg,bg);
		(*this) << std::flush; // all that was buffered with previous attributes/colors, will be printed now
		attron( COLOR_PAIR( color_bits ) ); // <--- ncurses - set color, future buffer flushes will use this one
		m_fg = fg;
		m_bg = bg;
	}
}


} // namespace

