////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Mapper (C++ Library)
// Copyright (C) 2020 Jean "Jango" Diogo <jeandiogo@gmail.com>
// 
// Licensed under the Apache License Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>.
// 
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// mapper.hpp
// 
// Mapper is a single header C++20 library for Linux, providing a function that maps (or creates and maps) a file into memory and returns it as a contiguous STL compatible container.
// 
// Usage:
// 
// #include "mapper.hpp"
// 
// mapper::map_t my_map = mapper::map("a.txt");                 //maps a file to memory and returns a container that behaves like an array of unsigned chars
// mapper::map_t my_map = mapper::map<char>("a.txt");           //the type underlying the array can be chosen at instantiation via template argument
// mapper::map_t my_map = mapper::map<int>("a.txt");            //note that trailing bytes will be ignored if the size of the file is not a multiple of the size of the chosen type
// mapper::map_t my_map = mapper::map("a.txt", 20);             //creates or overwrites file "a.txt" and loads it a empty map of 20 bytes
// mapper::map_t my_map = mapper::map<int>("a.txt", 30);        //creates or overwrites file "a.txt" and loads it a empty map of 30 * sizeof(int) bytes
// 
// unsigned char * my_data  = my_map.get_data();                //gets a raw pointer to file's data, whose underlying type is the one designated at instantiation (default is unsigned char)
// unsigned char * my_data  = my_map.data();                    //same as above, for STL compatibility
// std::size_t     my_size  = my_map.get_size();                //gets data size (which is equals to size of file divided by the size of the type) 
// std::size_t     my_size  = my_map.size();                    //same as above, for STL compatibility
// bool            is_empty = my_map.is_empty();                //returns true if map is ampty
// bool            is_empty = my_map.empty();                   //same as above, for STL compatibility
// bool            success  = my_map.flush();                   //flushes data to file
// unsigned char   my_var   = my_map.at(N);                     //gets the N-th element, throws if N is greater than or equal to "size()"
// unsigned char   my_var   = my_map[N];                        //same as above, but does not check range
// my_map.at(N) = V;                                            //assigns the value V to the N-th element, throws if N is greater than or equal to "size()"
// my_map[N]    = V;                                            //same as above, but does not check range
// 
// mapper::map_t::iterator               it = my_map.begin();   //returns a standard iterator to my map (same for "end")
// mapper::map_t::const_iterator         it = my_map.cbegin();  //returns a standard iterator to my map (same for "cend")
// mapper::map_t::reverse_iterator       it = my_map.rbegin();  //returns a standard iterator to my map (same for "rend")
// mapper::map_t::const_reverse_iterator it = my_map.crbegin(); //returns a standard iterator to my map (same for "crend")
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <cstddef>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

namespace mapper
{
	template <typename data_t>
	class [[nodiscard]] map_t
	{
		struct iterator_t
		{
			using iterator_type     = iterator_t;
			using iterator_category = std::contiguous_iterator_tag;
			using difference_type   = std::ptrdiff_t;
			using value_type        = data_t;
			using pointer           = value_type *;
			using reference         = value_type &;
			
			pointer data_ptr = nullptr;
			
			iterator_t(pointer _data_ptr) : data_ptr(_data_ptr)
			{
				if(!data_ptr)
				{
					throw std::runtime_error("cannot create iterator from null pointer");
				}
			}
			
			~iterator_t()
			{
				data_ptr = nullptr;
			}
			
			reference operator*()
			{
				return *data_ptr;
			}
			
			pointer operator->()
			{
				return data_ptr;
			}
			
			auto & operator++()
			{
				++data_ptr;
				return *this;
			}
			
			auto operator++(int)
			{
				auto other = *this;
				++(*this);
				return other;
			}
			
			auto & operator--()
			{
				--data_ptr;
				return *this;
			}
			
			auto operator--(int)
			{
				auto other = *this;
				--(*this);
				return other;
			}
			
			auto & operator+=(difference_type const steps)
			{
				data_ptr += steps;
				return *this;
			}
			
			auto & operator-=(difference_type const steps)
			{
				data_ptr -= steps;
				return *this;
			}
			
			auto operator+(difference_type const steps) const
			{
				return iterator_t(data_ptr + steps);
			}
			
			auto operator-(difference_type const steps) const
			{
				return iterator_t(data_ptr - steps);
			}
			
			auto operator-(iterator_type const other) const
			{
				return data_ptr - other.data_ptr;
			}
			
			auto operator==(iterator_type const & other) const
			{
				return data_ptr == other.data_ptr;
			}
			
			auto operator!=(iterator_type const & other) const
			{
				return data_ptr != other.data_ptr;
			}
			
