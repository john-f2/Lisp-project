#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"



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


 mpc library used for language construction 

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

//does the math evaluation between x y and the operator 
long eval_op(long x, char* op, long y){
	//strcmp returns 0 if the two strings are equal 
	if(strcmp(op, "+") == 0){ return x + y; }
	if(strcmp(op, "-") == 0){ return x - y; }
	if(strcmp(op, "*") == 0){ return x * y; }
	if(strcmp(op, "/") == 0){ return x / y; }

	return 0;
}

//recursion to evaluate the AST tree
long eval(mpc_ast_t* t){
	//base case
	//strstr checks if the second string is a substring of the first
	//tags in the AST tree are nested, ex: expr|number|regex
	if(strstr(t->tag, "number") ){ 
		//converts the string into an int
		return atoi(t->contents);
	}

	//the operator is always the second child in the AST structure
	//the first child will either be 'regex' or '('
	char* op = t->children[1]->contents;

	//the third child is the expr, which we recursively call on
	//expr childs can have the tag "number" or >, if it has > it will
	//be a root node of its own tree
	long x = eval(t->children[2]);

	//iterate through the rest of the child nodes
	//we start at int i = 3 because we want to access the 4th child since
	//we evaluate the third child as x
	//the reason why we evaluate the third child into x is because we want a variable
	//we can multiply all the child nodes into
	int i = 3;
	while(strstr(t->children[i]->tag, "expr")){
		//this is where we will do the rest of the recursion on the other child nodes
		//eval_op function is where we will do the actual computation 
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}


	return x;
}


int main(int argc, char** argv){


	//Parsers
	//we dynamically create new parser objects 
	//the first 3 parsers essentially build the structure of our "sentences"
	//hence: the grammar
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	//This parser is essentially the "sentence" itself, it will be built
	//from the components above 
	mpc_parser_t* Jlispy = mpc_new("jlispy");

	//Defining parsers with the following language
	//uses Regular expressions to define rules 



	mpca_lang(MPCA_LANG_DEFAULT, 
		"                                                    \
		number   : /-?[0-9]+/ ;                              \
		operator : '+' | '-' | '*' | '/';                    \
		expr     : <number> | '(' <operator> <expr>+ ')';    \
		jlispy   : /^/ <operator> <expr>+ /$/ ;              \
		",
		Number, Operator, Expr, Jlispy);



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

		//attempts to parse the user input into parseResult
		//this if statement will attempt to parse the user input and 
		//print out the structure of the input
		mpc_result_t r;
		if(mpc_parse("<stdin>", input, Jlispy, &r)){

			mpc_ast_t* a = r.output;

			//recursively goes through the AST tree
			//outputs the mathmatical evaluation of the input
			long results = eval(a);
			printf("Results: %li\n", results);

			//deallocates the mpc_ast_t object
			mpc_ast_delete(a);

	

		}
		else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}


		//deletes the input once we press enter
		free(input);


	}

	//deletes our parsers 
	mpc_cleanup(4, Number, Operator, Expr, Jlispy);

	return 0;
}