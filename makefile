# AMP
# @author Daniel Zainzinger
# @matrikelnummer 11777778
# @date 30.04.2020




CXX=g++
CFLAGS= -Wall  -fopenmp -c -g -MMD
LDFLAGS=  -fopenmp
SOURCES=main.cpp node.cpp Fine_Grained.cpp Coarse_Grained.cpp Optimistic.cpp
BUILD_DIR = ./.build
OBJECTS=$(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEP = $(OBJECTS:%.o=%.d)
EXECUTABLE=main

all: $(EXECUTABLE)

.PHONY: clean

$(EXECUTABLE): $(OBJECTS) #linker
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

# Include all .d files
-include $(DEP)

# The -MMD flags additionaly creates a .d file with
# the same name as the .o file.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $< -o $@


clean:
	rm -rf $(OBJECTS) $(DEP) $(EXECUTABLE) $(BUILD_DIR)

