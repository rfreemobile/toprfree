
#include "pfp-local/pfp_ncurses_stream_manip.hpp"

#include <ncurses.h> // system's

// ===========================================================================================================

cNcursesManipCol::cNcursesManipCol(short m_fg_, short m_bg_)
: m_pair( COLOR_PAIR(m_fg_ , m_bg_)
{ }

// -------------------------------------------------------------------

std::ostream & operator<<(std::ostream & ostream , cNcursesManipCol attr) {
	cNcursesOStream & as_ncurses = dynamic_cast<cNcursesOStream &>( ostream );
	if (! as_ncurses) throw std::runtime_error("cNcursesManipCol attribute was sent to a stream that is not compatible with it.");
	as_ncurses.apply( attr );
	return ostream;
}



// ===========================================================================================================

