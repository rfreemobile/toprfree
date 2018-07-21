#include <iostream>
#include "program.hpp"

using std::cout;
using std::cerr;
using std::cin;
using std::endl;

int main(const int argc, const char * const * argv) {
	cProgram program;
	program.options(argc,argv);
	program.run();
}
