#include <stdio.h>
#include <dlfcn.h>

extern int sqr(int a);
extern int power(int a, int b);
extern int sum(int a, int b);
extern int multi(int a, int b);

int main() {
	//from static library
	printf("5^2=%d\n", sqr(5));
	printf("10^3=%d\n", power(10, 3));
	//from first dynamic library
	printf("12+16=%d\n", sum(12, 16));
	printf("12*16=%d\n", multi(12, 16));
	void *second_dynamic_lib = dlopen("./libsdynamic.so", RTLD_LAZY);
	if (second_dynamic_lib == NULL) {
		printf("%s\n", dlerror());
		return 1;
	}
	//from second dynamic library
	int (*sub)(int, int) = (int (*)(int, int))dlsym(second_dynamic_lib, "sub");
	int (*div)(int, int) = (int (*)(int, int))dlsym(second_dynamic_lib, "div");
	printf("12-16=%d\n", sub(12, 16));
	printf("16/4=%d\n", div(16, 4));
	if (dlclose(second_dynamic_lib) < 0) {
		dlerror();
	}
	return 0;
}
