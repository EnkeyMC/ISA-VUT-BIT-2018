run: compile
	echo NYI

compile:
	echo NYI


test: compile
	cd test && ./test.sh ../cmake-build-debug/isa

.PHONY: test
