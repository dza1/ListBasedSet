/** @file benchmark.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Precompiler constants and Makros for benchmarking
 */
#ifndef BENCHMARK_H__
#define BENCHMARK_H__

#include <stdint.h>

#define REPEAT_TESTS 1 // Amount of iterations for each test
#define FOLDER_RESULT "result"

// empty sub_benchMark_t struct
#define SUB_BENCHMARK_E                                                                                                \
	{ 0, 0, 0, 0 }

// empty benchMark_t struct
#define BENCHMARK_E                                                                                                    \
	{ SUB_BENCHMARK_E, SUB_BENCHMARK_E, SUB_BENCHMARK_E }

#define BENCHM_TO_CSV(file,  str, testSizePre, testSizeMain)                                                      \
	(file << str.main.cores << "," << testSizePre << "," << testSizeMain << ',' << str.pre.time << ","  \
		  << str.pre.goToStart << "," << (double)str.pre.lostTime / 1000000 << "," << str.main.time << ","             \
		  << str.main.goToStart << "," << (double)str.main.lostTime / 1000000 << "," << str.check.time << ","          \
		  << str.check.goToStart << "," << (double)str.check.lostTime / 1000000 << endl)

#define WRITE_HEADER(file)                                                                                             \
	file << "cores,TestSizePre,TestSizeMain,time write,goToStart write,lostTime write,time "               \
			"mixed,goToStart mixed,lostTime mixed,time check,goToStart check,"                                         \
			"lostTime check\n"



struct sub_benchMark_t {
	uint32_t goToStart;
	uint32_t lostTime;
	uint16_t time;
	uint16_t cores;
};

struct benchMark_t {
	sub_benchMark_t pre;
	sub_benchMark_t main;
	sub_benchMark_t check;
};

void averBenchm(benchMark_t benchmark[], benchMark_t *averBench);
int write_csv(const char name[30],benchMark_t benchMark,uint32_t testSizePre, uint32_t testSizeMain);

#endif