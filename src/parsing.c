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
 of numbers, symbols, and other lists. Used to store the program in Lisp. 
 S-expression is represented in ( )

 While in Q-expressions ("quoted expressions"), the Lisp will not evaluate 
 the expression but instead store it


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
typedef struct lval{
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
enum{LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR};

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

//constructor for Qexpr
lval* lval_qexpr(void){
	lval* v=malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
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

		//if lval type is LVAL_SEXPR or LVAL_QEXPR
		case LVAL_SEXPR:
		case LVAL_QEXPR:
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
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid_number");

}

//foward declaration
lval* lval_add(lval* v, lval* y);

//lval_read recurssion function, goes through mpc_ast_t tree
//creates lval objects for according tags
lval* lval_read(mpc_ast_t* t){

	//if the tag of t is number or symbol, we directly return a lval* 
	//base case 
	if(strstr(t->tag, "number")){ return lval_read_num(t);}
	if(strstr(t->tag, "symbol")){return lval_sym(t->contents);}

	//if root (>) or sexpr then create an new lval type sexpr
	lval* x = NULL;
	if(strcmp(t->tag, ">") == 0){x = lval_sexpr();}
	if(strstr(t->tag,"sexpr")){x = lval_sexpr();}
	if(strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

	//fills in lval type sexpr cell list 
	for(int i =0; i<t->children_num; i++){
		//if contents are the following, ingnore them and continue the loop
		if(strcmp(t->children[i]->contents, "(" )==0){ continue; }
		if(strcmp(t->children[i]->contents, ")" )==0){ continue; }
		if(strcmp(t->children[i]->contents, "{")==0){ continue; }
		if(strcmp(t->children[i]->contents, "}")==0){ continue; }
		if(strcmp(t->children[i]->tag, "regex" )==0){ continue; }

		//lval_read recursively called on the children
		//added to x's cell
		x = lval_add(x, lval_read(t->children[i]));

	}
	return x;

}

//adds lval y to lval v's cell
lval* lval_add(lval* v, lval* y){
	v->count++;
	//reallocates the size of v
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	//sets the last cell in the list to y
	v->cell[v->count-1] = y;

	return v;
}

/* LVAL Printing Functions */

//forward declaration of lval_print
void lval_print(lval* v);

//this function will be called when the lval type is SEXPR
//open and close will be '(' and ')' from lval_print
void lval_expr_print(lval* v, char open, char close){

	//putchar writes a character to stdout
	putchar(open);

	//remember v will be the root of the tree
	for(int i =0; i< v->count; i++){

		//prints values within cell
		lval_print(v->cell[i]);

		//prints trailing space when element is not last 
		if(i != (v->count-1)){
			putchar(' ');
		}
	}
	putchar(close);


} 


//prints out lval
void lval_print(lval* v){
	switch(v->type){
		//lval type number case
		case LVAL_NUM:
			//prints the long value
			printf("%li", v->num);
			break;
		case LVAL_ERR:
			printf("Error: %s",v->err );
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_SEXPR:
			//if the lval is a sexpr, when we print, we encase it with ()
			lval_expr_print(v, '(',')');
			break;
		case LVAL_QEXPR:
			lval_expr_print(v, '{','}');
			break;

	}

}

void lval_println(lval* v){
	lval_print(v);
	putchar('\n');
}

lval* lval_eval(lval* v);
lval* lval_pop(lval* v, int index);
lval* lval_take(lval* v, int index);
lval* builtin_op(lval* v, char* op);
lval* builtin(lval* a, char* func);

/* Eval functions */

//evaluates the lval, starts by evaluating the children first
//if any child is an error, return that lval
//if the lval is an empty expression, hence (), return the lval directly 
//if the lval is a single expression, hence (5), return the single expression
lval* lval_eval_sexpr(lval* v){

	//Evaluates children 
	for(int i =0; i< v->count; i++){
		//evaulates each children
		//transforms each child and sets it in original cell 
		v->cell[i] = lval_eval(v->cell[i]);

	}

	//checks children for errors, returns child if error found
  	for (int i = 0; i < v->count; i++) {
    	if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  	}

	//checks empty expression
	if(v->count == 0){ 

		return v; 
	}

	//checks single expression
	if(v->count == 1){
		return lval_take(v,0);
	}

	//ensure first element is a symbol
	//if not, return error
	lval* f = lval_pop(v, 0);
	if(f->type != LVAL_SYM){
		lval_del(f);
		lval_del(v);
		return lval_err("S-Expression does not start with symbol");
	}

	//call builtin with operator
	//evaluates lval with symbol
	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;


}

lval* lval_eval(lval* v){
	//evaluates sexpr expressions
	if(v->type == LVAL_SEXPR){
		return lval_eval_sexpr(v);
	}
	//return all other types 
	return v;
}

//pops lval object at index from v's cell list
//we popout the symbol, so that we can just have a "list" of numbers to
//do the evalutation on
lval* lval_pop(lval* v, int index){
	//gets lval at index
	lval* x = v->cell[index];

	//shifts memory after lval at index is popped
	memmove(&v->cell[index], &v->cell[index+1],sizeof(lval*) * (v->count-index-1) );

	//descrease count after popping item
	v->count--;

	//reallocates lval cell memory
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);

	//return popped object
	return x;

}

//gets the lval at index and deletes original lval object afterwards
lval* lval_take(lval* v, int index){
	lval* x = lval_pop(v, index);
	lval_del(v);
	return x;

}

//takes in a lval object which represents all the 
lval* builtin_op(lval* a, char* op){

	//checks if all objects in v are numbers
	for(int i=0; i< a->count; i++){
		if(a->cell[i]->type != LVAL_NUM){
			lval_del(a);
			return lval_err("Error: Cannot operator on non-numerics");
		}
	}

	//pops the first element
	//all evaluations will be stored in x
	lval* x = lval_pop(a,0);

	//if no arguments and sub then perform unary negation
	//hence, if a is just a number with no other expressions and has a op of '-' 
	//then we just make it negative
	if((strcmp(op,"-") == 0 ) && a->count ==0){
		x->num = -x->num;
	}

	//while there are still remaining elements 
	while(a->count > 0){

		//pops the next element
		lval* y = lval_pop(a,0);

		//strcmp returns 0 if the two strings are equal 
		//returns a lval object with num equal to performed operation 
		if(strcmp(op, "+") == 0){ x->num += y->num; }
		else if(strcmp(op, "-") == 0){ x->num -= y->num; }
		else if(strcmp(op, "*") == 0){ x->num *= y->num; }
		else if(strcmp(op, "/") == 0){ 
			if(y->num == 0){
				return lval_err("Error: Division by Zero");
				break;
			}

			x->num /= y->num; 
		}
		//deallocate y after evaluation
		lval_del(y);
	}
	//deallocate a after evaluation
	lval_del(a);
	return x;


}

//MACRO: reprocessor statement for creating 
//function-like-things that are evaluated before the program is compiled.
//can be used to do better error checking
//its like python assert statments 

#define LASSERT(args, cond, err) \
	if(!(cond)){lval_del(args); return lval_err(err);}

//q expressions
lval* builtin_head(lval* a){

	//error checking

	//the lval we are passing in essentially holds another lval object
	//that will contain the data, hence, a lval object type qexpr will have 
	//one lval object in it's cell with all the other numbers/expressions
	LASSERT(a, a->count != 1,"Function 'head' passed too many arguments!");

	LASSERT(a, a->cell[0]->type != LVAL_QEXPR, "Function 'head' passed incorrect types!");

	LASSERT(a,a->cell[0]->count == 0, "Function 'head' passed {}!");

	lval* v = lval_take(a, 0);
	while(v->count > 1){
		lval_del(lval_pop(v,1));

	}
	return v;



}

lval* builtin_tail(lval* a){
	//error checking
	LASSERT(a, a->count != 1,"Function 'head' passed too many arguments!");

	LASSERT(a, a->cell[0]->type != LVAL_QEXPR, "Function 'head' passed incorrect types!");

	LASSERT(a,a->cell[0]->count == 0, "Function 'head' passed {}!");

	lval* v= lval_take(a,0);
	//delete and return the first item 
	lval_del(lval_pop(v,0));
	return v;


}

//converts a s-expression into a q-expression
lval* builtin_list(lval* a){
	a->type = LVAL_QEXPR;
	return a;

}

//converts q-expression to s-expression
lval* builtin_eval(lval* a){
	//error checking
	LASSERT(a, a->count != 1,"Function 'head' passed too many arguments!");

	LASSERT(a, a->cell[0]->type != LVAL_QEXPR, "Function 'head' passed incorrect types!");

	//gets the stored values
	lval* x = lval_take(a,0);

	//converts the stored lval into type LVAL_SEXPR
	x->type = LVAL_SEXPR;
	return lval_eval(x);



}

lval* lval_join(lval* x, lval* y);

//first check if all arguments are q expressions
//then we join them one by one
lval* builtin_join(lval* a){

	//checks if all arguments are q-expressions
	for(int i=0; i<a->count; i++){
		LASSERT(a, a->cell[i]->type != LVAL_QEXPR, "Function 'head' passed incorrect types!");

	}

	//the lval expressions will be joined into x
	lval* x = lval_pop(a, 0);
	while(a->count){
		x=lval_join(x, lval_pop(a,0));
	}

	lval_del(a);
	return x;

}

lval* lval_join(lval* x, lval* y){
	while(y->count){
		x = lval_add(x, lval_pop(y,0));

	}
	lval_del(y);
	return x;

}

lval* builtin(lval* a, char* func){
	if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
 	if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strstr("+-/*", func)) { return builtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}

// //does the math evaluation between x y and the operator 
// lval eval_op(lval x, char* op, lval y){
// 	//if either x or y type is error, return it
// 	if(x.type == LVAL_ERR){ return x; }
// 	if(y.type == LVAL_ERR){ return y; }

// 	//strcmp returns 0 if the two strings are equal 
// 	//returns a lval object with num equal to performed operation 
// 	if(strcmp(op, "+") == 0){ return lval_num(x.num + y.num); }
// 	else if(strcmp(op, "-") == 0){ return lval_num(x.num - y.num); }
// 	else if(strcmp(op, "*") == 0){ return lval_num(x.num * y.num); }
// 	else if(strcmp(op, "/") == 0){ 
// 		if(y.num == 0){
// 			return lval_err(LERR_DIV_ZERO);
// 		}

// 		return lval_num(x.num / y.num); 
// 	}

// 	//if none of the operator if statements are satisified, return a
// 	//lval with type error and err value of LERR_BAD_OP
// 	return lval_err(LERR_BAD_OP);
// }

// //recursion to evaluate the AST tree
// lval eval(mpc_ast_t* t){
// 	//base case
// 	//strstr checks if the second string is a substring of the first
// 	//tags in the AST tree are nested, ex: expr|number|regex
// 	if(strstr(t->tag, "number") ){ 
// 		//checks if there is an error in number conversion 
// 		//if there is, return a lval object with type error and err value of 
// 		//LERR_BAD_NUM
// 		errno = 0;
// 		long x = strtol(t->contents, NULL, 10);
// 		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
		
// 	}

// 	//the operator is always the second child in the AST structure
// 	//the first child will either be 'regex' or '('
// 	char* op = t->children[1]->contents;

// 	//the third child is the expr, which we recursively call on
// 	//expr childs can have the tag "number" or >, if it has > it will
// 	//be a root node of its own tree
// 	lval x = eval(t->children[2]);

// 	//iterate through the rest of the child nodes
// 	//we start at int i = 3 because we want to access the 4th child since
// 	//we evaluate the third child as x
// 	//the reason why we evaluate the third child into x is because we want a variable
// 	//we can multiply all the child nodes into
// 	int i = 3;
// 	while(strstr(t->children[i]->tag, "expr")){
// 		//this is where we will do the rest of the recursion on the other child nodes
// 		//eval_op function is where we will do the actual computation 
// 		x = eval_op(x, op, eval(t->children[i]));
// 		i++;
// 	}


// 	return x;
// }


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

	//chapter 10, added Qexpr
	mpc_parser_t* Qexpr = mpc_new("qexpr");

	//This parser is essentially the "sentence" itself, it will be built
	//from the components above 
	mpc_parser_t* Jlispy = mpc_new("jlispy");



	//Defining parsers with the following language
	//uses Regular expressions to define rules 


	//chapter 10, added more symbols for Q-expressions
	mpca_lang(MPCA_LANG_DEFAULT, 
		"                                                              \
		number   : /-?[0-9]+/ ;                                        \
        symbol : \"list\" | \"head\" | \"tail\"                        \
                 | \"join\" | \"eval\" | '+' | '-' | '*' | '/' ;       \
		sexpr    : '('<expr>*')';                                      \
		qexpr    : '{' <expr>* '}' ;                                   \
		expr     : <number> | <symbol> | <sexpr> | <qexpr>;            \
		jlispy   : /^/ <expr>* /$/ ;                                   \
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Jlispy);



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
			//used to printout the mpc_ast_t tree
			// '>' is the root 
			//mpc_ast_print(a);



			//recursively goes through the AST tree
			//outputs the mathmatical evaluation of the input
			// lval results = eval(a);
			// lval_print(results);

			lval* x = lval_eval(lval_read(a));
			lval_println(x);
			lval_del(x);


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
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Jlispy);

	return 0;
}