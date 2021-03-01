#include "process.hpp"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>

struct thing {
	thing() : x(0) {}
	thing(int x) : x(x) {}
	
	int x;
	//char pad[64-sizeof(x)];
};

bool operator==(const thing& lhs, const thing& rhs)
{
	return (lhs.x == rhs.x);
}

void make_data_set(std::vector<thing>& out, uint32_t workload_size)
{
	out.clear();
	for (uint32_t i = 0; i < workload_size; ++i)
		out.push_back(thing(i));
}

void print_vect(const char * name, std::vector<thing>& v)
{
	printf("%s: ", name);
	for (auto& n : v)
		printf("%d ", n.x);
	putchar('\n');
}

void equit(const char * err)
{
	fprintf(stderr, "error: %s\n", err);
	exit(EXIT_FAILURE);
}

void perform_test(int num_of_proc,
	uint32_t qsize,
	uint32_t workload_size,
	bool block_before_test
)
{
	// create data set
	std::vector<thing> data_set;
	make_data_set(data_set, workload_size);
	
	// create vectors for the processors
	std::vector<std::vector<thing>> all_vecs;
	for (int i = 0; i < num_of_proc; ++i)
		all_vecs.push_back(std::vector<thing>());
	
	// reserve capacity
	for (auto& v : all_vecs)
		v.reserve(workload_size);
	
	// create processors and give them the vectors
	std::vector<std::unique_ptr<process<thing>>> all_procs;
	for (int i = 0; i < num_of_proc; ++i)
	{
		all_procs.push_back(
			std::make_unique<process<thing>>(all_vecs[i], qsize)
		);
	}
	
	// block before test so the user has a chance to pin
	if (block_before_test)
	{
		puts("press enter to continue with the test");
		getchar();
	}
	
	// use wall clock time
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	
	// feed all processors one element at a time round-robin
	for (auto& n : data_set)
	{
		for (int i = 0; i < num_of_proc; ++i)
			all_procs[i]->give(&n);
	}
	
	uint32_t wasnt_ready = 0;
	volatile size_t vsz = 0; 
	for (int i = 0; i < num_of_proc; ++i)
	{	
		vsz = all_vecs[i].size();
		if (vsz != workload_size)
		{
			// ugly way to make sure the threads are done writing
			++wasnt_ready;
			--i;
			continue;
		}
		
		// stop processors
		all_procs[i]->stop();
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	// calculate wall clock time passed
	double time_spent = (end.tv_sec - begin.tv_sec);
	time_spent += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
	
	// wait for everyone to join
	for (int i = 0; i < num_of_proc; ++i)
		all_procs[i]->join();
	
	// make sure all processor vectors are the same as the data set
	for (auto& vec : all_vecs)
	{
		if (!(vec == data_set))
		{
			print_vect("data_set", data_set);
			print_vect("vec", vec);
			equit("data set different then the result; queue bug?");
		}
	}
	
	// report
	printf("threads %u qsize %u workload %u wasnt_ready %u time_(sec) %.4f\n",
		num_of_proc, qsize, workload_size, wasnt_ready, time_spent
	);
}

bool get_int(const char * str, int * out)
{
	return (1 == sscanf(str, "%d", out));
}
bool get_uint(const char * str, uint32_t * out)
{
	return (1 == sscanf(str, "%u", out));
}
void print_use(const char * pname)
{
	fprintf(stderr, "Use: %s <num-of-threads> <q-size> <workload>\n", pname);
}

int main(int argc, char * argv[])
{
	int num_of_proc;
	uint32_t qsize;
	uint32_t workload_size;
	
	if (argc < 4)
	{
		print_use(argv[0]);
		equit("bad args");
	}
	else
	{
		if (!(get_int(argv[1], &num_of_proc) &&
			get_uint(argv[2], &qsize) &&
			get_uint(argv[3], &workload_size)))
		{
			print_use(argv[0]);
			equit("bad number");
		}
	}

	perform_test(num_of_proc, qsize, workload_size, (argc > 4));
	
	return 0;
}