			auto operator>=(iterator_type const & other) const
			{
				return data_ptr >= other.data_ptr;
			}
			
			auto operator<=(iterator_type const & other) const
			{
				return data_ptr <= other.data_ptr;
			}
			
			auto operator>(iterator_type const & other) const
			{
				return data_ptr > other.data_ptr;
			}
			
			auto operator<(iterator_type const & other) const
			{
				return data_ptr < other.data_ptr;
			}
		};
		
		struct const_iterator_t
		{
			using iterator_type     = const_iterator_t;
			using iterator_category = std::contiguous_iterator_tag;
			using difference_type   = std::ptrdiff_t;
			using value_type        = data_t;
			using pointer           = value_type const *;
			using reference         = value_type const &;
			
			pointer data_ptr = nullptr;
			
			const_iterator_t(pointer _data_ptr) : data_ptr(_data_ptr)
			{
				if(!data_ptr)
				{
					throw std::runtime_error("cannot create iterator over null pointer");
				}
			}
			
			~const_iterator_t()
			{
				data_ptr = nullptr;
			}
			
			reference operator*()
			{
				return *data_ptr;
			}
			
			pointer operator->()
			{
				return data_ptr;
			}
			
			auto & operator++()
			{
				++data_ptr;
				return *this;
			}
			
			auto operator++(int)
			{
				auto other = *this;
				++(*this);
				return other;
			}
			
			auto & operator--()
			{
				--data_ptr;
				return *this;
			}
			
			auto operator--(int)
			{
				auto other = *this;
				--(*this);
				return other;
			}
			
			auto & operator+=(difference_type const steps)
			{
				data_ptr += steps;
				return *this;
			}
			
			auto & operator-=(difference_type const steps)
			{
				data_ptr -= steps;
				return *this;
			}
			
			auto operator+(difference_type const steps) const
			{
				return const_iterator_t(data_ptr + steps);
			}
			
			auto operator-(difference_type const steps) const
			{
				return const_iterator_t(data_ptr - steps);
			}
			
			auto operator-(iterator_type const other) const
			{
				return data_ptr - other.data_ptr;
			}
			
			auto operator==(iterator_type const & other) const
			{
				return data_ptr == other.data_ptr;
			}
			
			auto operator!=(iterator_type const & other) const
			{
				return data_ptr != other.data_ptr;
			}
			
			auto operator>=(iterator_type const & other) const
			{
				return data_ptr >= other.data_ptr;
			}
			
			auto operator<=(iterator_type const & other) const
			{
				return data_ptr <= other.data_ptr;
			}
			
			auto operator>(iterator_type const & other) const
			{
				return data_ptr > other.data_ptr;
			}
			
			auto operator<(iterator_type const & other) const
			{
				return data_ptr < other.data_ptr;
			}
		};
		
		int descriptor = -1;
		std::size_t data_size = 0;
		data_t * data_ptr = nullptr;
		
		public:
		
		map_t(map_t &) = delete;
		map_t(map_t &&) = delete;
		auto & operator=(map_t const &) = delete;
		auto & operator=(map_t &&) = delete;
		auto operator&() = delete;
		
		map_t(std::string const & filename, std::size_t const file_size)
		{
			if(file_size > 0)
			{
				auto output = std::ofstream(filename, std::fstream::binary);
				if(!output.good())
				{
					throw std::runtime_error("could not open file '" + filename + "'");
				}
				auto const data = std::vector<data_t>(file_size, {});
				output.write(reinterpret_cast<char const *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(data_t)));
				output.flush();
			}
			descriptor = ::open(filename.c_str(), O_RDWR, 0666);
			try
			{
				struct ::stat file_status;
				if(::fstat(descriptor, &file_status) < 0)
				{
					throw std::runtime_error("could not get status of '" + filename + "'");
				}
				data_size = static_cast<std::size_t>(file_status.st_size / static_cast<off_t>(sizeof(data_t)));
				data_ptr = static_cast<data_t *>(::mmap(nullptr, data_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, descriptor, 0));
				if(data_ptr == MAP_FAILED)
				{
					throw std::runtime_error("could not map file '" + filename + "'");
				}
			}
			catch(...)
			{
				::close(descriptor);
				throw;
			}
		}
		
		~map_t()
		{
			::msync(data_ptr, data_size, MS_SYNC);
			::munmap(data_ptr, data_size);
			::close(descriptor);
			descriptor = -1;
			data_size = 0;
			data_ptr = nullptr;
		}
		
		auto & operator[](std::size_t const index)
		{
			return data_ptr[index];
		}
		
		auto const & operator[](std::size_t const index) const
		{
			return data_ptr[index];
		}
		
