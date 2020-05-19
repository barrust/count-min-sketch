CC=gcc
TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-lm -Wall -Wpedantic -Winline



all: count_min_sketch
	$(CC) $(DISTDIR)/count_min_sketch.o $(TESTDIR)/count_min_sketch_test.c $(CCFLAGS) -o ./$(DISTDIR)/cms

debug: CCFLAGS += -g
debug: all

release: CCFLAGS += -O3
release: all

clean:
	if [ -f "./$(DISTDIR)/count_min_sketch.o" ]; then rm -r ./$(DISTDIR)/count_min_sketch.o; fi
	if [ -f "./$(DISTDIR)/cms" ]; then rm -r ./$(DISTDIR)/cms; fi

count_min_sketch:
	$(CC) -c $(SRCDIR)/count_min_sketch.c -o $(DISTDIR)/count_min_sketch.o $(CCFLAGS)
