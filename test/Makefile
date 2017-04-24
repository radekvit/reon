APPNAME=reon_test
INCLUDE=../include
LIBDIR = ../lib/ctf
LIBINCLUDE = $(LIBDIR)/include
LIBSRC = $(LIBDIR)/src
SRC=../src
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -I $(LIBINCLUDE)
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(SRC)/*.h) $(wildcard *.h)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
LIBHEADERS=$(wildcard $(LIBSRC)/*.hpp)

.PHONY: all format clean debug build test

all: debug

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/main.o: main.cpp $(HEADERS) $(LIBHEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS) $(LIBHEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJ) $(APPNAME)

format:
	clang-format -style=file -i ./*.cpp ./*.h

test: debug
test:
	./$(APPNAME) ; make clean
