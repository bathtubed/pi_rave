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
	
	enum adc_channels : pin_t {LDR = 0, MIC, N_ADCS};
	
private:
	pin_t led_pin_;
	pin_t mic_pin_;
	pin_t adc_cs_;
	
public:
	hardware(pin_t led_pin = 4, pin_t mic_pin = 1, pin_t adc_cs = 0);
	~hardware();
	
public:
	void set_led(short brightness);
	short readadc(pin_t a);
	
	template<typename IterT, typename TimeT>
	bool sample(IterT dest, unsigned n_samples, TimeT sample_dur);
	
	bool sample(short* buffer, 
				unsigned n_samples,
				std::chrono::microseconds sample_us);
	
};

// Generic timed sampling code
template<typename IterT, typename TimeT>
bool hardware::sample(IterT dest, size_t n_samples, TimeT sample_dur)
{
	namespace chrono = std::chrono;
	using clock = chrono::high_resolution_clock;
	using namespace std::chrono_literals;
	
	auto t = clock::now();
	bool caught_up = true;
	for(size_t i = 0; i < n_samples; i++)
	{
		*(dest++) = readadc(mic_pin_);
		if((t + sample_dur) < clock::now()) caught_up = false;
		
		std::this_thread::sleep_for((t + sample_dur) - clock::now());
		t += sample_dur;
	}
	
	return caught_up;
}

#endif // _HARDWARE_H_
