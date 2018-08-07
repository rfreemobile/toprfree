#pragma once
#ifndef INCLUDE_src_sensor_base_hpp
#define INCLUDE_src_sensor_base_hpp

#include "mylib/memory.hpp"

#include <chrono>

#include "ui/base.hpp"

class cSensor {
	public:
		cSensor();
		virtual ~cSensor()=default;

		virtual void gather() = 0; ///< populate current counters
		virtual void calc_stats() = 0; ///< calculate diffs etc
		virtual void print( shared_ptr<nToprfree::cUiBase> ui ) const = 0; ///< show results to user via given UI
		virtual void step() = 0; ///< next step for counters, e.g. move current counters to previous (if NOT m_before_first)

		using t_clock = std::chrono::steady_clock; ///< clock to be used for measuring
		using t_duration = std::chrono::duration<double>; ///< clock to be used for measuring

	protected:
		bool m_before_first; ///< was any step() executed yet
};

#endif
