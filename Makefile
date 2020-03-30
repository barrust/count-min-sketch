CC=gcc
TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-lm -Wall -Wpedantic -Winline -O3



all: count_min_sketch
	$(CC) $(DISTDIR)/count_min_sketch.o $(TESTDIR)/count_min_sketch_test.c $(CCFLAGS) -o ./$(DISTDIR)/cms

clean:
	if [ -d "./$(DISTDIR)/" ]; then rm -rf ./$(DISTDIR)/*; fi

count_min_sketch:
	$(CC) -c $(SRCDIR)/count_min_sketch.c -o $(DISTDIR)/count_min_sketch.o $(CCFLAGS)
