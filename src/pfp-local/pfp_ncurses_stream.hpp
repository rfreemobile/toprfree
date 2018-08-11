#ifndef INCLUDE_pfp_ncurses_stream
#define INCLUDE_pfp_ncurses_stream

/**
 * @file Provide facility like std::cout, but for ncurses, and with attributed including colors - just for 1 window (main screen)
 * Idea from https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
 * Other idea, NOT USED HERE, was in https://stackoverflow.com/questions/20126649/ncurses-and-ostream-class-method
 **/

#include <streambuf>
#include <ostream>
#include <iostream>

#include "mylib/memory.hpp"

#include "pfp-local/pfp_ncurses.hpp" // cPairMaker

namespace nPfp_ncurses {

class cNcursesStreamBuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	cNcursesStreamBuf(cPairMaker & pairMaker); ///< warning LIFETIME: the caller-provide pairMaker must remain valid in that address as long as (*this) object. Often done by cPairMaker::singleton()

	virtual void refresh_on_sync(bool enable); ///< set whether the sync() e.g. caused by endl, will result in ncurses refresh of screen

	cPairMaker & get_pairMaker();

protected:
	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	virtual int overflow(int c = Traits::eof());

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	virtual int sync(void);


	bool m_refresh_on_sync; /// see refresh_on_sync()

	cPairMaker & m_pairMaker; ///< LIFETIME: my creator provides this and it is responsible for keeping it alive (see my ctor)

private:
	// For EOF detection
	typedef std::char_traits<char> Traits;

	// Work in buffer mode. It is also possible to work without buffer.
	static const size_t BUF_SIZE = 16;
	char buf[BUF_SIZE];

	// This is the example userdata
	// ... data here ...

	// In this function, the characters are parsed.
	void putChars(const char* begin, const char* end);
};

class cNcursesOStream;

// ===========================================================================================================

// manipulators that the Stream knows about and can apply:
class cNcursesManipCol;

/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
class cNcursesOStream : public std::basic_ostream< char, std::char_traits< char > >
{

public:
	cNcursesOStream(cPairMaker & pairMaker); ///< warning LIFETIME: the caller-provide pairMaker must remain valid in that address as long as (*this) object. Often done by cPairMaker::singleton()

	virtual void refresh_on_sync(bool enable); ///< set whether the sync() e.g. caused by endl, will result in ncurses refresh of screen

	virtual void set_color(short fg, short bg);

private:
	cNcursesStreamBuf buf;

};

}


#endif

