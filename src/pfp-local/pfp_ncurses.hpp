#ifndef INCLUDE_pfp_ncurses
#define INCLUDE_pfp_ncurses

namespace nPfp_ncurses {

namespace nCol {

extern const short black;
extern const short red;
extern const short green;
extern const short yellow;
extern const short blue;
extern const short magenta;
extern const short cyan;
extern const short white;

} // namespace nCol


class cPairMaker {
	public:
		cPairMaker()=default;

		static cPairMaker & singleton();

		void init_ncurses_colors(); ///< call this ONLY when ncurses, screeens, start_color() is all initialized already. Can be called more then 1 but just waste of performance (re-inits colors)
		short colors_to_pair(int fg, int bg) const; ///< this works provided that init_ncurses_colors() was called previously. UB otherwise
};


} // namespace nPfp_ncurses


#endif

