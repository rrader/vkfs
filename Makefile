vkfs: obj/main.o obj/vklib.o
	g++ -d -lcurlpp -lfuse -lpcre -w obj/main.o obj/vklib.o -o bin/vkfs

obj/main.o: main.cpp
	g++ -w -c main.cpp -o obj/main.o -D_FILE_OFFSET_BITS=64

obj/vklib.o: vklib/vklib.cpp
	g++ -w -c vklib/vklib.cpp -o obj/vklib.o

clean:
	rm obj/*o bin/vkfs
