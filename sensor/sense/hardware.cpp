#define HARDWARE_CPP
#include "hardware.h"

#include <pigpio.h>

#include <stdexcept>

constexpr auto spi_rate = 1000000;

hardware::hardware(pin_t led_pin, pin_t adc_cs):
	led_pin_(led_pin), adc_cs_(adc_cs)
{
	if(gpioInitialise() < 0)
	{
		throw std::runtime_error("Setup of pigpio failed");
	}
	
	if((spi_ = spiOpen(adc_cs_, spi_rate, 0)) < 0)
	{
		throw std::runtime_error("Setup of SPI failed");
	}
	
	gpioSetMode(led_pin_, PI_OUTPUT);
}

void hardware::read_adc(const adc_buffer_t &tx, adc_buffer_t* rx) noexcept
{
	constexpr auto count = std::tuple_size<adc_buffer_t>::value;
	
	spiXfer(
		spi_,
		const_cast<char*>(tx.data()),
		rx->data(),
		count
	);
}

std::chrono::microseconds hardware::delay(std::chrono::microseconds dur) noexcept
{
	return std::chrono::microseconds(gpioDelay(dur.count()));
}

std::chrono::microseconds hardware::tick() noexcept
{
	return std::chrono::microseconds(gpioTick());
}

hardware::~hardware()
{
	gpioWrite(led_pin_, 0);
	gpioTerminate();
}

void hardware::set_led(unsigned int brightness) noexcept
{
	gpioHardwarePWM(led_pin_, 1000, brightness);
}
