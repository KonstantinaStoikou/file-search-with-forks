all: searcher splitter_merger root

searcher: searcher.c
	gcc -g searcher.c -o searcher

splitter_merger: splitter_merger.c
	gcc -g splitter_merger.c -o splitter_merger

root: root.c
	gcc -g root.c root_functions.c -lm -o myfind

root_functions: root_functions.c
	gcc -g root_functions.c -o root_functions

clean:
	rm -f searcher splitter_merger root root_functions myfind
