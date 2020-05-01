# AMP
# @author Daniel Zainzinger
# @matrikelnummer 11777778
# @date 30.04.2020




CC=g++
CFLAGS= -Wall -g -c -fopenmp
LDFLAGS= -g -fopenmp
SOURCES=node.cpp Coarse_Grained.cpp main.cpp
HEADER= Coarse-Grained.h
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(EXECUTABLE)
.PHONY: clean

$(EXECUTABLE): $(OBJECTS) #linker
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:%.cpp %.h
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -rf *.o $(EXECUTABLE) 

