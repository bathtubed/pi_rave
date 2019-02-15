#include "hardware.h"

#include <wiringPi.h>

#include <stdexcept>

constexpr auto spi_rate = 1000000;

namespace // anonymous
{

class setup_wiringpi
{
	static setup_wiringpi s;
	
	setup_wiringpi()
	{
		wiringPiSetupGpio();
	}
};

} // anonymous

hardware::hardware(pin_t blink_pin, pin_t adc_cs):
	blink_pin_(blink_pin), adc_cs_(adc_cs)
{
	if(wiringPiSPISetup(adc_cs_, 1000000) < 0)
	{
		throw std::runtime_error("Setup of SPI failed");
	}
}

short hardware::readadc(pin_t a)
{
	if(a > N_ADCS)
		return -1;
	
	unsigned char buffer[3] = {1}; // start bit
	buffer[1] = a << 4;
	wiringPiSPIDataRW(spiChannel, buffer, sizeof(buffer));
	
	return ( uint16_t(buffer[1] & 3) << 8 ) | buffer[2]; // get last 10 bits
}

// Instantiates here to achieve max throughput
void hardware::sample(short* buffer,
					  unsigned n_samples,
					  std::chrono::microseconds sample_us)
{
	sample<short*>(buffer, n_samples, sample_us);
}

hardware::~hardware()
{
	digitalWrite(blink_pin_, 0);
}

void hardware::blink()
{
	digitalWrite(blink_pin_, 1);
	delay(100);
	digitalWrite(blink_pin_, 0);
	delay(100);
}
