DIRS = $(wildcard */)
.PHONY: all libs

all: libs

libs:
	make -C $(foreach dir,$(DIRS),$(dir))