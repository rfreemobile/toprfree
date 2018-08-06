#ifndef INCLUDE_rfreetop_ui_ncurses_stream
#define INCLUDE_rfreetop_ui_ncurses_stream

/**
 * @file Provide facility like std::cout, but for ncurses, and with attributed including colors - just for 1 window (main screen)
 * Idea from https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
 * Other idea, NOT USED HERE, was in https://stackoverflow.com/questions/20126649/ncurses-and-ostream-class-method
 **/

#include <streambuf>
#include <ostream>
#include <iostream>


class MyBuffer : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	MyBuffer();

protected:
	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	virtual int overflow(int c = Traits::eof());

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	virtual int sync(void);

private:
	// For EOF detection
	typedef std::char_traits<char> Traits;

	// Work in buffer mode. It is also possible to work without buffer.
	static const size_t BUF_SIZE = 64;
	char buf[BUF_SIZE];

	// This is the example userdata
	// ... data here ...

	// In this function, the characters are parsed.
	void putChars(const char* begin, const char* end);
};


/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
class MyOStream : public std::basic_ostream< char, std::char_traits< char > >
{

public:
	MyOStream();

private:
	MyBuffer buf;

};

#endif

