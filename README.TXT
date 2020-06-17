List Based Set
=======================
This project includes different data structures for multiprocessor programming. 
The main program will perform a benchmark for each data structure which is defined with the files in the “testcases” folder. 
In the first step it will fill the data structure. In the second step there will be a mixed access with “contain”, “add” and remove. 
In the third step there is a check, iff all supposed entries in the data structure. 
The reslults are stored in the "result" folder. 

How To Use 
-----------------------
compile the code:
	make all

create test case files (python3 required):
	make testFiles

execute benchmarktest for List Based Set:
	./main [-n thread_amount]
		-thread_amount defines the number of threads to use, without this option it use the amount of available cores
	 	-The number of threads is limited with the number of rows in the CSV files.
		


Set the size of testcases
-----------------------
In ./testcases/permutation.py are two entries to set the size. “RAW_BASE” sets the amount of rows  and “COOLUMS_BASE”  the columns for the first test file. 
„FILE_AMOUNT“ define amount of test cases. In each iteration, RAW_BASE and COOLUMS_BASE is multiplied with the current iteration index.
For example:
RAW_BASE=10, COOLLUM_BASE=10 and FILE_AMOUNT =5.
It will create pre[0-4].csv and main[0-4].csv-
main0.csv will have 10*10 entrys and the main4.csv will have 50*50 entries. 

Repetition
-----------------------
In “benchmark.hpp” is the constant “REPEAT_TESTS” defined, which allows to change repetition for each benchmark. 
In the result will be an average of all repetition.