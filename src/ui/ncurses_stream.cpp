
#include "ui/ncurses_stream.hpp"

#include <streambuf>
#include <ostream>
#include <iostream>
#include <cassert>
#include <cstddef>
#include <limits>

#include <ncurses.h>


/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
MyBuffer::MyBuffer()
	{
		setp(buf, buf + BUF_SIZE);
	}

int MyBuffer::overflow(int c)
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

	int MyBuffer::sync(void)
	{
		// Handle output
		putChars(pbase(), pptr());
		// This tells that buffer is empty again
		setp(buf, buf + BUF_SIZE);
		return 0;
	}

	void MyBuffer::putChars(const char* begin, const char* end) {
		if (begin==end) return;
		if ( ! ( std::less<const char*>()(begin,end) )) throw std::runtime_error("invalid buffer in putChars");
		std::size_t diff = end-begin; // not using std::ptrdiff_t , because size_t has wider range, and we know result will be positive
		assert(diff>=1); // double checking

		// will it fit into "int" for addnstr?
		if ( ! ( diff > std::numeric_limits<int>::max()  ) ) throw std::runtime_error("too long buffer in putChars");
		int len { static_cast<int>( diff ) };

		addnstr(begin, len);
		for (const char* c = begin; c < end; c++){
			std::cout << *c;
		}
	}

/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
	MyOStream::MyOStream() :
	std::basic_ostream< char, std::char_traits< char > >(&buf),
	buf()
	{
	}

