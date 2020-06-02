#ifndef BENCHMARK_H__
#define BENCHMARK_H__

#include <stdint.h>

#define AMOUNT_TESTCASES 1
#define REPEAT_TESTS 1

#define SUB_BENCHMARK_E                                                                                                \
	{ 0, 0 }

#define BENCHMARK_E                                                                                                    \
	{ SUB_BENCHMARK_E, SUB_BENCHMARK_E, SUB_BENCHMARK_E }

#define BENCHM_TO_CSV(file, name, maxTd, str, testSizePre, testSizeMain)                                                                          \
	(file << name << "," << maxTd << "," << testSizePre << ","<<testSizeMain<<',' << str.pre.time << "," << str.pre.goToStart << ","     \
		  << str.main.time << "," << str.main.goToStart << "," << str.check.time << "," << str.check.goToStart         \
		  << endl)

#define WRITE_HEADER(file)                                                                                             \
	file << "Art of list,cores,TestSizePre,TestSizeMain,time write,goToStart write,time mixed,goToStart mixed,time check,goToStart "   \
			"check \n"

struct sub_benchMark_t {
	uint32_t goToStart;
	uint16_t time;
};

struct benchMark_t {
	sub_benchMark_t pre;
	sub_benchMark_t main;
	sub_benchMark_t check;
};

void averBenchm(benchMark_t benchmark[], benchMark_t *averBench);

#endif