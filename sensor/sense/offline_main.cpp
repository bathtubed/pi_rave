#include "circular_queue.h"

#include <algorithm>
#include <iterator>
#include <iostream>

int main()
{
	circular_queue<int> q(10);
	
	for(auto i = 0; i < 5; i++)
		q.push_back(i);
	
	auto print_q = [&]{
		std::copy(q.begin(), q.end(), std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;
	};
	
	print_q();
	
	for(auto i = 0; i < 3; i++)
		std::cout << q.pop_front() << " ";
	
	std::cout << std::endl;
	print_q();
	
	for(auto i = 0; i < 7; i++)
		q.push_back(i);
	
	print_q();
	
	for(auto i = 0; i < 18; i++)
		q.push_back(i);
	
	print_q();
	
	return 0;
}

