#include <stdlib.h>
#include <iostream>

#include "hardware.h"

int main(int argc, char* argv[])
{
	std::cout << "Hello, world!\n";
	
	hardware h;
	
	while(true)
	{
		std::cout << "Blinking..." << std::endl;
		h.blink();
	}
	
	return 0;
}
