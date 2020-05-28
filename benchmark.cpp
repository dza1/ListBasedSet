
#include "benchmark.hpp"
#include <stdint.h>
#include "stdio.h"


void averBenchm(benchMark_t benchmark[],benchMark_t *averBench){

uint64_t preTime=0;
uint64_t pregoTo=0;
uint64_t mainTime=0;
uint64_t maingoTo=0;
uint64_t checkTime=0;
uint64_t checkgoTo=0;
	for (size_t i = 0; i < REPEAT_TESTS; i++)
	{
		preTime+=benchmark[i].pre.time;
		pregoTo+=benchmark[i].pre.goToStart;
		mainTime+=benchmark[i].main.time;
		maingoTo+=benchmark[i].main.goToStart;
		checkTime+=benchmark[i].check.time;
		checkgoTo+=benchmark[i].check.goToStart;
	}
	averBench->pre.time			=preTime/REPEAT_TESTS;
	averBench->pre.goToStart	=pregoTo/REPEAT_TESTS;
	averBench->main.time		=mainTime/REPEAT_TESTS;
	averBench->main.goToStart	=maingoTo/REPEAT_TESTS;
	averBench->check.time		=checkTime/REPEAT_TESTS;
	averBench->check.goToStart	=checkgoTo/REPEAT_TESTS;
	

}