#include "Coarse_Grained.hpp"
#include "Fine_Grained.hpp"
#include "Lock_free.hpp"
#include "Lock_free_impr.hpp"
#include "Lock_free_impr_mem.hpp"
#include "Optimistic.hpp"
#include "benchmark.hpp"
#include <omp.h>
//#include <stdint.h>
#include "stdio.h"
#include "termcolor.hpp"
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
using namespace termcolor;
#include <set>



static uint32_t read_file(string name, vector<vector<int>> *cases);
static void runtest(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark);
static void check(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark);


int main(int argc, char *argv[]) {
	benchMark_t benchMark;
	benchMark_t benchMark_arr[REPEAT_TESTS];
	ofstream outFile("Test_result.csv", fstream::app);
	if (outFile.is_open()) {
		size_t maxTd = omp_get_num_procs();

		WRITE_HEADER(outFile);
		SetList<int> *list;
		for (size_t i = 0; i < AMOUNT_TESTCASES; i++) {
			vector<vector<int>> testcases[2];
			uint32_t testSizePre=0;
			uint32_t testSizeMain=0;
			char pre_file[20];
			char main_file[20];
			sprintf(pre_file, "testcases/pre%zu.txt", i);
			sprintf(main_file, "testcases/main%zu.txt", i);
			cout << blue << main_file << endl;
			testSizePre+=read_file(pre_file, &testcases[0]);
			testSizeMain=read_file(main_file, &testcases[1]);

			// ///////////////// CoarseList///////////////////////
			// cout << white << "CoarseList:" << endl;
			// for (size_t i = 0; i < REPEAT_TESTS; i++) {
			// 	list = new CoarseList<int>();
			// 	cout << "Pre: ";
			// 	benchMark_arr[i] = BENCHMARK_E;
			// 	runtest(testcases[0], list, &benchMark_arr[i].pre);
			// 	cout << "Main: ";
			// 	runtest(testcases[1], list, &benchMark_arr[i].main);
			// 	cout << "Check: ";
			// 	check(testcases[1], list, &benchMark_arr[i].check);
			// 	delete list;
			// }
			// benchMark=BENCHMARK_E;
			// averBenchm(benchMark_arr,&benchMark);
			// BENCHM_TO_CSV(outFile, "CoarseList", maxTd, benchMark,testSizePre,testSizeMain);

			// ////////////////////// FineList /////////////////////
			// cout << white << "FineList:" << endl;
			// for (size_t i = 0; i < REPEAT_TESTS; i++) {
			// 	list = new FineList<int>();
			// 	cout << "Pre: ";
			// 	benchMark_arr[i] = BENCHMARK_E;
			// 	runtest(testcases[0], list, &benchMark_arr[i].pre);
			// 	cout << "Main: ";
			// 	runtest(testcases[1], list, &benchMark_arr[i].main);
			// 	cout << "Check: ";
			// 	check(testcases[1], list, &benchMark_arr[i].check);
			// 	delete list;
			// }
			// benchMark=BENCHMARK_E;
			// averBenchm(benchMark_arr,&benchMark);
			// BENCHM_TO_CSV(outFile, "FineList", maxTd, benchMark,testSizePre,testSizeMain);

			// ////////////////////// Optimistic /////////////////////
			// cout << white << "Optimistic:" << endl;
			// for (size_t i = 0; i < REPEAT_TESTS; i++) {
			// 	list = new Optimistic<int>();
			// 	cout << "Pre: ";
			// 	benchMark_arr[i] = BENCHMARK_E;
			// 	runtest(testcases[0], list, &benchMark_arr[i].pre);
			// 	cout << "Main: ";
			// 	runtest(testcases[1], list, &benchMark_arr[i].main);
			// 	cout << "Check: ";
			// 	check(testcases[1], list, &benchMark_arr[i].check);
			// 	delete list;
			// }
			// benchMark=BENCHMARK_E;
			// averBenchm(benchMark_arr,&benchMark);
			// BENCHM_TO_CSV(outFile, "Optimistic", maxTd, benchMark,testSizePre,testSizeMain);

			// ////////////////////// LockFree /////////////////////
			// cout << white << "LockFree:" << endl;
			// for (size_t i = 0; i < REPEAT_TESTS; i++) {
			// 	list = new LockFree<int>();
			// 	cout << "Pre: ";
			// 	benchMark_arr[i] = BENCHMARK_E;
			// 	runtest(testcases[0], list, &benchMark_arr[i].pre);
			// 	cout << "Main: ";
			// 	runtest(testcases[1], list, &benchMark_arr[i].main);
			// 	cout << "Check: ";
			// 	check(testcases[1], list, &benchMark_arr[i].check);
			// 	delete list;
			// }
			// benchMark=BENCHMARK_E;
			// averBenchm(benchMark_arr,&benchMark);
			// BENCHM_TO_CSV(outFile, "LockFree", maxTd, benchMark,testSizePre,testSizeMain);


			// ////////////////////// LockFree_impr /////////////////////
			// cout << white << "LockFree_impr:" << endl;
			// for (size_t i = 0; i < REPEAT_TESTS; i++) {
			// 	list = new LockFree_impr<int>();
			// 	cout << "Pre: ";
			// 	benchMark_arr[i] = BENCHMARK_E;
			// 	runtest(testcases[0], list, &benchMark_arr[i].pre);
			// 	cout << "Main: ";
			// 	runtest(testcases[1], list, &benchMark_arr[i].main);
			// 	cout << "Check: ";
			// 	check(testcases[1], list, &benchMark_arr[i].check);
			// 	delete list;
			// }
			// benchMark=BENCHMARK_E;
			// averBenchm(benchMark_arr,&benchMark);
			// BENCHM_TO_CSV(outFile, "LockFree_impr", maxTd, benchMark,testSizePre,testSizeMain);

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
			benchMark=BENCHMARK_E;
			averBenchm(benchMark_arr,&benchMark);
			BENCHM_TO_CSV(outFile, "LockFree_impr_mem", maxTd, benchMark,testSizePre,testSizeMain);
		}
	} else {
		cout << "Unable to open file to write the results";
	}
	outFile.close();
	return 0;
}

