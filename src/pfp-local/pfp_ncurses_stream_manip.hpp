

#ifndef INCLUDE_pfp_ncurses_stream_manip
#define INCLUDE_pfp_ncurses_stream_manip

#include <ostream>
#include "pfp-local/pfp_ncurses.hpp" // cPairMaker

namespace nPfp_ncurses {


class cNcursesManipCol;
// ncurses pair maker, that can produce manipulators to send to our ostream
class cPairMakerForManip : public cPairMaker {
	public:
		cNcursesManipCol manip(short fg, short bg) const; ///< create manipulator usning this function

		static cPairMakerForManip & singleton();
};

// ===========================================================================================================

class cNcursesManip {
	public:
		~cNcursesManip()=default;
};

class cNcursesManipCol : public cNcursesManip {
	public:
		~cNcursesManipCol()=default;

		short get_pair_shifted() const;

	protected:
		cNcursesManipCol(short color_pair_shifted);
		short m_pair_shifted; ///< the ncurses color-pair number, already bit-shifted via COLOR_PAIR(), ready to pass to attron()

		friend cNcursesManipCol cPairMakerForManip::manip(short fg, short bg) const; ///< create manipulator usning this function
};
std::ostream & operator<<(std::ostream & ostream , cNcursesManipCol attr);


} // namespace

#endif


