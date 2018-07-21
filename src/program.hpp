#pragma once
#ifndef INCLUDE_src_program_hpp
#define INCLUDE_src_program_hpp

#include "mylib/memory.hpp"

class cProgram_pimpl;
class cProgram_pimpl_delete { public: void operator()(cProgram_pimpl *p) const; }; ///< delete incomplete-class unique_ptr idiom

class cProgram {
	public:
		virtual ~cProgram();

		void run();

	private:
		unique_ptr<cProgram_pimpl,cProgram_pimpl_delete> m_pimpl;
};

#endif
