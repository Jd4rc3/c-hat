CC = clang

all: server

run: server
	./server

server: linked_list.o main.o
	$(CC) -o $@ $^
main.o: main.c
	$(CC) -g -c $^ -o $@
linked_list.o: linked_list.c
	$(CC) -g -c $^ -o $@
clean:
	rm *.o server