static void runtest(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark) {

	auto start = chrono::high_resolution_clock::now();

	// Run test Cases
	size_t maxTd = omp_get_num_procs();

	sub_benchMark_t sub_benchMark_arr[maxTd];
#pragma omp parallel shared(sub_benchMark_arr)
	{
		sub_benchMark_t sub_benchMark_loc = SUB_BENCHMARK_E;
		size_t tid = omp_get_thread_num();
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
				}

				else
					throw new invalid_argument("0 not allowed");
			}
		}
		sub_benchMark_arr[tid] = sub_benchMark_loc;
		list->emteyQueue(true);
	}

	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

	sub_benchMark->time = ms;
	for (size_t i = 0; i < maxTd; i++) {
		sub_benchMark->goToStart += sub_benchMark_arr[i].goToStart;
	}
	cout << green << "Test succeeded" << reset << " in " << ms << "ms with " << maxTd << " cores" << endl;
}

static void check(vector<vector<int>> cases, SetList<int> *list, sub_benchMark_t *sub_benchMark) {
	auto start = chrono::high_resolution_clock::now();

	// Compare to Valid
	bool correct = true;
	size_t maxTd = omp_get_num_procs();
	sub_benchMark_t sub_benchMark_arr[maxTd];
#pragma omp parallel
	{
		sub_benchMark_t sub_benchMark_loc = SUB_BENCHMARK_E;
		size_t tid = omp_get_thread_num();
#pragma omp for
		for (auto it = cases.begin(); it < cases.end(); it++) {
			for (const auto &j : *it) {
				if (j < 0) {
					if (list->contains(j, &sub_benchMark_loc)) {
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
	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	sub_benchMark->time = ms;
	for (size_t i = 0; i < maxTd; i++) {
		sub_benchMark->goToStart += sub_benchMark_arr[i].goToStart;
	}
	if (correct) {
		cout << green << "Test succeeded" << reset << " in " << ms << "ms with " << maxTd << " cores" << endl;
	}
	cout << endl;
	mutex outl;
}

static uint32_t read_file(string name, vector<vector<int>> *cases) {
	ifstream file(name);
	string line;
	uint32_t testSize=0;

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
		cout << "Unable to open file";
		return 0;
	}
}

