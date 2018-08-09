
#ifndef INCLUDE_rfreetop_ui_ncurses
#define INCLUDE_rfreetop_ui_ncurses

#include <ostream>
#include <istream>
#include <memory>

#include "ui/base.hpp"

namespace nToprfree {

class cUiNcurses_impl;
class cUiNcurses_impl_deleter { public: void operator()(cUiNcurses_impl *p) const; }; ///< delete incomplete-class unique_ptr idiom

class cUiNcurses : public cUiBase {
	public:
		cUiNcurses();
		virtual ~cUiNcurses()=default;

		virtual void init();

		virtual void start_frame();
		virtual std::ostream& write();
		virtual void finish_frame();

		virtual t_keyboard_key get_key();

	protected:
		std::unique_ptr<cUiNcurses_impl, cUiNcurses_impl_deleter> m_impl;

		nPfp_ncurses::cPairMaker m_pair_maker;
};

};

#endif

