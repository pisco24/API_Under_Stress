CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb
MONGO_FLAGS := $(shell pkg-config --cflags --libs libmongocxx)

BIN		:= bin
SRC		:= src/*.cpp $(shell find $(lib) -name *.cpp)
INCLUDE	:= -Iinclude -Ilib \
 -I/usr/local/include/mongocxx/v_noabi \
 -I/usr/local/include/bsoncxx/v_noabi
LIB		:= lib

LIBRARIES	:= -lboost_system -lpthread -lcrypto \
-lssl -lmongocxx -lbsoncxx
EXECUTABLE	:= main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)
	$(CXX) $(CXX_FLAGS) $(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*

# Makefile

# Compiler to use
CXX := c++

# Compiler flags, -std=c++11 for C++11 standard compliance
CXXFLAGS := -std=c++11

# Using pkg-config to get the flags for MongoDB C++ driver
MONGO_FLAGS := $(shell pkg-config --cflags --libs libmongocxx)

# Target executable name
TARGET := tutorial

# Source files
SOURCES := tutorial.cpp

# Rule to make the target
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(MONGO_FLAGS) -o $(TARGET)

# Rule for cleaning up
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: clean
