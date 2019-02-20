#include "circular_queue.h"
#include "fft.h"
#include "spectrum_analyzer.h"
#include "publisher.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <thread>

int main()
{
	circular_queue<int> q(10);
	
	for(auto i = 0; i < 5; i++)
		q.push_back(i);
	
	auto print = [](const auto &q) {
		std::copy(q.begin(), q.end(),
				  std::ostream_iterator<decltype(*q.begin())>(std::cout, " "));
		std::cout << std::endl;
	};
	
	print(q);
	
	for(auto i = 0; i < 3; i++)
		std::cout << q.pop() << " ";
	
	std::cout << std::endl;
	
	auto t = std::thread([&q]
	{
		for(auto i = 0; i < 3; i++)
		{
			std::cout << "Pushing " << i << std::endl;
			q.push_back(i);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	});
	
	std::copy_n(front_consumer(q), 5,
				std::ostream_iterator<decltype(q.front())>(std::cout, "\n"));
	
	std::cout << std::endl;
	print(q);
	
	for(auto i = 0; i < 7; i++)
		q.push_back(i);
	
	print(q);
	
	for(auto i = 0; i < 18; i++)
		q.push_back(i);
	
	print(q);
	
	print(q.slice(0, 4));
	
	std::cout << std::endl;
	
	std::cout << "Generating fft plan..." << std::endl;
	fft_plan fft(9);
	
	std::cout << "Generating impulse function:" << std::endl;
	q.push_back(1000);
	for(int i = 0; i < 8; i++)
	{
		q.push_back(0);
	}
	
	print(q);
	
	std::cout << "Executing fft:" << std::endl;
	print(fft.execute(q.begin()));
	
	std::cout << "Generating constant:" << std::endl;
	for(int i = 0; i < 10; i++)
		q.push_back(1000);
	
	print(q);
	
	std::cout << "Executing fft:" << std::endl;
	print(fft.execute(q.begin()));
	
	std::cout << "Connecting to mosquitto" << std::endl;
	
	publisher mqtt("192.168.1.74");
	
	for(int i = range_t::SUB_BASS; i < range_t::N; i++)
		mqtt.publish_freq(i, i*10 + 10);
	
	return 0;
}

