EXE := build/ast

SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

OBJ_NOMAIN := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))

CPPFLAGS :=
CFLAGS   := -std=c++17 -Wall
LDFLAGS  :=
LDLIBS   := -lcosa2 -lsmt-switch-cvc4 -lsmt-switch -lgmp

.PHONY: all clean test

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

test: $(OBJ_DIR)/test.o
	$(CXX) $(LDFLAGS) $(LDLIBS) $(OBJ_NOMAIN) $(OBJ_DIR)/test.o -o build/runtest

$(OBJ_DIR)/test.o:
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c test/test.cpp -o obj/test.o

clean:
	$(RM) $(OBJ)
	rm -f build/ast
	rm -f build/runtest
