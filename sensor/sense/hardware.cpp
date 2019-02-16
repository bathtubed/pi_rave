#include "hardware.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <stdexcept>

constexpr auto spi_rate = 1000000;

hardware::hardware(pin_t led_pin, pin_t mic_pin, pin_t adc_cs):
	led_pin_(led_pin), mic_pin_(mic_pin), adc_cs_(adc_cs)
{
	if(wiringPiSetupGpio() < 0)
	{
		throw std::runtime_error("Setup of wiringPi failed");
	}
	
	if(wiringPiSPISetup(adc_cs_, 1000000) < 0)
	{
		throw std::runtime_error("Setup of SPI failed");
	}
	
	pinMode(led_pin_, PWM_OUTPUT);
}

short hardware::readadc(pin_t a)
{
	if(a > N_ADCS)
		return -1;
	
	unsigned char buffer[3] = {1}; // start bit
	buffer[1] = 1 << 7 | a << 4;
	wiringPiSPIDataRW(adc_cs_, buffer, sizeof(buffer));
	
	return ( uint16_t(buffer[1] & 3) << 8 ) | buffer[2]; // get last 10 bits
}

// Instantiates here to achieve max throughput
bool hardware::sample(short* buffer,
					  unsigned n_samples,
					  std::chrono::microseconds sample_us)
{
	return sample<short*>(buffer, n_samples, sample_us);
}

hardware::~hardware()
{
	pinMode(led_pin_, OUTPUT);
	digitalWrite(led_pin_, 0);
}

void hardware::set_led(short brightness)
{
	pwmWrite(led_pin_, brightness);
}
