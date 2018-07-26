#pragma once
#ifndef INCLUDED_src_sensors_interrupts_hpp
#define INCLUDED_src_sensors_interrupts_hpp

#include "sensor/base.hpp"

#include "sensor/interrupts.ipp"

class cSensorInterruptsError : public std::runtime_error {
	public:
		cSensorInterruptsError(const string & err="");
};

class cSensorInterrupts : cSensor {
	public:
		virtual ~cSensorInterrupts()=default;
		virtual void gather();
		virtual void print() const;

	protected:
		int m_num_cpu;

		vector<cOneInterruptInfo> m_info; ///< for each interrupt: its information
		vector<cOneInterruptCounter> m_current; ///< for each interrupt: all its CPU counters - current value
};

#endif


