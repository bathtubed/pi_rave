#ifndef _CIRCULAR_QUEUE_H_
#define _CIRCULAR_QUEUE_H_

#include <vector>
#include <cassert>
#include <iterator>
#include <type_traits>

template<typename IterT>
class range
{
public:
	using ptr_type = IterT;
	using size_type = size_t;
	
private:
	IterT start_;
	IterT stop_;
	
public:
	constexpr range(IterT start, IterT stop): start_(start), stop_(stop) {}
	constexpr range(IterT start, size_type len): range(start, start+len) {}
	
public:
	constexpr auto begin() const {return start_;}
	constexpr auto end() const {return stop_;}
	constexpr auto size() const {return end() - begin();}
	
public:
	constexpr auto& operator[] (size_type i) {return *(start_ + i);}
	constexpr const auto& operator[] (size_type i) const {return *(start_ + i);}
};

template<typename T>
class circular_queue
{
public:
	using value_type = T;
	using size_type = size_t;
	
private:
	template<typename CQ>
	class iterator_temp: public std::iterator<
							std::random_access_iterator_tag,
							std::remove_reference_t<
								decltype(std::declval<CQ>().front())>>
	{
	public:
		using value_type = decltype(std::declval<CQ>().front());
		using difference_type = long;
		
	private:
		CQ& q_;
		size_type i_;
		
	public:
		iterator_temp(CQ& q, size_type i): q_(q), i_(i) {}
		
		template<typename CQU>
		iterator_temp(const iterator_temp<CQU>& other):
			q_(other.q_), i_(other.i_) {}
		
	public:
		const value_type& operator* () const {return q_.data_[i_];}
		//value_type& operator* () {return q_.data_[i_];}
		
		difference_type operator- (const iterator_temp& rhs) const
		{
			assert(&(q_) == &(rhs.q_));
			if(i_ >= q_.start_)
				if(rhs.i_ >= q_.start_)
					return difference_type(i_) - rhs.i_;
				else
					return -difference_type(q_.capacity() - i_ + rhs.i_);
			else
				if(rhs.i_ >= q_.start_)
					return difference_type(q_.capacity() - rhs.i_ + i_);
				else
					return difference_type(i_) - rhs.i_;
		}
		
		bool operator== (const iterator_temp& other) const
		{
			return &q_ == &other.q_ && i_ == other.i_;
		}
		
		bool operator!= (const iterator_temp& other) const
		{
			return &q_ != &other.q_ || i_ != other.i_;
		}
		
		bool operator< (const iterator_temp& other) const
		{
			return *this - other < 0;
		}
		
		bool operator> (const iterator_temp& other) const
		{
			return *this - other > 0;
		}
		
		bool operator<= (const iterator_temp& other) const
		{
			return *this - other <= 0;
		}
		
		bool operator>= (const iterator_temp& other) const
		{
			return *this - other >= 0;
		}
		
		iterator_temp& operator++ ()
		{
			q_.shift_right(i_);
			return *this;
		}
		
		iterator_temp operator++ (int)
		{
			auto ret = *this;
			q_.shift_right(i_);
			return ret;
		}
		
		iterator_temp& operator-- ()
		{
			q_.shift_left(i_);
			return *this;
		}
		
		iterator_temp operator-- (int)
		{
			auto ret = *this;
			q_.shift_left(i_);
			return ret;
		}
		
		iterator_temp& operator+= (size_type len)
		{
			q_.shift_right(i_, len);
			return *this;
		}
		
		iterator_temp& operator-= (size_type len)
		{
			q_.shift_left(i_, len);
			return *this;
		}
		
		friend iterator_temp operator+ (iterator_temp lhs, size_type rhs)
		{
			return lhs += rhs;
		}
		
		friend iterator_temp operator+ (size_type lhs, iterator_temp rhs)
		{
			return rhs + lhs;
		}
		
		friend iterator_temp operator- (iterator_temp lhs, size_type rhs)
		{
			return lhs -= rhs;
		}
	};
	
public:
	using contig_range_type = range<const T*>;
	using iterator = iterator_temp<circular_queue>;
	using const_iterator = iterator_temp<const circular_queue>;
	using range_type = range<iterator>;
	using const_range_type = range<const_iterator>;
	
private:
	using storage_type = std::vector<T>;
	using bound_type = volatile size_type;
	
private:
	// DATA
	storage_type data_;
	size_type capacity_;
	bound_type start_;
	bound_type stop_;
	
public:
	circular_queue(size_type buf_size, const T &init = T()):
		data_(buf_size, init), capacity_(buf_size), start_(0), stop_(0)
	{
	}
	
	bool contig() const {return start_ <= stop_;}
	size_type size() const;
	size_type capacity() const {return capacity_;}
	
public:
	contig_range_type array_one() const;
	contig_range_type array_two() const;
	
	iterator begin() {return {*this, start_};}
	const_iterator begin() const {return const_iterator{*this, start_};}
	
	iterator end() {return {*this, stop_};}
	const_iterator end() const {return const_iterator{*this, stop_};}
	
	
	value_type& front() {return *begin();}
	const value_type& front() const {return *begin();}
	
	value_type& back() {return *(end() - 1);}
	const value_type& back() const {return *(end() - 1);}
	
	auto slice(size_type from, size_type to) const
	{return const_range_type{begin()+from, begin()+to};}
	
public:
	void push_back();
	void push_back(value_type value);
	
	void pop_some(int len) {shift_right(start_, len);}
	void pop_front() {shift_right(start_);}
	[[nodiscard]] value_type pop();
	
private:
	size_type shift_right(size_type& i, size_type len = 1) const
	{
		return i = (i+len) % capacity();
	}
	
	size_type shift_right(bound_type& i, size_type len = 1) const
	{
		return i = (i+len) % capacity();
	}
	
	size_type shift_left(size_type& i, size_type len = 1) const
	{
		return shift_right(i, -len);
	}
	
	size_type shift_left(bound_type& i, size_type len = 1) const
	{
		return shift_right(i, -len);
	}
};

template<typename T>
auto circular_queue<T>::size() const -> size_type
{
	if(contig())
		return stop_ - start_;
	else
		return data_.size() - start_ + stop_;
}

template<typename T>
auto circular_queue<T>::array_one() const -> contig_range_type
{
	if(contig())
		return {data_.begin() + start_, size()};
	else
		return {data_.begin() + start_, data_.end()};
}

template<typename T>
auto circular_queue<T>::array_two() const -> contig_range_type
{
	if(contig())
		return {data_.begin() + stop_, 0};
	else
		return {data_.begin(), stop_};
}

template<typename T>
void circular_queue<T>::push_back()
{
	if(size() == capacity_ - 1)
	{
		shift_right(stop_);
		shift_right(start_);
	}
	else
	{
		shift_right(stop_);
	}
}

template<typename T>
void circular_queue<T>::push_back(T value)
{
	data_[stop_] = std::move(value);
	push_back();
}

template<typename T>
auto circular_queue<T>::pop() -> value_type
{
	auto ret = front();
	pop_front();
	return ret;
}




#endif // _CIRCULAR_QUEUE_H_
