#pragma once
#ifndef INCLUDE_src_sensor_base_hpp
#define INCLUDE_src_sensor_base_hpp

class cSensor {
	public:
		virtual ~cSensor()=default;

		virtual void gather() = 0;
};

#endif
