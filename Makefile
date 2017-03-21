APPNAME=reon
INCLUDE=include
LIBDIR = lib/ctf
LIBINCLUDE = $(LIBDIR)/include
LIBFILES = $(LIBDIR)/libctf.so
SRC=src
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -I $(LIBINCLUDE)
LDLIBS = -Llib/ctf -lctf
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(INCLUDE)/*.h)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test pack doc run libbuild cleanall

all: deploy

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build
debug: LIBTARGET= debug

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build
deploy: LIBTARGET= deploy

$(APPNAME): libbuild
$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $@ $(LDLIBS)

libbuild:
	make -C lib/

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

cleanall: clean
	make -C lib/ clean

clean:
	-rm -rf $(OBJFILES) $(APPNAME) doc/html

format:
	clang-format -style=file -i $(SRC)/*.cpp $(INCLUDE)/*.h

test:
	make -C test test

pack: all
pack:
	zip ctf.zip include/*.h $(APPNAME)

doc:
	make -C doc

run: all
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib/ctf/ ;\
	./reon