
/***
 * See header file for copyrights and notes.
*/


#include "ui/ncurses_stream.hpp"

#include <streambuf>
#include <ostream>
#include <iostream>
#include <cassert>
#include <cstddef>
#include <limits>

#include <ncurses.h>


cNcursesStreamBuf::cNcursesStreamBuf()
: m_refresh_on_sync(true)
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

// ===========================================================================================================

	cNcursesOStream::cNcursesOStream() :
	std::basic_ostream< char, std::char_traits< char > >(&buf),
	buf()
	{
	}

void cNcursesOStream::refresh_on_sync(bool enable) {
	buf.refresh_on_sync(enable);
}

void cNcursesOStream::apply(cNcursesManipCol attr) {
	attron( COLOR_PAIR(attr.m_pair)  ); // <--- ncurses
}


// ===========================================================================================================

std::ostream & operator<<(std::ostream & ostream , cNcursesManipCol attr) {
	cNcursesOStream & as_ncurses = dynamic_cast<cNcursesOStream &>( ostream );
	if (! as_ncurses) throw std::runtime_error("cNcursesManipCol attribute was sent to a stream that is not compatible with it.");
	as_ncurses.apply( attr );
	return ostream;
}


