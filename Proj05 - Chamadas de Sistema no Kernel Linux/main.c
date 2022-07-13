#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

char *getValue(int value) {
    char *result;
    if(value == -1) {
        result = "Peso Invalido!";
    }
    else{
        sprintf(result, "%d", value);
    }
    return result;
}

int main(int argc, char *argv[]) {
    int userID = atoi(argv[1]);
    int weight = atoi(argv[2]);
    int setUserWeight = 451;
    int getUserWeight = 452;

	printf("Peso do usuario %d antes da alteração: %s", userID, getValue(syscall(getUserWeight, userID)));

	if(syscall(setUserWeight, userID, weight) == -1) {
		printf("Houve um erro durante a definição do novo peso!");
		return -1;
	}

    printf("Peso do usuario %d depois da alteração: %s", userID, getValue(syscall(getUserWeight, userID)));

	return 0;
}
