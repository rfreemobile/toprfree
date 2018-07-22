#pragma once
#ifndef INCLUDED_src_sensors_interrupts_ipp
#define INCLUDED_src_sensors_interrupts_ipp

#include "sensor/base.hpp"

#include "mylib/memory.hpp"
#include "mylib/string.hpp"
#include "mylib/container.hpp"

class cSensorInterrupts;
unique_ptr<cSensorInterrupts> factory_cSensorInterrupts();

/// @class this data are the counters, changable
class cOneInterruptCounter final {
	public:
		cOneInterruptCounter()=default;
		cOneInterruptCounter(std::vector<int> && per_cpu_call);

		/// should not be changed from outside
		/// @{
		std::vector<int> m_per_cpu_call; /// per given CPU (0-indexed): number of interrupt calls
		/// @}
};

/// @class this data is the statis information (like name etc)
class cOneInterruptInfo final {
	public:
		cOneInterruptInfo()=default;
		cOneInterruptInfo(const string & col0, const string & col1, const string & col2, const string & col3);

		int get_id_num() const; ///< is non-standard, then returns the name, otherwise throws

		/// should not be changed from outside
		/// @{
		int m_id_as_num; ///< if non-standard then this is the number, otherwise might be undefined
		bool m_standard; ///< is this a standard interrupt (the one that has a shortcut like NMI in 1st collumn), or nonstarnard

		string m_id; ///< a short ID, usually from a number like "30", or shortcut like "NMI" or "SPU", see also get_num()
		string m_name1; ///< for non-standard interrupts, the 1st word like "IR-IO-APIC", else ""
		string m_name2; ///< for non-standard interrupts, the 2nd word like "2-edge", else ""
		string m_name; ///< best name, e.g. part 3 of specific name like "timer" or "xhci_hcd", or the full standard name like "Non-maskable interrupts" for "NMI"

		/// @}

	private:
		static bool is_id_standard(const string & id_str, int & out_id_num); ///< returns true for ID like "NMI", else returns false for id like "30" and then also writes number 30 into out_id_num
};

#endif


