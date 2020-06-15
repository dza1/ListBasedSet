/** @file key.hpp
 * @author Daniel Zainzinger
 * @author Noah Bruns
 * @date 2.6.2020
 *
 * @brief Main file to test the datastructures
 */
#include "Coarse_Grained.hpp"
#include "Fine_Grained.hpp"
#include "Lock_free.hpp"
#include "Lock_free_impr.hpp"
#include "Lock_free_impr_mem.hpp"
#include "Optimistic.hpp"
#include "Optimistic_mem.hpp"
#include "Lazy.hpp"
#include "Lazy_mem.hpp"
#include "benchmark.hpp"
#include "stdio.h"
#include "termcolor.hpp"
#include <chrono>
#include <fstream>
#include <omp.h>
#include <sstream>
#include <vector>
using namespace termcolor;
#include <set>

#define PRE_FILENAME "testcases/pre%zu.csv"
#define MAIN_FILENAME "testcases/main%zu.csv"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static uint32_t read_file(string name, vector<vector<int>> *cases);
static void runtest(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark);
static void check(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark);

/**
 * @brief Entrypoint of the Programm
 * @details calls perform for each datastructure few benchmarks and write the result in a csv file
 * @param [const]	PRE_FILENAME filename of the add testfile
 * @param [const]	MAIN_FILENAME filename of the mixed testfile
 */
int main(int argc, char *argv[]) {
	benchMark_t benchMark;
	benchMark_t benchMark_arr[REPEAT_TESTS];
	ofstream outFile("Test_result.csv", fstream::app);
	if (outFile.is_open()) {
		WRITE_HEADER(outFile);
		SetList<int> *list;
		size_t testCnt = 0;
		while (true) { // Read all available main and pre files
			vector<vector<int>> testcases[2];
			uint32_t testSizePre = 0;
			uint32_t testSizeMain = 0;
			char pre_file[20];
			char main_file[20];
			sprintf(pre_file, PRE_FILENAME, testCnt);
			sprintf(main_file, MAIN_FILENAME, testCnt);

			testSizePre = read_file(pre_file, &testcases[0]);
			testSizeMain = read_file(main_file, &testcases[1]);
			if (testSizePre == 0 || testSizeMain == 0) { // stop, if there is no file to read
				break;
			}
			cout << blue << main_file << endl;

			///////////////// CoarseList///////////////////////
			cout << white << "CoarseList:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new CoarseList<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "CoarseList", benchMark,testSizePre,testSizeMain);

			////////////////////// FineList /////////////////////
			cout << white << "FineList:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new FineList<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "FineList", benchMark,testSizePre,testSizeMain);

			////////////////////// Optimistic /////////////////////
			cout << white << "Optimistic:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new Optimistic<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "Optimistic", benchMark,testSizePre,testSizeMain);

			////////////////////// Optimistic_mem /////////////////////
			cout << white << "Optimistic_mem:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new Optimistic_mem<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark = BENCHMARK_E;
			averBenchm(benchMark_arr, &benchMark);
			BENCHM_TO_CSV(outFile, "Optimistic_mem", benchMark, testSizePre, testSizeMain);

			////////////////////// Lazy /////////////////////
			cout << white << "Lazy:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new Lazy<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark = BENCHMARK_E;
			averBenchm(benchMark_arr, &benchMark);
			BENCHM_TO_CSV(outFile, "Lazy", benchMark, testSizePre, testSizeMain);

			////////////////////// Lazy_mem /////////////////////
			/*cout << white << "Lazy_mem:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new Lazy_mem<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark = BENCHMARK_E;
			averBenchm(benchMark_arr, &benchMark);
			BENCHM_TO_CSV(outFile, "Lazy_mem", benchMark, testSizePre, testSizeMain);*/

			////////////////////// LockFree /////////////////////
			cout << white << "LockFree:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new LockFree<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "LockFree", benchMark,testSizePre,testSizeMain);

			////////////////////// LockFree_impr /////////////////////
			cout << white << "LockFree_impr:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new LockFree_impr<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "LockFree_impr", benchMark,testSizePre,testSizeMain);

			////////////////////// LockFree_impr /////////////////////
			cout << white << "LockFree_impr_mem:" << endl;
			for (size_t i = 0; i < REPEAT_TESTS; i++) {
				list = new LockFree_impr_mem<int>();
				cout << "Pre: ";
				benchMark_arr[i] = BENCHMARK_E;
				runtest(testcases[0], list, &benchMark_arr[i].pre);
				cout << "Main: ";
				runtest(testcases[1], list, &benchMark_arr[i].main);
				cout << "Check: ";
				check(testcases[1], list, &benchMark_arr[i].check);
				delete list;
			}
			benchMark = BENCHMARK_E;
			averBenchm(benchMark_arr, &benchMark);
			BENCHM_TO_CSV(outFile, "LockFree_impr_mem", benchMark, testSizePre, testSizeMain);

			testCnt++;
		}
	} else {
		cout << "Unable to open file to write the results";
	}
	outFile.close();
	return 0;
}

