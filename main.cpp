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

void runtest( set<int> valid, vector<vector<int>> cases, SetList<int> *list);
void read_file(string name, set<int> *valid,set<int> *invalid, vector<vector<int>> *cases);
void check(set<int> valid,set<int> invalid, SetList<int> *list);

int main(int argc, char *argv[]) {
	set<int> valid_basic, valid_rem,valid_pre,valid_main;
	set<int> invalid_basic, invalid_rem,invalid_pre,invalid_main;
	vector<vector<int>> cases_basic, cases_rem,cases_pre, cases_main;
	read_file("testcases/basic.txt", 	&valid_basic,	&invalid_basic,	 	&cases_basic);
	read_file("testcases/remove.txt", 	&valid_rem,		&invalid_rem,	 	&cases_rem);
	read_file("testcases/pre.txt", 		&valid_pre,		&invalid_pre,	 	&cases_pre);
	read_file("testcases/main.txt", 	&valid_main,	&invalid_main,	 	&cases_main);

	SetList<int> *list;

	/////////////////// CoarseList///////////////////////
	// cout << endl << endl << "CoarseList:" << endl;

	// list = new CoarseList<int>();
	// runtest(valid_basic,cases_basic, list);
	// runtest(valid_rem,cases_rem, list);

	// delete list;
	// list = new CoarseList<int>();
	// runtest(valid_pre,cases_pre, list);
	// runtest(valid_main,cases_main, list);
	// check(valid_main,invalid_main, list);
	// delete list;

	////////////////////// FineList /////////////////////
	// cout << endl << endl << "FineList:" << endl;

	// list = new FineList<int>();

	// runtest(valid_basic,cases_basic, list);
	// runtest(valid_rem,cases_rem, list);
	// delete list;

	// list = new FineList<int>();
	// runtest(valid_pre,cases_pre, list);
	// runtest(valid_main,cases_main, list);
	// check(valid_main,invalid_main, list);
	// delete list;

	// ///////////////////// Optimistic_mem /////////////////////
	//cout << endl << endl << "Optimistic_mem:" << endl;

	// list = new Optimistic_mem<int>();
		
	// runtest(valid_basic,cases_basic, list);
	// runtest(valid_rem,cases_rem, list);
	// delete list;

	// list = new Optimistic_mem<int>();
	// runtest(valid_pre,cases_pre, list);
	// runtest(valid_main,cases_main, list);
	// check(valid_main,invalid_main, list);
	// delete list;

	// ///////////////////// Optimistic /////////////////////
	// cout << endl << endl << "Optimistic:" << endl;

	// list = new Optimistic<int>();
		
	// runtest(valid_basic,cases_basic, list);
	// runtest(valid_rem,cases_rem, list);
	// delete list;

	// list = new Optimistic<int>();
	// runtest(valid_pre,cases_pre, list);
	// runtest(valid_main,cases_main, list);
	// check(valid_main,invalid_main, list);
	// delete list;
	///////////////////// Lock Free /////////////////////
	cout << endl << endl << "Lock Free:" << endl;
	//for (size_t i = 0; i < 1; i++) {

	list = new LockFree_mem<int>();
	
	runtest(valid_basic,cases_basic, list);
	runtest(valid_rem,cases_rem, list);
	delete list;

	for (size_t i=0; i < 500; i++){
		list = new LockFree_mem<int>();
		runtest(valid_pre,cases_pre, list);
		runtest(valid_main,cases_main, list);
		check(valid_main,invalid_main, list);
		delete list;
	}
	return 0;
}


void runtest(set<int> valid, vector<vector<int>> cases, SetList<int> *list) {


	auto start = chrono::high_resolution_clock::now();

// Run test Cases
#pragma omp parallel
#pragma omp for
	for (auto it = cases.begin(); it < cases.end(); it++) {
		for (const auto &j : *it) {
			if (j < 0) {
				list->remove(-j);
				if (list->contains(j)) {
					cout << red << "Error: " << reset << j << " in list" << endl;
				}
			} else if (j > 0)
				list->add(j);

			else
				throw new invalid_argument("0 not allowed");
		}
		{
			// const lock_guard<mutex> lock(outl);
			// cout<<"Added "<<it->size()<<" by "<<cyan<<"Thread "<<omp_get_thread_num()<<reset<<endl;
		}
	}

	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

	// Compare to Valid
	bool correct = true;
	for (const auto &i : valid) {
		if (!list->contains(i)) {
			cout << red << "Error: " << reset << i << " not in list" << endl;
			correct = false;
		}
	}

	if (correct) {
		cout << green << "Test succeeded" << reset 
			 << " in " << ms << "ms" << endl;
	}
}

void check(set<int> valid,set<int> invalid, SetList<int> *list) {



	auto start = chrono::high_resolution_clock::now();

	// Compare to Valid
	bool correct = true;
	for (const auto &i : valid) {
		if (!list->contains(i)) {
			cout << red << "Error: " << reset << i << " not in list" << endl;
			correct = false;
		}
	}

	for (const auto &i : invalid) {
		if (list->contains(i)) {
			cout << red << "Error: " << reset << i << " in list" << endl;
			correct = false;
		}
	}

	auto finish = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = finish - start;
	auto ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	if (correct) {
		cout << green << "Test succeeded" << reset  << " in " << ms << "ms" << endl;
	}
	cout << endl;
	mutex outl;
}


void read_file(string name, set<int> *valid,set<int> *invalid, vector<vector<int>> *cases) {
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

				if (t < 0)
					invalid->erase(-t);
				else if (t > 0)
					valid->insert(t);
				else
					throw new invalid_argument("0 not allowed");
			}

			cases->push_back(x);
		}

		file.close();
	} else {
		cout << "Unable to open file";
		return;
	}
}