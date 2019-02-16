#ifndef _SPECTRUM_ANALYZER_H_
#define _SPECTRUM_ANALYZER_H_

#include "fft.h"

enum range_t {
	SUB_BASS = 0, BASS, LOW_MID, HIGH_MID, PRESENCE, BRILLIANCE, N
};

constexpr auto freq_to_bin(int f, int n, int sample_freq)
{
	return f * n / sample_freq;
}

constexpr auto frequencies = std::array{16, 60, 250, 2000, 4000, 6000, 16000};

constexpr auto frequency_names = std::array{
	"sub-bass", "bass", "low-mids", "high-mids", "presence", "brilliance"};

constexpr auto get_freq_bins(int sample_freq, int n)
{
	return std::apply([sample_freq, n](auto... freq)
	{
		return std::array{freq_to_bin(freq, n, sample_freq)...};
	}, frequencies);
}

std::array<int, range_t::N> get_amplitudes(
	const freq_domain &spectrum, 
	const std::array<int, range_t::N+1> &freq_bins
);

#endif // _SPECTRUM_ANALYZER_H_
