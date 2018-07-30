#pragma once
#ifndef INCLUDED_src_sensors_interrupts_hpp
#define INCLUDED_src_sensors_interrupts_hpp

#include "sensor/base.hpp"

#include "sensor/interrupts-api.hpp"

#include <chrono>

class cSensorInterruptsError : public std::runtime_error {
	public:
		cSensorInterruptsError(const string & err="");
};

class cSensorInterrupts : cSensor {
	public:

		virtual ~cSensorInterrupts()=default;

		virtual void gather();
		virtual void calc_stats();
		virtual void print() const;
		virtual void step();

		cInterruptOptions m_options;

		std::chrono::time_point< cSensor::t_clock  > m_time_previous; ///< time point when we meastured m_previous
		std::chrono::time_point< cSensor::t_clock > m_time_current;  ///< time point when we meastured m_current
		cSensor::t_duration m_timediff_current; ///< duration between m_current and m_previous

	protected:
		size_t m_num_cpu;


		vector<cOneInterruptInfo> m_info; ///< for each interrupt: its information
		vector<cOneInterruptCounter> m_current; ///< for each interrupt: all its CPU counters - current value
		vector<cOneInterruptCounter> m_previous; ///< same as m_current but previous iteration
		vector<cOneInterruptCounter> m_diff_ival; ///< same as m_current but counters are difference since previous step - per interval
		vector<cOneInterruptCounter> m_diff_sec; ///< same as m_diff_ival, but per second
};

#endif


