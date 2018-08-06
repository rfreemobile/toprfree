
#include <streambuf>
#include <ostream>
#include <iostream>


/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
class MyData
{
	//example data class, not used
};

class MyBuffer : public std::basic_streambuf<char, std::char_traits<char> >
{

public:

	inline MyBuffer(MyData data) :
	data(data)
	{
		setp(buf, buf + BUF_SIZE);
	}

protected:

	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	inline virtual int overflow(int c = Traits::eof())
	{
#ifdef DEBUG
		std::cout << "(over)";
#endif
		// Handle output
		putChars(pbase(), pptr());
		if (c != Traits::eof()) {
			char c2 = c;
			// Handle the one character that didn't fit to buffer
			putChars(&c2, &c2 + 1);
		}
		// This tells that buffer is empty again
		setp(buf, buf + BUF_SIZE);
		
		return c;
	}

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	inline virtual int sync(void)
	{
		// Handle output
		putChars(pbase(), pptr());
		// This tells that buffer is empty again
		setp(buf, buf + BUF_SIZE);
		return 0;
	}

private:

	// For EOF detection
	typedef std::char_traits<char> Traits;

	// Work in buffer mode. It is also possible to work without buffer.
	static const size_t BUF_SIZE = 64;
	char buf[BUF_SIZE];

	// This is the example userdata
	MyData data;

	// In this function, the characters are parsed.
	inline void putChars(const char* begin, const char* end){
#ifdef DEBUG
		std::cout << "(putChars(" << static_cast<const void*>(begin) <<
		    "," << static_cast<const void*>(end) << "))";
#endif
		//just print to stdout for now
		for (const char* c = begin; c < end; c++){
			std::cout << *c;
		}
	}

};


/// based on https://stackoverflow.com/questions/772355/how-to-inherit-from-stdostream
class MyOStream : public std::basic_ostream< char, std::char_traits< char > >
{

public:

	inline MyOStream(MyData data) :
	std::basic_ostream< char, std::char_traits< char > >(&buf),
	buf(data)
	{
	}

private:

	MyBuffer buf;

};

int main(void)
{
	MyData data;
	MyOStream o(data);
	
	for (int i = 0; i < 8; i++)
		o << "hello world! ";
	
	o << std::endl;
	
	return 0;
}


