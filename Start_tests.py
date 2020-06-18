#!/usr/bin/python3

import sys, getopt
import subprocess

import numpy as np

threads = 64
repets =10

for test in range(3, 10):
	for test in range(repets):
		subprocess.run(["srun", "--partition=q_student", 
								"--time=5:00", "-N 1", "-c 64", 
								"-n "+ str(threads),"-t"+ str(test)], shell=True, check=True)
