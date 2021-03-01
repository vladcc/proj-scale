#include "spscq.hpp"

#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>

template <typename T>
class process {
	
	public:
	process(std::vector<T> & output, uint32_t qsize) :
		_queue(qsize),
		_output(output),
		_work(true)
	{
		_thread = std::thread{&process::run, this};
	}
	
	void run()
	{
		T mine;
		while (_work)
		{
			while (_queue.read(&mine))
				_output.push_back(mine);
		}
	}
	
	inline void give(T * what)
	{
		while (!_queue.write(what))
			continue;
	}
	
	inline void join()
	{
		_thread.join();
	}
	
	void stop() {_work = false;}
	
	private:
	spscq<T> _queue;
	std::thread _thread;
	std::vector<T> & _output;
	std::atomic<bool> _work;
};
