DIRS = $(wildcard */)
.PHONY: all deploy debug clean

all: deploy

deploy debug clean:
	make -C $(foreach dir,$(DIRS),$(dir)) $@
