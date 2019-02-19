#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <cstdint>
#include <chrono>
#include <thread>
#include <array>

class hardware
{
public:
	// TYPES
	using pin_t = uint8_t;
	enum adc_channel : pin_t {LDR = 0, MIC, N_ADCS};
	
	using adc_buffer_t = std::array<char, 3>;
	
	struct sample_t: public adc_buffer_t
	{
		operator short () const {return ((*this)[1] << 8) | (*this)[2];}
		sample_t(short v = 0)
		{
			(*this)[1] = v >> 8;
			(*this)[2] = v & 0xFF;
		}
	};
	
private:
	pin_t led_pin_;
	pin_t adc_cs_;
	int spi_;
	
public:
	hardware(pin_t led_pin = 4, pin_t adc_cs = 0);
	~hardware() noexcept;
	
public:
	// Returns the value of the ADC on channel
	template<adc_channel Chan = LDR>
	sample_t read_adc() noexcept;
	
	void set_led(unsigned int brightness) noexcept;
	
	/* Fills buffer with n_samples results of read_adc<Chan>
	 * If it can't manage to get a sample in time, it will copy that reading
	 * into the next sample until it catches up.
	 * Returns the number of samples missed.
	 */
	template<adc_channel Chan>
	int sample(sample_t* buffer,
				unsigned n_samples,
				std::chrono::microseconds sample_us) noexcept;
				
public:
	// Returns actual number of microseconds delayed
	std::chrono::microseconds delay(std::chrono::microseconds dur) noexcept;
	
private:
	void read_adc(const adc_buffer_t &tx, adc_buffer_t* rx) noexcept;
	std::chrono::microseconds tick() noexcept;
};

template<hardware::adc_channel Chan>
auto hardware::read_adc() noexcept -> sample_t
{
	constexpr auto start_bit = 1;
	constexpr auto single_ended = 1 << 7;
	constexpr adc_buffer_t adc_command{start_bit, single_ended | Chan << 4};
	
	sample_t ret;
	read_adc(adc_command, &ret);
	return ret;
}

// This the hottest code I will ever write
template<hardware::adc_channel Chan>
int hardware::sample(sample_t* buffer,
					 unsigned n_samples,
					 std::chrono::microseconds sample_us) noexcept
{
	int ret = n_samples;
	auto start = tick();
	for(unsigned i = 0; i < n_samples; i = (tick() - start) / sample_us)
	{
		buffer[i++] = read_adc<Chan>();
		const auto dur = start + i*sample_us - tick();
		if(dur.count() > 0) delay(dur);
		--ret;
	}
	
	return ret;
}

// Want to link to hardware.cpp's instantiations
#ifndef HARDWARE_CPP
#define EXTERN extern
#else
#define EXTERN 
#endif

EXTERN template hardware::sample_t hardware::read_adc<hardware::MIC>() noexcept;

EXTERN template int hardware::sample<hardware::MIC>(
	sample_t*, unsigned, std::chrono::microseconds) noexcept;
		

#endif // _HARDWARE_H_
