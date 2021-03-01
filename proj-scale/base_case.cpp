#include <ctime>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
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

void perform_test(uint32_t workload_size, bool block_before_test)
{
	// create data set
	std::vector<thing> data_set;
	make_data_set(data_set, workload_size);
	
	// block before test so the user has a chance to pin
	if (block_before_test)
	{
		puts("press enter to continue with the test");
		getchar();
	}
	
	std::vector<thing> result;
	result.reserve(workload_size);
	
	// use wall clock time
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	
	for (auto& n : data_set)
		result.push_back(n);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	// calculate wall clock time passed
	double time_spent = (end.tv_sec - begin.tv_sec);
	time_spent += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
	
	// make sure vectors are the same as the data set
	if (!(result == data_set))
	{
		print_vect("data_set", data_set);
		print_vect("vec", result);
		equit("data set different then the result; queue bug?");
	}
	
	// report
	printf("threads 1 workload %u time_(sec) %.4f\n",
		workload_size, time_spent
	);
}

bool get_uint(const char * str, uint32_t * out)
{
	return (1 == sscanf(str, "%u", out));
}
void print_use(const char * pname)
{
	fprintf(stderr, "Use: %s <workload>\n", pname);
}

int main(int argc, char * argv[])
{
	uint32_t workload_size;
	
	if (argc < 2)
	{
		print_use(argv[0]);
		equit("bad args");
	}
	else
	{
		if (!get_uint(argv[1], &workload_size))
		{
			print_use(argv[0]);
			equit("bad number");
		}
	}

	perform_test(workload_size, (argc > 2));
	
	return 0;
}
