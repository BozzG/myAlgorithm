#include <algorithm>
#include <numeric>
#include <queue>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <climits>
#include <random>

typedef int stype;

template<class T>
class maxheap
{
public:
	maxheap(int maxk)
	{
		size = 0;
		a = (T*)malloc(sizeof(T) * (2*maxk + 2));
		a[0] = INT_MAX;
		for (int i = 0; i <= maxk; i++)
			a[maxk + i] = INT_MIN;
	}

	~maxheap() { free(a); }
	T top() const { return a[1]; }

	void push(const T x)
	{
		int c = ++size;
		int p = size / 2;

		while (x > a[p] /*&& p >= 1*/) {
			a[c] = a[p];
			c = p;
			p /= 2;
		}
		a[c] = x;
	}

	void pop()
	{
		const T x = a[size--];
		int p = 1, c = 1;

		while (x < a[c]/* && c <= size &&*/) {
			a[p] = a[c];
			p = c;
			c *= 2;
			if (a[c + 1] > a[c])
				c++;
		}
		a[p] = x;
	}

//private:

	T *a;
	int size;
};

void rand_swap(stype a[], int n, int k)
{
	const int step = n / k;
//	std::sort(a, a + k);
	for (int i = 1; i < k; i ++) {
#if 0
		int h = rand() % k, t = i * step + rand() % step;
		if (a[h] > a[t])
			std::swap(a[h], a[t]);
		else if (a[i] > a[t])
			std::swap(a[i], a[t]);
#endif
	}
}

void reset(stype a[], int n)
{
	memcpy(a, a + n, n * sizeof(a[0]));
	rand_swap(a, n, 10000);
}

#if __linux__
#include <sys/resource.h>
#elif _WIN32
#include <windows.h>
#endif

static clock_t getTime()
{
#if 0
	FILETIME ptime[4] = {0};
	GetThreadTimes(GetCurrentThread(), &ptime[0], &ptime[1], &ptime[2], &ptime[3]);
	return (ptime[2].dwLowDateTime + ptime[3].dwLowDateTime) / 10000;
	//return clock();
#elif 0
	struct rusage rup;
	getrusage(RUSAGE_SELF, &rup);
	long sec  = rup.ru_utime.tv_sec  + rup.ru_stime.tv_sec;
	long usec = rup.ru_utime.tv_usec + rup.ru_stime.tv_usec;
	return sec * 1000 + usec / 1000;
#elif __linux__ || __unix__
	return clock() / 1000;
#else
	return clock();
#endif
}

void stl_sort(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	std::sort(a, a + n);
	printf("stl sort       %4ld ms, a[%d] = %d\n\n", getTime() - ts, k, a[k - 1]);
}

