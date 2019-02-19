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
#include <atomic>

#include <signal.h>
#include <string.h>
#include <unistd.h>

namespace chrono = std::chrono;
using clk = chrono::high_resolution_clock;
using namespace std::chrono_literals;

template<typename F>
auto benchmark(F&& f)
{
	auto t = clk::now();
	std::forward<F>(f)();
	return clk::now() - t;
}

std::atomic<bool> running{true};
void term(int)
{
	running = false;
}

int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;
	
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGABRT, &action, NULL);
	
	constexpr auto sample_size = 1024;
	constexpr auto sample_period = 100us;
	constexpr auto sample_frequency = 1s / sample_period;
	
	using sample_chunk_t = std::array<hardware::sample_t, sample_size>;
	
	constexpr auto publish_frequency = 50;
	constexpr auto publish_period = 1000000us / publish_frequency;
	
	std::cout << "Hello, world! (sample frequency = " << sample_frequency << "Hz)\n";
	
	hardware h{18};
	
	sample_chunk_t init{0};
	// God forbid we're ever behind by 100.
	circular_queue<sample_chunk_t> sample_q(100, init);
	
	// Measure overhead of a single sample
	{
		const auto actual_sample_period = 
			chrono::duration_cast<std::chrono::microseconds>(benchmark([&h]
			{
				h.read_adc<hardware::MIC>();
			}));
		
		if(actual_sample_period > sample_period)
		{
			std::cout << "Warning: single sample takes longer than period:";
			std::cout << actual_sample_period.count() << "us > ";
			std::cout << sample_period.count() << "us." << std::endl;
		}
		else
		{
			std::cout << "Single Sample (" << h.read_adc<hardware::MIC>();
			std::cout << ") took " << actual_sample_period.count() << "us\n";
		}
	}
	
	// Calculate the indecies of the frequencies we care about
	constexpr auto freq_bins = get_freq_bins(sample_frequency, sample_size);
	
	// Start Sampling
	std::thread sampling_thread([&]()
	{
		std::cout << "Beginning Sample" << std::endl;
		while(running)
		{
			sample_chunk_t& next = *sample_q.end();
			const auto missed = h.sample<hardware::MIC>(
				next.data(), sample_size, sample_period);
			sample_q.push_back();
			
			h.set_led(missed * 1000000 / sample_size);
		}
	});
	
	fft_plan fft(sample_size);
	
	std::cout << "Connecting to broker..." << std::endl;
	publisher mqtt("mosquitto");
	std::cout << "Starting to publish every " << publish_period.count() << std::endl;
	
	auto t = clk::now();
	
	while(running)
	{
		// Wait for enough samples
		while(sample_q.size() < 1 && running)
			std::this_thread::sleep_for(sample_size*sample_period/5);
		
		// Perform the FFT on the previous data
		const auto& spectrum = fft.execute(sample_q.front().begin());
		for(auto s : sample_q.slice(0, sample_q.size()))
			s.fill(0);
		
		sample_q.pop_some(sample_q.size());
		
		// Analyze the spectrum
		const auto amplitudes = get_amplitudes(spectrum, freq_bins);
		
		for(unsigned i = 0; i < amplitudes.size(); i++)
		{
			mqtt.publish_freq(i, amplitudes[i]);
		}
		
		std::this_thread::sleep_for((t + publish_period) - clk::now());
		t += publish_period;
	}
	
	std::cout << "Exiting nicely~" << std::endl;
	sampling_thread.join();
	std::cout << "Goodbye!!!" << std::endl;
	
	return 0;
}
