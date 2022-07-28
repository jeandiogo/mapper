# Mapper

Mapper is a single header C++20 library for Linux, providing a function that maps (or creates and maps) a file into memory and returns it as a contiguous STL compatible container.

When compiling with g++, use the flag *-std=c++20* (available since GCC 10).

To compile and run the test, enter *make test* in the terminal.

## Usage:
```
#include "mapper.hpp"

int main()
{
	auto my_map = mapper::map("my_file.txt");
	
	//...
}
```
*Copyright (C) 2020 Jean "Jango" Diogo*
