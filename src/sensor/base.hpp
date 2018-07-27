#pragma once
#ifndef INCLUDE_src_sensor_base_hpp
#define INCLUDE_src_sensor_base_hpp

class cSensor {
	public:
		cSensor();
		virtual ~cSensor()=default;

		virtual void gather() = 0; ///< populate current counters
		virtual void print() const = 0; ///< show results to user
		virtual void step() = 0; ///< next step for counters, e.g. move current counters to previous (if NOT m_before_first)

	protected:
		bool m_before_first; ///< was any step() executed yet
};

#endif
