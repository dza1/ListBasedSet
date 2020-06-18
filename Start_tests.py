#!/usr/bin/python3

import sys, getopt
import subprocess



threads = 64
repets =4

for test in range(7, 10):
	for repets in range(repets):
		subprocess.run(["srun", "--partition=q_student", "--time=5", "-N 1", "-c 64", "./main", "-n "+ str(threads),"-t "+ str(test)],check=True)
