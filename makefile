all: 	searcher splitter_merger root

searcher: searcher.c
	gcc -g searcher.c -o searcher

splitter_merger: splitter_merger.c
	gcc -g splitter_merger.c -lm -o splitter_merger

root: root.c
	gcc -g root.c -lm -o root


clean:
	rm -f searcher splitter_merger root