/**
 * @brief Function add values from cases to the datastructure and removes them, if there a below 0
 * @details Function starts maximal available threads. It will add all positiv values from the vector "cases"
 * and remove all negative. Also the timemessurement takes place in this function.
 * @param[in]  	cases  			vector with all testcases
 * @param[in]  	list  			datastructure for which the benchmark is performed
 * @param[out]  sub_benchMark  	a struct, which stores information for benchmarking
 */
static void runtest(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark) {
	auto startTime = chrono::high_resolution_clock::now();
	size_t Tmax = omp_get_num_procs(); // available number of threads
	sub_benchMark_t sub_benchMark_arr[Tmax];

#pragma omp parallel shared(sub_benchMark_arr) num_threads(MIN(cases.size(), Tmax))
	// limit the maximal threads, if testfile has less lines
	{
		sub_benchMark_t sub_benchMark_loc = SUB_BENCHMARK_E;
		sub_benchMark_loc.cores = omp_get_num_threads(); // used number of threads
		size_t tid = omp_get_thread_num();				 // curred thread id
#pragma omp for
		for (auto it = cases.begin(); it < cases.end(); it++) {
			for (const auto &j : *it) {
				if (j < 0) {
					list->remove(-j, &sub_benchMark_loc);
					if (list->contains(j, &sub_benchMark_loc)) {
						cout << red << "Error: " << reset << j << " in list" << endl;
					}
				} else if (j > 0) {
					list->add(j, &sub_benchMark_loc);
					if (!list->contains(j, &sub_benchMark_loc)) {
						cout << red << "Error 1: " << reset << j << " not in list" << endl;
					}
				} else
					throw new invalid_argument("0 not allowed");
			}
		}
		sub_benchMark_arr[tid] = sub_benchMark_loc;
		list->emptyQueue(true);
	}
	uint16_t cores = sub_benchMark_arr[0].cores; // amount of cores from core 0
	auto finishTime = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finishTime - startTime;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

	sub_benchMark->time = ms;
	sub_benchMark->cores = cores;
	for (size_t i = 0; i < cores; i++) {
		sub_benchMark->goToStart += sub_benchMark_arr[i].goToStart;
		sub_benchMark->lostTime += sub_benchMark_arr[i].lostTime;
	}
	cout << green << "Test succeeded" << reset << " in " << ms << "ms with " << cores << " cores" << endl;
}

/**
 * @brief Function checks, iff all values in the dataset
 * @details It checks with the maximal available threads, if all values from the cases vector
 * with negative not in the list and positive values in the list.
 * @param[in]  	cases  			vector with all testcases
 * @param[in]  	list  			datastructure for which the benchmark is performed
 * @param[out]  sub_benchMark  	a struct, which stores information for benchmarking
 */
static void check(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark) {
	auto startTime = chrono::high_resolution_clock::now();
	// Compare to Valid
	bool correct = true;
	size_t Tmax = omp_get_num_procs();
	sub_benchMark_t sub_benchMark_arr[Tmax];

#pragma omp parallel num_threads(MIN(cases.size(), Tmax))
	{
		sub_benchMark_t sub_benchMark_loc = SUB_BENCHMARK_E;
		/* get the total number of threads available in this parallel region */
		sub_benchMark_loc.cores = omp_get_num_threads();
		size_t tid = omp_get_thread_num();
#pragma omp for
		for (auto it = cases.begin(); it < cases.end(); it++) {
			for (const auto &j : *it) {
				if (j < 0) {
					if (list->contains(-j, &sub_benchMark_loc)) {
						cout << red << "Error: " << reset << j << " in list" << endl;
					}
				} else if (j > 0) {
					if (!list->contains(j, &sub_benchMark_loc)) {
						cout << red << "Error 1: " << reset << j << " not in list" << endl;
					}
				} else
					throw new invalid_argument("0 not allowed");
			}
		}
		sub_benchMark_arr[tid] = sub_benchMark_loc;
	}
	auto finishTime = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finishTime - startTime;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	uint16_t cores = sub_benchMark_arr[0].cores; // amount of cores from core 0
	sub_benchMark->time = ms;
	sub_benchMark->cores = cores;

	// sum up the "goToStart" from all threads
	for (size_t i = 0; i < cores; i++) {
		sub_benchMark->goToStart += sub_benchMark_arr[i].goToStart;
		sub_benchMark->lostTime += sub_benchMark_arr[i].lostTime;
	}
	if (correct) {
		cout << green << "Test succeeded" << reset << " in " << ms << "ms with " << cores << " cores" << endl;
	}
	cout << endl;
}

/**
 * @brief Function read a csv file and put all values to the cases vector
 * @param[in]  		name  			name of the file, which should be read (csv file with "," as a seperator)
 * @param[out]  	cases  			vector with all testcases
 * @return amount of values, that was read, 0 if it was not possible to read
 */
static uint32_t read_file(string name, vector<vector<int>> *cases) {
	ifstream file(name);
	string line;
	uint32_t testSize = 0;

	// Load Test cases
	if (file.is_open()) {
		while (getline(file, line)) {
			vector<int> x;
			string segment;
			stringstream l(line);

			while (getline(l, segment, ',')) {
				auto t = stoi(segment);
				x.push_back(t);
				testSize++;
			}
			cases->push_back(x);
		}
		file.close();
		return testSize;
	} else {
		return 0;
	}
}
