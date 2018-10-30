run: main.o libstatic.a libfdynamic.so libsdynamic.so
	gcc  -o bin main.o -L. -lstatic -L. -Wl,-rpath="." -lfdynamic -ldl
	./bin
main.o: main.c
	gcc -c main.c

static_lib.o: static_lib.c static_lib.h
	gcc -static -c static_lib.c

libstatic.a: static_lib.o
	ar cr libstatic.a static_lib.o


first_dynamic_lib.o: first_dynamic_lib.c first_dynamic_lib.h
	gcc -fPIC -c first_dynamic_lib.c


libfdynamic.so: first_dynamic_lib.o
	gcc -shared -o libfdynamic.so first_dynamic_lib.o


second_dynamic_lib.o: second_dynamic_lib.c second_dynamic_lib.h
	gcc -fPIC -c second_dynamic_lib.c

libsdynamic.so: second_dynamic_lib.o
	gcc -shared -o libsdynamic.so second_dynamic_lib.o 


clean:
	rm -f *.o *.a *.so bin