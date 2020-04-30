# AMP
# @author Daniel Zainzinger
# @matrikelnummer 11777778
# @date 30.04.2020




CC=g++
CFLAGS= -Wall -g  -c
LDFLAGS=
SOURCES=test.cpp node.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test

all: $(EXECUTABLE)
.PHONY: clean

$(EXECUTABLE): $(OBJECTS) #linker
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -rf *.o $(EXECUTABLE) 

