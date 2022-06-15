.PHONY: all run clean

all: libmylibc.so libmyfs.so test

test: test.o libmylibc.so libmyfs.so
	g++ test.o ./libmylibc.so ./libmyfs.so -o test -fPIC

libmyfs.so: my_fs.o
	g++ my_fs.o -shared -o libmyfs.so -fPIC

libmylibc.so: mylibc.o
	g++ mylibc.o -shared -o libmylibc.so -fPIC

mylibc.o:
	g++ -c mylibc.cpp -fPIC

%.o: %.cpp
	g++ -c $< -o $@ -fPIC

clean:
	rm -f *.o test *.so