#include "Coarse_Grained.h"
#include <omp.h>
//#include <stdint.h>
#include "stdio.h"
#include <fstream>
#include <sstream>
#include <vector>
#include "termcolor.hpp"
using namespace termcolor;
#include <set>

void runtest(string name, SetList<int> &list);

int main(int argc, char *argv[]){

	CoarseList<int> list;
	runtest("testcases/basic.txt", list);
	//runtest("testcases/remove.txt", list);
	runtest("testcases/large.txt", list);

	return 0;
}

void runtest(string name, SetList<int> &list) {
	ifstream file(name);
	string line;
	vector<vector<int>> cases;
	set<int> valid;

	// Load Test cases
	if (file.is_open())
	{
		while ( getline (file, line) )
		{
			vector<int> x;
			string segment;
			stringstream l(line);

			while(getline(l, segment, ' '))
			{
				auto t = stoi(segment);
				x.push_back(t);
				
				if (t < 0)
					valid.erase(t);
				else if (t > 0)
					valid.insert(t);
				else
					throw new invalid_argument("0 not allowed");
			}

			cases.push_back(x);
		}

		file.close();
	}
	else {
		cout << "Unable to open file";
		return;
	}

	cout<<endl;
	mutex outl;

	// Run test Cases
	#pragma omp parallel
	#pragma omp for
	for (auto it = cases.begin(); it != cases.end(); it++) {
		for (const auto &j : *it) {
			if (j < 0)
				list.remove(j);
			else if (j > 0)
				list.add(j);
			else
				throw new invalid_argument("0 not allowed");
		}
		{
			const lock_guard<mutex> lock(outl);
			cout<<"Added "<<it->size()<<" by "<<cyan<<"Thread "<<omp_get_thread_num()<<reset<<endl;
		}
	}

	// Compare to Valid
	bool correct = true;
	for (const auto &i : valid) {
		if (!list.contains(i)) {
			cout<<red<<"Error: "<<reset<<i<<" not in list"<<endl;
			correct = false;
		}
	}

	if (correct) {
		cout<<green<<"Test succeeded"<<reset<<" ("<<name<<")"<<endl;
	}
}