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

class cNcursesStreamBuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	cNcursesStreamBuf();

	virtual void refresh_on_sync(bool enable); ///< set whether the sync() e.g. caused by endl, will result in ncurses refresh of screen

protected:
	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	virtual int overflow(int c = Traits::eof());

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	virtual int sync(void);


	bool m_refresh_on_sync; /// see refresh_on_sync()

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

class cNcursesManip {
	public:
		~cNcursesManip()=default;
};

class cNcursesManipCol : public cNcursesManip {
	public:
		~cNcursesManipCol()=default;
		cNcursesManipCol(short m_fg_, short m_bg_);
		short m_pair; ///< the ncurses color-pair number
};
std::ostream & operator<<(std::ostream & ostream , cNcursesManipCol attr);

/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
class cNcursesOStream : public std::basic_ostream< char, std::char_traits< char > >
{

public:
	cNcursesOStream();

	virtual void refresh_on_sync(bool enable); ///< set whether the sync() e.g. caused by endl, will result in ncurses refresh of screen

	virtual void apply(cNcursesManipCol attr);

private:
	cNcursesStreamBuf buf;

};



#endif

