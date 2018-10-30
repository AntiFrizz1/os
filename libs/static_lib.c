/*
 * Static library source
 */
#include "static_lib.h"
//x^2
int sqr(int a) {
	return a * a;
}

//x^y
int power(int a, int b) {
	int ans = 1;
	for (int i = 0; i < b; i++) {
		ans *= a;
	}
	return ans;
}