all: trace

trace: LC4.o loader.o trace.c
	#
	#NOTE: CIS 240 students - this Makefile is broken, you must fix it before it will work!!
	#
	clang -g LC4.o loader.o trace.c -o trace

LC4.o: LC4.c
	#
	#CIS 240 TODO: update this target to produce LC4.o
	#
	clang -c -g LC4.c -o LC4.o

loader.o: loader.c
	#
	#CIS 240 TODO: update this target to produce loader.o
	#
	clang -c -g loader.c -o loader.o

clean:
	rm -rf *.o

clobber: clean
	rm -rf trace
