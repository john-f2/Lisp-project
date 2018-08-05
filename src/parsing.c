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

 S-Expressions: an internal list structure that is built up recursively 
 of numbers, symbols, and other lists. Used to store the program in Lisp


 build command
 cc -std=c99 -Wall parsing.c mpc.c -ledit -lm -o parsing
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



//Error handling, this struct will be used so that an expression will evaluate 
//to a number or an error
typedef struct{
	//type determines whether the lval object is a number or an error
	int type;
	long num;

	//err and sym types have some string data
	char* err;
	char* sym;

	//counter and lval list 
	//S-Expressions are variable length lists of other values.
	//this will be stored in cell 

	//lval with type SEXPR will use count and cell 
	int count;
	//we put 'struct' here because we dont want the pointer to refer to itself
	struct lval** cell;



}lval;

//enum is used to assign names to an integer constant 
//creates an enumeration of possible lval types 

//Chapter 9: added 2 more types, LVAL_SYM, LVAL_SEXPR, for S-Expressions
enum{LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};

//creates an enumeration of possible lval err values
//enum{LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM}; //orignal Error enum 


/* LVAL CONSTRUCTORS */
//lval constructors now return a pointer to lval object, this will
//make it easier to reference in a lval cell list

//constructs a pointer to lval type number
lval* lval_num(long x){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;

}

//constructs a pointer to lval type err
lval* lval_err(char* x){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;

	//allocate memory to err string
	//we add +1 because strlen() doesn't include the null terminator 
	//thus we add +1 so that there is enough allocated space 
	v->err = malloc(strlen(x) + 1);
	//copies parameter to err
	strcpy(v->err, x);
	return v;

}

//constructs a pointer to a lval type symbol
lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;

}

//constructs a pointer to a lval type sexpr with an empty  cell
lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;

}

/* DEALLOCATOR FOR LVAL */
void lval_del(lval* v){
	switch(v->type){

		//do nothing for number types
		case LVAL_NUM: break;

		//ERR and SYM, free allocated char memory
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;

		case LVAL_SEXPR:
			//deallocates each lval in the cell
			for(int i =0; i<v->count; i++){
				lval_del(v->cell[i]);
			}
			//deallocates the memory to contain the pointers to lval
			free(v->cell);	 
			break;
	}
	//finally deallocates the pointer to v 
	free(v);

}

/* READING EXPRESSION FUNCTIONS */

//parsing though the mpc input tree, number values are
//still strings, we need to convert them before we call our number constructor
lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	//converts t's content to long
	long x = strtol(mpc_ast_t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid_number");

}

//lval_read recurssion function, goes through mpc_ast_t tree
//creates lval objects for according tags
lval* lval_read(mpc_ast_t* t){

	//if the tag of t is number or symbol, we directly return a lval* 
	//base case 
	if(strcmp(mpc_ast_t->tag, "number")){ return lval_read_num(t)};
	if(strcmp(mpc_ast_t->tag, "symbol")){return lval_sym(t->contents)};

	//if root (>) or sexpr then create an new lval type sexpr
	lval* x = NULL;
	if(strcmp(t->tag, ">")){x = lval_sexpr();}
	if(strcmp(t->tag,"sexpr")){x = lval_sexpr();}

	//fills in lval type sexpr cell list 
	for(int i =0; i<t->children_cnum; i++){
		if(strcmp(t->children[i]->contents, "(" )){ continue; }
	}






}





//prints out lval
void lval_print(lval v){
	switch(v.type){
		//lval type number case
		case LVAL_NUM:
			//prints the long value
			printf("%li\n", v.num);
			break;

		case LVAL_ERR:
			if(v.err == LERR_DIV_ZERO){
				puts("Error: Cannot divide by zero");
			}
			else if(v.err == LERR_BAD_NUM){
				puts("Error: Invalid number");
			}
			else if(v.err == LERR_BAD_OP){
				puts("Error: Invalid operator");
			}
			break;



	}

}


//does the math evaluation between x y and the operator 
lval eval_op(lval x, char* op, lval y){
	//if either x or y type is error, return it
	if(x.type == LVAL_ERR){ return x; }
	if(y.type == LVAL_ERR){ return y; }

	//strcmp returns 0 if the two strings are equal 
	//returns a lval object with num equal to performed operation 
	if(strcmp(op, "+") == 0){ return lval_num(x.num + y.num); }
	else if(strcmp(op, "-") == 0){ return lval_num(x.num - y.num); }
	else if(strcmp(op, "*") == 0){ return lval_num(x.num * y.num); }
	else if(strcmp(op, "/") == 0){ 
		if(y.num == 0){
			return lval_err(LERR_DIV_ZERO);
		}

		return lval_num(x.num / y.num); 
	}

	//if none of the operator if statements are satisified, return a
	//lval with type error and err value of LERR_BAD_OP
	return lval_err(LERR_BAD_OP);
}

//recursion to evaluate the AST tree
lval eval(mpc_ast_t* t){
	//base case
	//strstr checks if the second string is a substring of the first
	//tags in the AST tree are nested, ex: expr|number|regex
	if(strstr(t->tag, "number") ){ 
		//checks if there is an error in number conversion 
		//if there is, return a lval object with type error and err value of 
		//LERR_BAD_NUM
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
		
	}

	//the operator is always the second child in the AST structure
	//the first child will either be 'regex' or '('
	char* op = t->children[1]->contents;

	//the third child is the expr, which we recursively call on
	//expr childs can have the tag "number" or >, if it has > it will
	//be a root node of its own tree
	lval x = eval(t->children[2]);

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

	//Chapter 9 update
	//edited towards S-Expressions
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	//This parser is essentially the "sentence" itself, it will be built
	//from the components above 
	mpc_parser_t* Jlispy = mpc_new("jlispy");

	//Defining parsers with the following language
	//uses Regular expressions to define rules 


	mpca_lang(MPCA_LANG_DEFAULT, 
		"                                                    \
		number   : /-?[0-9]+/ ;                              \
		symbol   : '+' | '-' | '*' | '/';                    \
		sexpr     : '('<expr>*')';                           \
		expr    : <number> | <symbol> | <sexpr>;             \
		jlispy   : /^/ <expr>* /$/ ;                         \
		",
		Number, Symbol, Sexpr, Expr, Jlispy);



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
			// lval results = eval(a);
			// lval_print(results);


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
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Jlispy);

	return 0;
}