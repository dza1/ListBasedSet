#!/usr/bin/python3

import sys, getopt
import subprocess




repets =10

for cores in range(4, 7):
	threads = 2**cores
	
	for test in range(0, 1):
		for i in range(repets):
			subprocess.run(["srun", "--partition=q_student", "--time=5", "-N 1", "-c 64", "./main","-n "+str(threads) ,"-t "+ str(test)],check=True)
