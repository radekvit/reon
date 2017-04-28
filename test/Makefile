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

all: test

test:
	chmod +x test.sh
	./test.sh
