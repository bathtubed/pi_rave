#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <cstdint>
#include <chrono>
#include <thread>

class hardware
{
public:
	// TYPES
	
	// Represents the BCM pin numbering
	using pin_t = uint8_t;
	
private:
	pin_t blink_pin_;
	pin_t adc_cs_;
	pin_t mic_pin_;
	
public:
	hardware(pin_t blink_pin = 4, pin_t mic_pin_t mic_pin = 1, pin_t adc_cs = 0);
	~hardware();
	
public:
	void blink();
	short readadc(pin_t a);
	
	template<typename IterT, typename TimeT>
	void sample(IterT dest, unsigned n_samples, TimeT sample_dur);
	
	void sample(short* buffer, 
				unsigned n_samples,
				std::chrono::microseconds sample_us = 100);
	
};

// Generic timed sampling code
template<typename IterT, TimeT>
void hardware::sample(IterT dest, size_t n_samples, TimeT sample_dur)
{
	using chrono = std::chrono;
	using clock = chrono::high_resolution_clock;
	using namespace std::chrono_literals;
	
	auto t = clock::now();
	for(size_t i = 0; i < n_samples; i++)
	{
		*(dest++) = readadc(mic_pin);
		std::this_thread::sleep_for((t + sample_dur) - clock::now());
		t += sample_dur;
	}
}

#endif // _HARDWARE_H_
