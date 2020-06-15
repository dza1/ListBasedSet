
/** @file benchmark.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Functions for benchmarking
 */
#include "benchmark.hpp"
#include "stdio.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;



/**
 * @brief Write results to the according file
 * @details Create a folder with the name of the constant FOLDER_RESULT
 *
 * @param[in]   name[30]	  	name of the datastructure
 * @param[in]  benchMark  		a struct, which stores information for benchmarking
 * @param[in]  testSizePre 		amount of entries in the pre file
 * @param[in]  testSizeMain		amount of entries in the main file
 */
int write_csv(const char name[30], benchMark_t benchMark, uint32_t testSizePre, uint32_t testSizeMain) {

	mkdir(FOLDER_RESULT, 0770);
	char fullName[40] = FOLDER_RESULT;
	strcat(fullName, "/");	// append string two to the result.
	strncat(fullName, name, 30); // append string two to the result.
	strcat(fullName, ".csv");	// append string two to the result.

	ifstream file_check(fullName);
	ofstream outFile;
	bool exist = file_check.is_open();

	if (exist == true) {
		file_check.close();
	}
	outFile.open(fullName, fstream::app);
	if (outFile.is_open() == false) {
		cerr << "Unable to open file" << endl;
		return 0;
	}
	if (exist == false) {
		WRITE_HEADER(outFile);
	}
	BENCHM_TO_CSV(outFile, benchMark, testSizePre, testSizeMain);
	outFile.close();
	return 1;
}

/**
 * @brief Calculates the average of repeated benchmark tests
 *
 * @param[in]   benchmark[]  	Array of benchmarks, from which the mean should be calculated
 * @param[out]  averBench  		benchmark struct, which includes the mean values
 */
void averBenchm(benchMark_t benchmark[], benchMark_t *averBench) {

	uint64_t preTime = 0;
	uint64_t pregoTo = 0;
	uint64_t prelostTime = 0;
	uint64_t precores = 0;

	uint64_t mainTime = 0;
	uint64_t maingoTo = 0;
	uint64_t mainlostTime = 0;
	uint16_t maincores = 0;

	uint64_t checkTime = 0;
	uint64_t checkgoTo = 0;
	uint64_t checklostTime = 0;
	uint16_t checkcores = 0;
	for (size_t i = 0; i < REPEAT_TESTS; i++) {
		preTime += benchmark[i].pre.time;
		pregoTo += benchmark[i].pre.goToStart;
		prelostTime += benchmark[i].pre.lostTime;
		precores += benchmark[i].pre.cores;
		precores += benchmark[i].pre.cores;

		mainTime += benchmark[i].main.time;
		maingoTo += benchmark[i].main.goToStart;
		mainlostTime += benchmark[i].main.lostTime;
		maincores += benchmark[i].main.cores;

		checkTime += benchmark[i].check.time;
		checkgoTo += benchmark[i].check.goToStart;
		checklostTime += benchmark[i].check.lostTime;
		checkcores += benchmark[i].check.cores;
	}
	averBench->pre.time = preTime / REPEAT_TESTS;
	averBench->pre.goToStart = pregoTo / REPEAT_TESTS;
	averBench->pre.lostTime = prelostTime / (REPEAT_TESTS * 1); // convert to sec
	averBench->pre.cores = precores / REPEAT_TESTS;

	averBench->main.time = mainTime / REPEAT_TESTS;
	averBench->main.goToStart = maingoTo / REPEAT_TESTS;
	averBench->main.lostTime = mainlostTime / (REPEAT_TESTS * 1); // convert to sec
	averBench->main.cores = maincores / REPEAT_TESTS;

	averBench->check.time = checkTime / REPEAT_TESTS;
	averBench->check.goToStart = checkgoTo / REPEAT_TESTS;
	averBench->check.lostTime = checklostTime / (REPEAT_TESTS * 1); // convert to sec
	averBench->check.cores = checkcores / REPEAT_TESTS;
}