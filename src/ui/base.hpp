#ifndef INCLUDE_rfreetop_ui_base
#define INCLUDE_rfreetop_ui_base

#include <ostream>
#include <istream>

namespace nToprfree {

class cUiBase {
	public:
		using t_keyboard_key = int;

		virtual ~cUiBase()=default;

		virtual void init()=0;

		virtual void start_frame()=0;
		virtual std::ostream& write()=0;
		virtual void finish_frame()=0;

		virtual t_keyboard_key get_key()=0;
};

} // namespace

#endif
