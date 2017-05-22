TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-Wall -Wpedantic



all: clean count_min_sketch
	gcc $(DISTDIR)/count_min_sketch.o $(TESTDIR)/count_min_sketch_test.c $(CCFLAGS) -o ./$(DISTDIR)/cms

clean:
	rm -rf ./$(DISTDIR)/*

count_min_sketch:
	gcc -c $(SRCDIR)/count_min_sketch.c -o $(DISTDIR)/count_min_sketch.o $(CCFLAGS)
