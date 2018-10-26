EXEC=feedreader
ZIP=xomach00.zip
SOURCES=$(wildcard src/*)
MISC_FILES=Makefile CMakeLists.txt test/test.sh
LIB_FILES=$(wildcard lib/*)
TESTS=$(shell find test -type f -regex ".*\(test\|out\)")

run: compile
	./$(EXEC) https://xkcd.com/atom.xml

compile: $(SOURCES) CMakeLists.txt
	mkdir -p cmake-build-debug
	cd cmake-build-debug && cmake .. && make && mv $(EXEC) ../

compile-force: $(SOURCES) CMakeLists.txt
	mkdir -p cmake-build-debug
	cd cmake-build-debug && rm CMakeCache.txt && cmake .. && make && mv $(EXEC) ../

clean:
	rm -f $(EXEC)
	rm -f $(ZIP)

pack:
	zip $(ZIP) $(SOURCES) $(MISC_FILES) $(TESTS) $(LIB_FILES)

test: compile
	cd test && ./test.sh ../$(EXEC)

test-r: compile-force
	cd test && ./test.sh ../$(EXEC)

test-remote: pack
	scp $(ZIP) xomach00@merlin.fit.vutbr.cz:~/isa/
	ssh xomach00@merlin.fit.vutbr.cz 'cd isa/tmp && unzip -o ../$(ZIP) && make test-r'
	ssh xomach00@eva.fit.vutbr.cz 'cd isa/tmp &&  make test-r'

.PHONY: test pack clean

valgrind: compile
	valgrind ./$(EXEC) https://xkcd.com/atom.xml
