#ifndef SPSCQ_HPP
#define SPSCQ_HPP

#include <cstdint>
#include <cstdlib>
#include <atomic>

/*
References:
https://kceiw.me/lock-free-ring-buffer/
http://daugaard.org/blog/writing-a-fast-and-versatile-spsc-ring-buffer/
*/

/*
note: _size must be > 1
*/

template <typename T>
class spscq {
	
	public:
	spscq(uint32_t size) :
		_size(size), _read_here(0), _write_here(0)
	{
		_data = (T*)malloc(_size*sizeof(T));
	}
	
	~spscq()
	{
		free(_data);
	}
	
	bool write(T * what)
	{
		uint32_t write = _write_here;
		uint32_t read = _read_here;
		
		++write;
		write *= (write != _size);
		
		bool has_space = (write != read);
		
		if (has_space)
		{
			_data[write] = *what;
			_write_here = write;
		}
		
		return has_space;
	}
	
	bool read(T * where)
	{
		uint32_t write = _write_here;
		uint32_t read = _read_here;
		
		bool has_stuff = (read != write);
		
		if (has_stuff)
		{
			++read;
			read *= (read != _size);
			*where = _data[read];
			_read_here = read;
		}
		
		return has_stuff;
	}
	
	
	private:
	
#ifdef PAD
#define CACHE_SIZE 64
	
	T * _data;
	uint32_t _size;
	
	char _pad1[CACHE_SIZE];
	
	std::atomic<uint32_t> _read_here;
	
	char _pad2[CACHE_SIZE];
	
	std::atomic<uint32_t> _write_here;
	
	char _pad3[CACHE_SIZE];

#undef CACHE_SIZE
#else

	T * _data;
	uint32_t _size;
	std::atomic<uint32_t> _read_here;
	std::atomic<uint32_t> _write_here;

#endif

};

#endif
