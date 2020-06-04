/** @file benchmark.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Precompiler constants and Makros for benchmarking
 */
#ifndef BENCHMARK_H__
#define BENCHMARK_H__

#include <stdint.h>

#define REPEAT_TESTS 2		//Amount of iterations for each test

//empty sub_benchMark_t struct
#define SUB_BENCHMARK_E                                                                                                \
	{ 0, 0 ,0}

//empty benchMark_t struct
#define BENCHMARK_E                                                                                                    \
	{ SUB_BENCHMARK_E, SUB_BENCHMARK_E, SUB_BENCHMARK_E }

#define BENCHM_TO_CSV(file, name, str, testSizePre, testSizeMain)                                                                          \
	(file << name << "," << str.main.cores << "," << testSizePre << ","<<testSizeMain<<',' << str.pre.time << "," << str.pre.goToStart << ","     \
		  << str.main.time << "," << str.main.goToStart << "," << str.check.time << "," << str.check.goToStart         \
		  << endl)

#define WRITE_HEADER(file)                                                                                             \
	file << "Art of list,cores,TestSizePre,TestSizeMain,time write,goToStart write,time mixed,goToStart mixed,time check,goToStart "   \
			"check \n"


struct sub_benchMark_t {
	uint32_t goToStart;
	uint16_t time;
	uint16_t cores;
};

struct benchMark_t {
	sub_benchMark_t pre;
	sub_benchMark_t main;
	sub_benchMark_t check;
};

void averBenchm(benchMark_t benchmark[], benchMark_t *averBench);

#endif