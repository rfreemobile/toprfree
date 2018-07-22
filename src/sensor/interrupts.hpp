#pragma once
#ifndef INCLUDED_src_sensors_interrupts_hpp
#define INCLUDED_src_sensors_interrupts_hpp

#include "sensor/base.hpp"

#include "sensor/interrupts.ipp"

class cSensorInterrupts : cSensor {
	public:
		virtual ~cSensorInterrupts()=default;
		virtual void gather();

	protected:
		int m_num_cpu;

		cOneInterruptInfo m_info;
		cOneInterruptCounter m_current;
		cOneInterruptCounter m_diff;
};

#endif


