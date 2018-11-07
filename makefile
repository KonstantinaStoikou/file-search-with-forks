all: 	searcher splitter_merger root

searcher: searcher.c
	gcc -g searcher.c -o searcher

splitter_merger: splitter_merger.c
	gcc -g splitter_merger.c -o splitter_merger

root: root.c
	gcc -g root.c -o root


clean:
	rm -f searcher splitter_merger root