#include <stdio.h>
#include <sys/syscall.h>

int main(void){

	long int return_value = syscall(335);
	printf("System Call returned : %ld\n", return_value);

	return 0;
}
