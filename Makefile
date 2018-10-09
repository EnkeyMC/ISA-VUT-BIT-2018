EXEC=isa
ZIP=xomach00.zip
SOURCES=$(wildcard src/*)
MISC_FILES=Makefile CMakeLists.txt
TESTS=$(shell find test -type f -regex ".*\(test\|out\)")

run: compile
	echo NYI

compile: cmake
	echo NYI

cmake:
	rm -rf cmake-build-debug
	mkdir cmake-build-debug
	cd cmake-build-debug && cmake .. && make && mv $(EXEC) ../
.PHONY: cmake

clean:
	rm -f $(EXEC)
	rm -f $(ZIP)

pack:
	zip $(ZIP) $(SOURCES) $(MISC_FILES) $(TESTS)

test: compile
	cd test && ./test.sh ../$(EXEC)

test-remote: pack
	scp $(ZIP) xomach00@merlin.fit.vutbr.cz:~/isa/
	ssh xomach00@merlin.fit.vutbr.cz 'cd isa/tmp && unzip ../$(ZIP) && make test'
	ssh xomach00@eva.fit.vutbr.cz 'cd isa/tmp &&  make test'

.PHONY: test pack