		auto & at(std::size_t const index)
		{
			if(index >= data_size)
			{
				throw std::runtime_error("index " + std::to_string(index) + " is out of the range");
			}
			return data_ptr[index];
		}
		
		auto const & at(std::size_t const index) const
		{
			if(index >= data_size)
			{
				throw std::runtime_error("index " + std::to_string(index) + " is out of the range");
			}
			return data_ptr[index];
		}
		
		auto data()
		{
			return data_ptr;
		}
		
		data_t const * data() const
		{
			return data_ptr;
		}
		
		auto empty() const
		{
			return (data_size == 0);
		}
		
		auto flush()
		{
			return (::msync(data_ptr, data_size, MS_SYNC) >= 0);
		}
		
		auto get_data()
		{
			return data_ptr;
		}
		
		data_t const * get_data() const
		{
			return data_ptr;
		}
		
		auto get_size() const
		{
			return data_size;
		}
		
		auto is_empty() const
		{
			return (data_size == 0);
		}
		
		auto size() const
		{
			return data_size;
		}
		
		using iterator               = iterator_t;
		using const_iterator         = const_iterator_t;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		
		inline auto begin()
		{
			return !data_ptr ? iterator(nullptr) : iterator(&data_ptr[0]);
		}
		
		inline auto end()
		{
			return !data_ptr ? iterator(nullptr) : iterator(&data_ptr[0]) + static_cast<iterator::difference_type>(data_size);
		}
		
		inline auto cbegin() const
		{
			return !data_ptr ? const_iterator(nullptr) : const_iterator(&data_ptr[0]);
		}
		
		inline auto cend() const
		{
			return !data_ptr ? const_iterator(nullptr) : const_iterator(&data_ptr[0]) + static_cast<iterator::difference_type>(data_size);
		}
		
		inline auto rbegin()
		{
			return std::make_reverse_iterator(end());
		}
		
		inline auto rend()
		{
			return std::make_reverse_iterator(begin());
		}
		
		inline auto crbegin() const
		{
			return std::make_reverse_iterator(cend());
		}
		
		inline auto crend() const
		{
			return std::make_reverse_iterator(cbegin());
		}
		
		inline auto begin() const
		{
			return cbegin();
		}
		
		inline auto end() const
		{
			return cend();
		}
		
		inline auto rbegin() const
		{
			return crbegin();
		}
		
		inline auto rend() const
		{
			return crend();
		}
		
		friend auto begin(map_t & me)
		{
			return me.begin();
		}

		friend auto begin(map_t const & me)
		{
			return me.cbegin();
		}

		friend auto cbegin(map_t const & me)
		{
			return me.cbegin();
		}

		friend auto rbegin(map_t & me)
		{
			return me.rbegin();
		}

		friend auto rbegin(map_t const & me)
		{
			return me.crbegin();
		}

		friend auto crbegin(map_t const & me)
		{
			return me.crbegin();
		}
		
		friend auto end(map_t & me)
		{
			return me.end();
		}

		friend auto end(map_t const & me)
		{
			return me.cend();
		}

		friend auto cend(map_t const & me)
		{
			return me.cend();
		}

		friend auto rend(map_t & me)
		{
			return me.rend();
		}

		friend auto rend(map_t const & me)
		{
			return me.crend();
		}

		friend auto crend(map_t const & me)
		{
			return me.crend();
		}
		
		friend auto operator+(iterator::difference_type const steps, iterator const & it)
		{
			return it + steps;
		}

		friend auto operator+(const_iterator::difference_type const steps, const_iterator const & it)
		{
			return it + steps;
		}
		
		friend auto advance(iterator & me, iterator::difference_type const steps = 0)
		{
			me += steps;
		}

		friend auto advance(const_iterator & me, const_iterator::difference_type const steps = 0)
		{
			me += steps;
		}
		
		friend auto distance(iterator const & lhs, iterator const & rhs)
		{
			return lhs - rhs;
		}

		friend auto distance(const_iterator const & lhs, const_iterator const & rhs)
		{
			return lhs - rhs;
		}
		
		friend auto next(iterator const & it, iterator::difference_type const steps = 0)
		{
			return it + steps;
		}

		friend auto next(const_iterator const & it, const_iterator::difference_type const steps = 0)
		{
			return it + steps;
		}
		
		friend auto prev(iterator const & it, iterator::difference_type const steps = 0)
		{
			return it - steps;
		}

		friend auto prev(const_iterator const & it, const_iterator::difference_type const steps = 0)
		{
			return it - steps;
		}
	};

	template <typename data_t = unsigned char>
	auto map(std::string const & filename, std::size_t const size = {})
	{
		return map_t<data_t>(filename, size);
	}
}

#endif