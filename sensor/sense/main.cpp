#include "hardware.h"
#include "circular_queue.h"
#include "fft.h"
#include "spectrum_analyzer.h"
#include "publisher.h"

#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>
#include <array>
#include <tuple>


int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;
	
	std::cout << "Hello, world!\n";
	
	hardware h{18, hardware::MIC};
	
	circular_queue<short> sample_q((1 << 18) - 1);
	
	constexpr auto sample_size = 1024;
	constexpr auto sample_period = 100us;
	constexpr auto sample_frequency = 1s / sample_period;
	
	constexpr auto publish_frequency = 50;
	constexpr auto publish_period = 1s / publish_frequency;
	
	// Calculate the indecies of the frequencies we care about
	constexpr auto freq_bins = get_freq_bins(sample_frequency, sample_size);
	
	// Start Sampling
	std::thread sampling_thread([&]()
	{
		h.set_led(0);
		while(1)
		{
			if(!h.sample(std::back_inserter(sample_q), sample_size, sample_period))
				h.set_led(1000);
		}
	});
	
	fft_plan fft(sample_size);
	
	publisher mqtt("mosquitto");
	
	// Wait for enough samples
	while(sample_q.size() < sample_size)
		std::this_thread::sleep_for(sample_size*sample_period/5);
	
	
	namespace chrono = std::chrono;
	using clock = chrono::high_resolution_clock;
	using namespace std::chrono_literals;
	
	auto t = clock::now();
	
	while(true)
	{
		// Perform the FFT on the previous data
		const auto& spectrum = fft.execute(sample_q.begin());
		
		// Analyze the spectrum
		const auto amplitudes = get_amplitudes(spectrum, freq_bins);
		
		// Leave space
		sample_q.pop_front(sample_q.size() - sample_size);
		
		for(unsigned i = 0; i < amplitudes.size(); i++)
		{
			mqtt.publish_freq(i, amplitudes[i]);
		}
		
		std::this_thread::sleep_for((t + publish_period) - clock::now());
		t += publish_period;
	}
	
	return 0;
}
