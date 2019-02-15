#include "hardware.h"
#include "circular_queue.h"

#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>

int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;
	
	std::cout << "Hello, world!\n";
	
	hardware h{18, hardware::LDR};
	
	circular_queue<short> sample_q((1 << 16) - 1);
	
	constexpr auto sample_size = 1000;
	constexpr auto sample_period = 1ms;
	
	std::thread sampling([&]()
	{
		while(1)
		{
			h.sample(std::back_inserter(sample_q), sample_size, sample_period);
		}
	});
	
	// Wait for enough samples
	while(sample_q.size() < sample_size)
		std::this_thread::sleep_for(sample_size*sample_period/5);
	
	while(true)
	{
		double avg = 0.0;
		auto q = sample_q.begin();
		for(int i = 0; i < sample_size; i++)
			avg += double(*(q++)) / sample_size;
		
		std::cout << "Average brightness: " << avg << std::endl;
		h.set_led(avg);
		
		// Lose about a fifth of the samples
		std::this_thread::sleep_for(sample_period/5);
		
		sample_q.pop_front(sample_q.size() - 1000);
	}
	
	return 0;
}
