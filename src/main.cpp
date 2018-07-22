#include <iostream>
#include "program.hpp"

#include <exception>

using std::cout;
using std::cerr;
using std::cin;
using std::endl;

int main(const int argc, const char * const * argv) {
	try {
		cProgram program;
		program.options(argc,argv);
		program.run();
	}
	catch(const std::runtime_error & ex) {
		cout << "Error: " << ex.what() << endl;
	}
	catch(const std::exception & ex) {
		cout << "Error: " << ex.what() << endl;
	}
	catch(...) {
		cout << "Error: (unknown excpetion) - rethrowing" << endl;
		throw; // perhaps default handler will tell more about it
	}
}
