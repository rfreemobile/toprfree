
#include "pfp-local/pfp_ncurses_stream_manip.hpp"
#include "pfp-local/pfp_ncurses_stream.hpp"

#include <ncurses.h> // system's

namespace nPfp_ncurses {


/*
cNcursesManipCol::cNcursesManipCol(short color_pair_shifted)
: m_pair_shifted( color_pair_shifted )
{ }

short cNcursesManipCol::get_pair_shifted() const {
	return this->m_pair_shifted;
}

// -------------------------------------------------------------------

std::ostream & operator<<(std::ostream & ostream , cNcursesManipCol attr) {
	cNcursesOStream & stream_as_ncurses = dynamic_cast<cNcursesOStream &>( ostream );
	if (! as_ncurses) throw std::runtime_error("cNcursesManipCol attribute was sent to a stream that is not compatible with it.");
	stream_as_ncurses.apply( attr );
	return ostream;
}
*/

} // namespace

// ===========================================================================================================

