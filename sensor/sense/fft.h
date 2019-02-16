#ifndef _FFTW_PLAN_H_
#define _FFTW_PLAN_H_

#include <fftw3.h>

#include <memory>
#include <complex>
#include <algorithm>

namespace transform_types
{
	struct fftw_data_deleter
	{
		void operator()(void* p) const {fftw_free(p);}
	};
	
	using samp_t = double;
	using freq_t = std::complex<samp_t>;
	
	template<typename T>
	using data_t = std::unique_ptr<T[], fftw_data_deleter>;
};

template<typename T>
class transform: public transform_types::data_t<T>
{
public:
	using value_type = T;
	using data_t = transform_types::data_t<T>;
	
private:
	// DATA
	unsigned len_;
	
public:
	transform(unsigned len);
	
public:
	value_type* begin() {return this->get();}
	value_type* end() {return this->get() + len_;}
	
	const value_type* begin() const {return this->get();}
	const value_type* end() const {return this->get() + len_;}
};

template<>
inline transform<transform_types::samp_t>::transform(unsigned len):
	data_t(fftw_alloc_real(len)), len_(len) {}


template<>
inline transform<transform_types::freq_t>::transform(unsigned len):
	data_t(reinterpret_cast<transform_types::freq_t*>(fftw_alloc_complex(len)))
	, len_(len) {}

using time_domain = transform<transform_types::samp_t>;
using freq_domain = transform<transform_types::freq_t>;

class fft_plan
{
private:
	size_t n_;
	time_domain buf_in_;
	freq_domain buf_out_;
	fftw_plan plan_;
	
public:
	fft_plan(size_t n, bool estimate = false):
		n_(n), buf_in_(n_), buf_out_(n_/2+1), plan_(generate_plan(estimate)) {}
	
	~fft_plan() {fftw_destroy_plan(plan_);}
	
public:
	fftw_plan generate_plan(bool estimate = false) const
	{
		return fftw_plan_dft_r2c_1d(
			n_, 
			reinterpret_cast<double*>(buf_in_.get()),
			reinterpret_cast<fftw_complex*>(buf_out_.get()), 
			estimate? FFTW_ESTIMATE:FFTW_MEASURE
		);
	}
	
public:
	template<typename InputIter>
	const auto& execute(InputIter from)
	{
		std::copy_n(from, n_, buf_in_.begin());
		fftw_execute(plan_);
		return buf_out_;
	}
};


#endif // _FFTW_PLAN_H_
