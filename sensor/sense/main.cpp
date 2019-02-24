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
void term(int i = 0)
{
	running = false;
}

int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;
	
	{
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGTERM);
		sigaddset(&set, SIGABRT);
		pthread_sigmask(SIG_BLOCK, &set, NULL);
	}
	
	constexpr auto chunk_size = 64;
	constexpr auto chunks_per_sample = 4;
	constexpr auto sample_size = chunk_size * chunks_per_sample;
	constexpr auto sample_period = 50us;
	constexpr auto sample_frequency = 1s / sample_period;
	
	using sample_chunk_t = std::array<hardware::sample_t, chunk_size>;
	
	constexpr auto publish_frequency = 30;
	constexpr auto publish_period = 1000000us / publish_frequency;
	
	std::cout << "Hello, world! (sample frequency = " << sample_frequency << "Hz)\n";
	
	hardware h{18};
	
	// Set signals after pigpio tries overwriting them
	signal(SIGTERM, term);
	signal(SIGABRT, term);
	
	sample_chunk_t init{0};
	// God forbid we're ever behind by 100.
	circular_queue<sample_chunk_t> chunk_q(100, init);
	
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
		std::cout << "Beginning Sample Thread" << std::endl;
		
		{
			cpu_set_t cpu_set;
			CPU_ZERO(&cpu_set);
			CPU_SET(0, &cpu_set);
			if(pthread_setaffinity_np(
				pthread_self(), sizeof(cpu_set_t), &cpu_set))
			{
				term();
				return;
			}
		}
		
		while(running)
		{
			sample_chunk_t& next = *chunk_q.end();
			const auto missed = h.sample<hardware::MIC>(
				next.data(), chunk_size, sample_period);
			chunk_q.push_back();
			
			h.set_led(missed * 1000000 / chunk_size);
		}
	});
	
	
	{
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGTERM);
		sigaddset(&set, SIGABRT);
		pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	}
	
	fft_plan fft(sample_size);
	
	std::cout << "Connecting to broker..." << std::endl;
	publisher mqtt("mosquitto");
	std::cout << "Starting to publish every " << publish_period.count() << std::endl;
	
	// Stores the previous values we care about in a queue.
	circular_queue<short> sample_q(sample_size+1, 0);
	
	// Wait for enough samples
	while(chunk_q.size() < chunks_per_sample && running)
		std::this_thread::sleep_for(sample_size*sample_period);
	
	auto t = clk::now();
	
	while(running)
	{
		// Process the data
		for(auto& chunk : chunk_q)
		{
			for(short samp : chunk)
			{
				if(samp == 0)
					sample_q.push_back(sample_q.back());  // interpolating
				else
					sample_q.push_back(samp);
			}
			
			// reset the chunk
			chunk.fill(0);
			chunk_q.pop_front();
		}
		
		// Perform the FFT on the previous data
		const auto& spectrum = fft.execute(sample_q.begin());
		
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
