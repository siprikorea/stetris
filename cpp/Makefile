# Every language folder exposes the same targets. C++ has one extra,
# because it is the only one with a front end.

all: test console

test:
	$(MAKE) -C test test

console:
	$(MAKE) -C app/console

clean:
	$(MAKE) -C test clean
	$(MAKE) -C app/console clean

.PHONY: all test console clean
