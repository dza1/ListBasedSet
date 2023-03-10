List Based Set
=======================
This project includes different data structures for multiprocessor programming. 
The main program will perform a benchmark for each data structure which is defined with the files in the “testcases” folder. 
In the first step it will fill the data structure. In the second step there will be a mixed access with “contain”, “add” and remove. 
In the third step there is a check, iff all supposed entries in the data structure. 
The results are stored in the "result" folder. 

How To Use 
-----------------------
compile the code:
	make all

create test case files (python3 required):
	make testFiles

execute benchmarktest for List Based Set:
	./main [-n thread_amount] [-t tested_list]
	
		-thread_amount defines the number of threads to use, without this option it use the amount of available cores
	 		-The number of threads is limited with the number of rows in the CSV files.

		-tested_list define which list should be used for the benchmark. without this option all lists are tested
			-0 all lists are tested
			-1 List-based set with coarse-grained locks
			-2 List-based set with fine-grained locks
			-3 List-based set with optimistic synchronization
			-4 List-based set with optimistic synchronization with memory management
			-4 List-based set with lazy synchronization
			-5 List-based set with lazy synchronization with memory management
			-6 Lock-free list-based set
			-8 Lock-free list-based set with memory management
			-9 Lock-free list-based set with improvement and memory management


Set the size of testcases
-----------------------
In ./testcases/permutation.py are two entries to set the size. “RAW_BASE” sets the amount of rows  and “COOLUMS_BASE”  the columns for the first test file. 
„FILE_AMOUNT“ define amount of test cases. In each iteration, RAW_BASE and COOLUMS_BASE is multiplied with 2 of the power of the current iterationn index.
For example:
RAW_BASE=10, COOLLUM_BASE=10 and FILE_AMOUNT =5.
It will create pre[0-4].csv and main[0-4].csv-
pre0.csv will have 10*10 entries and the pre4.csv will have 10*10*2^4 entries. 
The amount of entries in pre[0-9]+.csv define the size of the list. main[0-9]+.csv defines the amount of write operations on the list and it
has always the double entries than the pre-file. 

Repetition
-----------------------
In “benchmark.hpp” is the constant “REPEAT_TESTS” defined, which allows to change repetition for each benchmark. 
In the result will be an average of all repetition.