void stl_nth(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	std::nth_element(a, a + k, a + n);
	stype maxe = *std::max_element(a, a + k);
	stype sum =  std::accumulate(a, a + k, 0);
	printf("stl nth_element  %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

void stl_makeheap(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	std::make_heap(a, a + k);

	stype maxe = a[0];
	for (int i = k; i < n; ++i) {
		if (a[i] < maxe) {
			std::pop_heap(a, a + k);
			a[k - 1] = a[i];
			std::push_heap(a, a + k);
			maxe = a[0];
		}
	}

	stype sum = std::accumulate(a, a + k, 0);
	printf("stl make_heap   %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

void stl_priqueue(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	std::priority_queue<stype> pri_queue;
	for (int m = 0; m < k; m++)
		pri_queue.push(a[m]);

	stype maxe = pri_queue.top();
	for (int i = k; i < n; i++) {
		if (a[i] < maxe) {
			pri_queue.pop();
			pri_queue.push(a[i]);
			maxe = pri_queue.top();
		}
	}

	stype sum = 0;
	for (int j = 0; j < k; j ++) {
		sum += pri_queue.top();
		pri_queue.pop();
	}

	printf("stl pri_queue   %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

void max_heap(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	maxheap<int> my_heap(k);
	for (int i = 0; i < k; i++)
		my_heap.push(a[i]);

//    std::sort(a, a + k);

	stype maxe = my_heap.top();
	for (int j = k; j < n; j++) {
		if (a[j] < maxe) {
			my_heap.pop();
			my_heap.push(a[j]);
			maxe = my_heap.top();
		}
	}

	stype sum = std::accumulate(my_heap.a + 1, my_heap.a + k + 1, 0);
	printf("my max_heap     %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

void bucket_sort(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	const unsigned int segment = 20;
	const unsigned int segsize = 1 << (32 - segment);
	int bucket[segsize] = {0}, checks = 0; //32k
	for (int m = 0; m < n; m++) {
		const int bindex = a[m] >> segment;
		bucket[bindex] ++;
		if (bindex < 4 && checks ++ >= k)
			break;
	}

	stype maxe = 1 << segment;
	int i = 0, j = 0, bsize = 0;
	for (i = 0, bsize = 0; i < segsize; i++) {
		bsize += bucket[i];
		if (bsize >= k) {
			maxe = (i + 1) << segment;
			break;
		}
	}

	for (i = 0; i < n; i++) {
		if (a[i] < maxe) {
			a[j++] = a[i];
			if (j >= k * 2) {
				std::sort(a, a + j);
				maxe = a[(j = k) - 1];
			}
		}
	}

	std::sort(a, a + j);
	stype sum = std::accumulate(a, a + k, 0);
	printf("bucket_sort      %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, j, a[k - 1], sum);
}

void swap_array(stype a[], const int k)
{
	for (int i = 0; i < k / 2; i ++)
		std::swap(a[i], a[k - i - 1]);
}

void merge_sort(stype a[], int n, const int k)
{
	reset(a, n);
	clock_t ts = getTime();

	std::sort(a, a + k);
	stype* best_a = a + k;
	stype maxe = a[k - 1];
	int auxn = k / 20 + 10, bestn = 0;

	for (int i = k; i < n; i++) {
		if (a[i] < maxe) {
			best_a[bestn++] = a[i];
			if (bestn >= auxn) {
				std::sort(best_a, best_a + bestn);
				if (a[0] >= best_a[bestn - 1]) {
#ifdef SW
					swap_array(a, k); swap_array(best_a, bestn); swap_array(a, k + bestn);
#else
					memmove(a + bestn, a, sizeof(a[0]) * (k - bestn)); memcpy(a, best_a, sizeof(a[0]) * bestn);
#endif
				} else
					std::inplace_merge(a, best_a, best_a + bestn);
				maxe = a[k - 1];
				bestn = 0;
			}
		}
	}

	std::sort(a, a + k + bestn);
	maxe = a[k - 1];
	stype sum = std::accumulate(a, a + k, 0);
	printf("sort-merge      %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

void merge_array2(stype a[], stype b[], const int k)
{
	std::sort(b, b + k);
	if (a[0] >= b[k - 1]) {
		memcpy(a, b, sizeof(stype) * k);
		return;
	}

	int i = k / 10 * 6 + 16, s = 16;
	int j = k - 1 + s;

	if (a[k / 2] > b[k / 2])
		i = k / 2;
	else if (j < i || a[i - s] <= b[j - i])
		i = k - 1;
	while (i > s && a[i - s] > b[j - i])
		i -= s;
	for (; i >= 0; i--) {
		j = k - 1 - i;
		if (a[i] > b[j])
			continue ;

		//merge a[0, i]/b[0, j - 1]
#ifndef ME
		int m = k - 1; j --;
		while (j >= 0)
			a[m --] = a[i] <= b[j] ? b[j --] : a[i --];
#else
		memcpy(a + i + 1, b, sizeof(stype) * j);
		std::inplace_merge(a, a + i + 1, a + k);
#endif
		return;
	}
}

void merge_sort2(stype a[], int n, const int k)
{
	reset(a, n);

	clock_t ts = getTime();
	std::sort(a , a + k);
	a[-1] = INT_MIN;

	stype* best_a = a + k;
	stype maxe = a[k - 1]; int bestn = 0;
	for (int i = k; i < n; i++) {
		if (a[i] < maxe) {
			best_a[bestn++] = a[i];
			if (bestn == k) {
				merge_array2(a, best_a, k);
				maxe = a[k - 1];
				bestn = 0;
			}
		}
	}

	std::sort(a, best_a + bestn);
	maxe = a[k - 1];
	stype sum = std::accumulate(a, a + k, 0);
	printf("sort-merge2     %4ld ms, a[%d] = %d, sum = %d\n", getTime() - ts, k, maxe, sum);
}

static void printInfo()
{
	const char* sepator =
		"------------------------------------------------------------------------------------------------------------";
	puts(sepator);
//	puts("Copyright (C) by 2018-2020 Huang Yuanbing 22738078@qq.com/bailuzhou@163.com\n");

	char cbuff[500];
	char* info = cbuff;
#ifdef __clang__
	info += sprintf(info, "clang %s", __clang_version__); //vc/gcc/llvm
#if __llvm__
	info += sprintf(info, " on llvm/");
#endif
#endif

#if _MSC_VER
	info += sprintf(info, "Compiled by vc++ %d", _MSC_VER);
#elif __GNUC__
	info += sprintf(info, "Compiled by gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif __INTEL_COMPILER
	info += sprintf(info, "Compiled by intel c++ %d", __INTEL_COMPILER);
#endif

#if __cplusplus
	info += sprintf(info, " __cplusplus = %d", (int)__cplusplus);
#endif

#if __x86_64__ || __amd64__ || _M_X64 || __amd64 || __x86_64
	info += sprintf(info, " x86-64");
#elif __i386__ || _M_IX86 || _X86_ || __i386
	info += sprintf(info, " x86");
#elif __arm64__
	info += sprintf(info, " arm64");
#elif __arm__
	info += sprintf(info, " arm");
#else
	info += sprintf(info, " unknow");
#endif

	puts(cbuff);
	puts(sepator);
}

#define  MAXN  10000*10000
int main(int argc, char* argv[])
{
	srand(time(NULL));
	printf("\ncmd:topk k(<=%d) n(<=%d) [type = 0 rand,1 increase,2 wavy,3 wavy, 4,5 rand, 6 decrease]\n\n", 1000000, MAXN);
	printInfo();

	int maxn = MAXN , k = MAXN / 10000, type = 0;
	if (argc > 1) { k = atoi(argv[1]); }
	if (argc > 2) {
		int ration = atoi(argv[2]);
		if (ration >= -100 && ration < 0) maxn = MAXN / (-ration);
		if (ration <= 100 && ration > 0) maxn = MAXN * ration;
		else if (ration > 0) maxn = ration;
	}

	stype* arr = (stype *)malloc(sizeof(stype) * maxn * 2 + 8) + 2;
	stype* buff = arr + maxn;

	std::default_random_engine e(time(NULL));
	std::mt19937 rng; rng.seed(time(NULL));
	std::uniform_int_distribution<unsigned int> u(0, 1 << 30);
	std::normal_distribution<> d(1 << 25, 1 << 10);

	for (int j = 0; j <= 6; j ++) {
		stype s = rand(), r = 0;
		type = j;
		for (stype i = 0; i < maxn; i++) {
//			if (i % RAND_MAX == 0) srand(time(NULL));
			if (type == 0)
				r = u(rng);
			else if (type == 1)
				r = d(rng);
			else if (type == 2)
				r = s * rand() + i;
			else if (type == 3)
				r = (s - i) * i;
			else if (type == 4)
				r = rng();
			else if (type == 5)
				r = e();
			else if (type == 6)
				r = maxn - i; // i + 1

			if (r < 0)
				r = -r;
			buff[i] = r;
		}

		printf("maxn = %d, topk = %d, type = %d\n", maxn, k, type);
#if __cplusplus
		stl_nth(arr, maxn, k);
		if (sizeof(stype) <= sizeof(int)) bucket_sort(arr, maxn, k);
		max_heap(arr, maxn, k);
		stl_priqueue(arr, maxn, k);
		stl_makeheap(arr, maxn, k);
#endif
		merge_sort(arr, maxn, k);
		merge_sort2(arr, maxn, k);
		putchar('\n'); putchar('\n');
	}

	free(arr - 2);
	return 0;
}

// https://www.jdoodle.com/online-compiler-c++
// https://repl.it/repls/ProductivePowerlessSlope
// https://www.tutorialspoint.com/online_cpp_compiler.php
// http://rextester.com/l/cpp_online_compiler_gcc
// https://www.onlinegdb.com/
// https://www.ideone.com/3rCdob

