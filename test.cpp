#include "mapper.hpp"

#include <algorithm>
#include <iostream>

int main()
{
	auto my_map = mapper::map<int>("my_map.txt", 30);
	
	for(std::size_t i = 0; i < my_map.size(); ++i)
	{
		my_map[i] = static_cast<int>(i);
	}
	
	std::sort(my_map.rbegin(), my_map.rend());
	
	for(auto const & item : my_map)
	{
		std::cout << item << ' ';
	}
	std::cout << '\n';
}