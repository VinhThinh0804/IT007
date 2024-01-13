#include<stdio.h>
#include<signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

void on_sigint(){
	printf("count.sh has stopped! Goodbye!\n");
	exit(0);
}

int main(){
	printf("Welcome to IT007. I am 22521417\n");
	__pid_t pid;
	pid = fork();
	if(pid > 0){
		signal(SIGINT, on_sigint);
		wait(NULL);
	}
	if(pid == 0){
		execl("./count.sh", "./count.sh", "120", NULL);
	}
	return 0;
}
