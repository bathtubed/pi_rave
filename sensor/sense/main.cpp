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
	
	hardware h{18, hardware::MIC};
	
	circular_queue<short> sample_q((1 << 18) - 1);
	
	constexpr auto sample_size = 20000;
	constexpr auto sample_period = 50us;
	
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
		short max = 0, min = 1024;
		
		for(auto sample : sample_q.slice(0, sample_size))
		{
			max = std::max(max, sample);
			min = std::min(min, sample);
		}
		
		std::cout << "Peak-to-peak: " << (max-min) << std::endl;
		h.set_led(max-min);
		
		// Lose about a fifth of the samples
		std::this_thread::sleep_for(sample_period/5);
		
		sample_q.pop_front(sample_q.size() - sample_size);
	}
	
	return 0;
}
