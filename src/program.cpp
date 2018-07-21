
#include "program.hpp"
#include "mylib/iostream.hpp"

#include "sensor/interrupts.hpp"

void cProgram_pimpl_delete::operator()(cProgram_pimpl *p) const { delete p; }

class cProgram_pimpl {
	public:
};


void cProgram::run() {
	cout << "Running the program" << endl;
}

cProgram::~cProgram() { }

