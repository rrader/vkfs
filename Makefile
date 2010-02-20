vkfs: obj/main.o obj/vklib.o
	g++ $(CFLAGS) -d -lcurlpp -lfuse -w obj/main.o obj/vklib.o -o bin/vkfs

obj/main.o: main.cpp
	g++ $(CFLAGS) -w -c main.cpp -o obj/main.o -D_FILE_OFFSET_BITS=64

obj/vklib.o: vklib/vklib.cpp
	g++ $(CFLAGS) -w -c vklib/vklib.cpp -o obj/vklib.o
