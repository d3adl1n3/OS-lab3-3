#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	while (1) {
		printf("hello: pid %d\n", getpid());
		sleep(1);
	}
}
