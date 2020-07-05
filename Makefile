CC=gcc
TESTDIR=tests
DISTDIR=dist
SRCDIR=src
COMPFLAGS=-lm -Wall -Wpedantic -Winline


all: count_min_sketch
	$(CC) $(DISTDIR)/count_min_sketch.o $(TESTDIR)/count_min_sketch_test.c $(CCFLAGS) $(COMPFLAGS) -o ./$(DISTDIR)/cms

debug: COMPFLAGS += -g
debug: all

release: COMPFLAGS += -O3
release: all

sanitize: COMPFLAGS += -fsanitize=undefined
sanitize: test

test: COMPFLAGS += -coverage
test: count_min_sketch
	$(CC) $(DISTDIR)/count_min_sketch.o $(TESTDIR)/test_cms.c $(CCFLAGS) $(COMPFLAGS) -lcrypto -o ./$(DISTDIR)/test

clean:
	if [ -f "./$(DISTDIR)/count_min_sketch.o" ]; then rm -r ./$(DISTDIR)/count_min_sketch.o; fi
	if [ -f "./$(DISTDIR)/cms" ]; then rm -r ./$(DISTDIR)/cms; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcno; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcda; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -r ./$(DISTDIR)/test; fi
	rm -f ./*.gcno
	rm -f ./*.gcda
	rm -f ./*.gcov

count_min_sketch:
	$(CC) -c $(SRCDIR)/count_min_sketch.c -o $(DISTDIR)/count_min_sketch.o $(CCFLAGS) $(COMPFLAGS)
