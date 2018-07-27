#pragma once
#ifndef INCLUDED_src_sensors_interrupts_api_hpp
#define INCLUDED_src_sensors_interrupts_api_hpp

#include "sensor/base.hpp"

#include "mylib/memory.hpp"
#include "mylib/string.hpp"
#include "mylib/container.hpp"

class cSensorInterrupts;
unique_ptr<cSensorInterrupts> factory_cSensorInterrupts();

/// @class this data are the counters, changable
class cOneInterruptCounter final {
	public:
		using t_count = long long int;

		cOneInterruptCounter()=default;
		cOneInterruptCounter(std::vector<cOneInterruptCounter::t_count> && per_cpu_call);

		/// should not be changed from outside
		/// @{
		std::vector<cOneInterruptCounter::t_count> m_per_cpu_call; /// per given CPU (0-indexed): number of interrupt calls
		/// @}
};

/// @class this data is the statis information (like name etc)
class cOneInterruptInfo final {
	public:
		cOneInterruptInfo()=default;
		cOneInterruptInfo(const string & col0, const string & col1, const string & col2, vector<string> && devs);

		int get_id_num() const; ///< is non-standard, then returns the name, otherwise throws
		string get_full_info() const;

		/// should not be changed from outside
		/// @{
		int m_id_as_num; ///< if non-standard then this is the number, otherwise might be undefined
		bool m_standard; ///< is this a standard interrupt (the one that has a shortcut like NMI in 1st collumn), or nonstarnard

		string m_id; ///< a short ID, usually from a number like "30", or shortcut like "NMI" or "SPU", see also get_num()
		string m_name1; ///< for non-standard interrupts, the 1st word like "IR-IO-APIC", else ""
		string m_name2; ///< for non-standard interrupts, the 2nd word like "2-edge", else ""
		string m_name; ///< best name, e.g. part 3 of specific name like "timer" or "xhci_hcd", or the full standard name like "Non-maskable interrupts" for "NMI"
		vector<string> m_devs; ///< devices names eg "ehci_hcd:usb3", "ehci_hcd:usb6"
		string m_devs_str; ///< devices string for display like "ehci_hcd:usb3,ehci_hcd:usb6"

		/// @}

		static bool is_id_standard(const string & id_str); ///< returns true for ID like "NMI", else returns false for id like "30" and then also writes number 30 into out_id_num
	private:
		static bool is_id_standard_and_parse(const string & id_str, int & out_id_num); ///< returns true for ID like "NMI", else returns false for id like "30" and then also writes number 30 into out_id_num

		string make_dev_str() const; ///< returns string with all devices, to be saved into m_devs_str
};

#endif


