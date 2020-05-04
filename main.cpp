#include "Coarse_Grained.h"
#include "Fine_Grained.h"
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

	FineList<int> list;
	runtest("testcases/basic.txt", list);

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
				valid.insert(t);
			}

			cases.push_back(x);
		}

		file.close();
	}
	else {
		cout << "Unable to open file";
		return;
	}

	// Run test Cases
	cout<<"Running: "<<cases.size()<<" parallel cases"<<endl;
	// #pragma omp parallel
	// #pragma omp for
	for (const auto &i : cases) {
		for (const auto &j : i) {
			list.add(j);
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
		cout<<green<<"Test succeeded"<<reset<<endl;
	}
}