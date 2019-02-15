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
	
	using prec_t = double;
	using samp_t = std::complex<prec_t>;
	using data_t = std::unique_ptr<samp_t[], fftw_data_deleter>;
};

class transform: public transform_types::data_t
{
public:
	using prec_t = transform_types::prec_t;
	using samp_t = transform_types::samp_t;
	using data_t = transform_types::data_t;
	
private:
	// DATA
	unsigned len_;
	
public:
	transform(unsigned len): len_(len) {}
	
public:
	samp_t* begin() {return get();}
	samp_t* end() {return get() + len_;}
	
	const samp_t* begin() const {return get();}
	const samp_t* end() const {return get();}
};

class fft_plan
{
private:
	size_t n_;
	transform buf_in_, buf_out_;
	fftw_plan plan_;
	
public:
	fft_plan(size_t n, bool estimate = false):
		n_(n), buf_in_(n_), buf_out_(n_), plan_(generate_plan(estimate)) {}
	
	~fft_plan() {fftw_destroy_plan(plan_);}
	
public:
	fftw_plan generate_plan(bool estimate = false) const
	{
		return fftw_plan_dft_1d(
			n_, 
			reinterpret_cast<fftw_complex*>(buf_in_.get()),
			reinterpret_cast<fftw_complex*>(buf_out_.get()), 
			FFTW_FORWARD, estimate? FFTW_ESTIMATE:FFTW_MEASURE);
	}
	
public:
	template<typename InputIter>
	const transform& execute(InputIter from)
	{
		std::copy_n(from, n_, buf_in_.begin());
		fftw_execute(plan_);
		return buf_out_;
	}
};


#endif // _FFTW_PLAN_H_
