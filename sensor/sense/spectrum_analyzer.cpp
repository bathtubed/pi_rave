#include "spectrum_analyzer.h"

std::array<int, range_t::N> get_amplitudes(
	const freq_domain &spectrum, 
	const std::array<int, range_t::N+1> &freq_bins
)
{
	std::array<int, range_t::N> ret;
	
	for(int i = BASS; i < range_t::N; i++)
	{
		double total = 0.0;
		for(int bin = freq_bins[i]; bin < freq_bins[i+1]; bin++)
			total += std::abs(spectrum[bin]);
		
		ret[i] = total / (freq_bins[i+1] - freq_bins[i]);
	}
	
	return ret;
}
