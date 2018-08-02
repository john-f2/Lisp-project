#include <stdio.h>
#include <stdlib.h>



/*

Interactive prompt
- Based on REPL (Read-Evaluate-Print-Loop)
- Prompts user inputs, then our program will evaulate the input 
  and print a responce


 editline library
 the editline library allows users to use their arrow keys to edit this response
 On mac, the arrow keys on terminal default to special arrow characters

 compile using cc -std=c99 -Wall prompt.c -ledit -o prompt
 "-ledit" flag lets us link our program to the editline library


 remember char* is a string (stirngs in C are char arrays)

*/


/* preprocessor to detect operating system, used to prevent compiliation issues. 
If compiled on a windows machine, we have defined our own functions for readline 
and add_history*/
#ifdef _WIN32
#include <string.h>

//readline function for windows 
char* readline(char* prompt){
	static char buffer[2048];

	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);

	//malloc allocates the requested memory and returns a pointer to it.
	char* cpy = malloc(strlen(buffer)+1);
	//copies buffer into cpy
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';

	return cpy;


}

//add_history function for windows
//it does nothing, its not needed 
void add_history(char* unused){}

/* Preprocessor for Mac/Linux, includes the editline library */
// __APPLE__ is the preprocessor term for MacOS
#else
#include <editline/readline.h>
#endif


int main(int argc, char** argv){



	//Prints Lisp version and information on exiting
	puts("JLispy Version 0.0.1");
	puts("CTRL+C to Exit\n");

	//while(1) is a while true loop
	while(1){

		//readline lets us prompt and get input in the same function
		//rather than doing it in to like above 
		//dynamic allocation of memory
		char* input = readline("JLispy>>> ");

		//we pass the input to the add_history function which will record the input
		add_history(input);

		printf("JLispy>>> JLispy Responce Back: %s\n", input);

		//deletes the input once we press enter
		free(input);


	}

	return 0;
}