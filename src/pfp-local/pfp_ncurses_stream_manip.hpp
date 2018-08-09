

#ifndef INCLUDE_pfp_ncurses_stream_manip
#define INCLUDE_pfp_ncurses_stream_manip

#include <ostream>

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



#endif


