#include "Coarse_Grained.h"
#include "Fine_Grained.h"
#include "Lock_free.h"
#include "Lock_free_mem.h"
#include "Optimistic.h"
#include "Optimistic_mem.h"
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
#define AMOUNT_TESTCASES 5

void runtest(vector<vector<int>> cases, SetList<int> *list);
void read_file(string name, vector<vector<int>> *cases);
void check(vector<vector<int>> cases, SetList<int> *list);

int main(int argc, char *argv[]) {

	SetList<int> *list;
	for (size_t i = 0; i < AMOUNT_TESTCASES; i++) {
		vector<vector<int>> testcases[2];
		char pre_file[20];
		char main_file[20];
		sprintf(pre_file, "testcases/pre%zu.txt", i);
		sprintf(main_file, "testcases/main%zu.txt", i);
		cout << blue << main_file << endl;
		read_file(pre_file, &testcases[0]);
		read_file(main_file, &testcases[1]);
		///////////////// CoarseList///////////////////////
		cout << white<< "CoarseList:" << endl;
		list = new CoarseList<int>();
		cout  << "Pre: ";
		runtest(testcases[0], list);
		cout << "Main: ";
		runtest(testcases[1], list);
		cout << "Check: ";
		check(testcases[1], list);
		delete list;

		////////////////////// FineList /////////////////////
		cout << "FineList:" << endl;
		list = new FineList<int>();
		cout << "Pre: ";
		runtest(testcases[0], list);
		cout << "Main: ";
		runtest(testcases[1], list);
		cout << "Check: ";
		check(testcases[1], list);
		delete list;

		////////////////////// Optimistic /////////////////////
		cout << "Optimistic:" << endl;
		list = new Optimistic<int>();
		cout << "Pre: ";
		runtest(testcases[0], list);
		cout << "Main: ";
		runtest(testcases[1], list);
		cout << "Check: ";
		check(testcases[1], list);
		delete list;

		////////////////////// LockFree /////////////////////
		cout << endl << "LockFree:" << endl;
		list = new LockFree<int>();
		cout << "Pre: ";
		runtest(testcases[0], list);
		cout << "Main: ";
		runtest(testcases[1], list);
		cout << "Check: ";
		check(testcases[1], list);
		delete list;
	}
	return 0;
}

void runtest(vector<vector<int>> cases, SetList<int> *list) {

	auto start = chrono::high_resolution_clock::now();

	// Run test Cases
	size_t maxTd = omp_get_num_procs();

	int benchMark_arr[maxTd];
#pragma omp parallel shared(benchMark_arr)
	{
		int benchMark = 0;
		size_t tid = omp_get_thread_num();
#pragma omp for
		for (auto it = cases.begin(); it < cases.end(); it++) {
			for (const auto &j : *it) {
				if (j < 0) {
					list->remove(-j, &benchMark);
					if (list->contains(j, &benchMark)) {
						cout << red << "Error: " << reset << j << " in list" << endl;
					}
				} else if (j > 0) {
					list->add(j, &benchMark);
					if (!list->contains(j, &benchMark)) {
						cout << red << "Error 1: " << reset << j << " not in list" << endl;
					}
				}

				else
					throw new invalid_argument("0 not allowed");
			}
		}
#pragma omp barrier
		benchMark_arr[tid] = benchMark;
	}
	int result = 0;
	for (size_t i = 0; i < maxTd; i++) {
		result = result + benchMark_arr[i];
	}
	// cout << "Result of benchmark: " << result << endl;

	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

	cout << green << "Test succeeded" << reset << " in " << ms << "ms" << endl;
}

void check(vector<vector<int>> cases, SetList<int> *list) {

	auto start = chrono::high_resolution_clock::now();

	// Compare to Valid
	bool correct = true;
	int benchMark_arr[omp_get_num_threads()];
#pragma omp parallel
	{
		int benchMark = 0;
#pragma omp for
		for (auto it = cases.begin(); it < cases.end(); it++) {
			for (const auto &j : *it) {
				if (j < 0) {
					if (list->contains(j, &benchMark)) {
						cout << red << "Error: " << reset << j << " in list" << endl;
					}
				} else if (j > 0) {
					if (!list->contains(j, &benchMark)) {
						cout << red << "Error 1: " << reset << j << " not in list" << endl;
					}
				}

				else
					throw new invalid_argument("0 not allowed");
			}
			{
				// const lock_guard<mutex> lock(outl);
				// cout<<"Added "<<it->size()<<" by "<<cyan<<"Thread "<<omp_get_thread_num()<<reset<<endl;
			}
		}
	}
	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	if (correct) {
		cout << green << "Test succeeded" << reset << " in " << ms << "ms" << endl;
	}
	cout << endl;
	mutex outl;
}

void read_file(string name, vector<vector<int>> *cases) {
	ifstream file(name);
	string line;

	// Load Test cases
	if (file.is_open()) {
		while (getline(file, line)) {
			vector<int> x;
			string segment;
			stringstream l(line);

			while (getline(l, segment, ' ')) {
				auto t = stoi(segment);
				x.push_back(t);
			}

			cases->push_back(x);
		}

		file.close();
	} else {
		cout << "Unable to open file";
		return;
	}
}