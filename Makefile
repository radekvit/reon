APPNAME=reon
INCLUDE=include
LIBDIR = lib/ctf
LIBINCLUDE = $(LIBDIR)/include
LIBSRC = $(LIBDIR)/src
SRC=src
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -I $(LIBINCLUDE)
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(INCLUDE)/*.h)
LIBHEADERS=$(wildcard $(LIBSRC)/*.hpp)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test pack doc run libbuild cleanall

all: deploy

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $@ $(LDLIBS)

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS) $(LIBHEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJFILES) $(APPNAME) doc/html

format:
	clang-format -style=file -i $(SRC)/*.cpp $(INCLUDE)/*.h

test: all
	make -C test test

pack: all
pack:
	zip ctf.zip include/*.h $(APPNAME)

doc:
	make -C doc

run: all
	./reon
