#include <xinu.h>
#include <stdio.h>
#include <string.h>

sid32 connecting_run_hello;
shellcmd xsh_hello(int nargs, char *args[]){
	
	if(nargs==2){
		printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
	}
	else{
		//Assignment 3
		printf("Syntax: run hello name\n");
	}
	//Assignment 2
	// else if(nargs>2){
	// 	printf("Too many arguments!");
	// }
	// else if(nargs<2){
	// 	printf("One argument required!!");
	// }
	signal(connecting_run_hello);
	return 0;
}
