
#ifndef INCLUDE_rfreetop_ui_ncurses
#define INCLUDE_rfreetop_ui_ncurses

#include <ostream>
#include <istream>

#include "ui/base.hpp"

namespace nToprfree {

class cUiNcurses : public cUiBase {
	public:
		virtual ~cUiNcurses()=default;

		virtual void init();

		virtual void start_frame();
		virtual std::ostream& write();
		virtual void finish_frame();

		virtual t_keyboard_key get_key();
};

};

#endif
