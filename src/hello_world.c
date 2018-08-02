#include <stdio.h>

/* 

Lisp project



*/

int main(int argc, char** argv){

	//the puts function writes a string to 
	//stdout up to but not including the null character.
	puts("hello world!");

	//puts and printf do the same thing, as they both write out to stdout
	//but printf allows for string formating 


	//COMPILATION
	//cc -std=c99 -Wall hello_world.c -o hello_world
	//The '-std=c99' flag tells our compiler what version of C we want to run
	//after compilation we can run the program using ./hello_world

	//lldb and valgrind
	//use lldb for debug and valgrind for memory leaks 

	return 0;
}