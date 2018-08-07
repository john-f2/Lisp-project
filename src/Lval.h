#ifndef Lval
#define Lval

#include "mpc.h"

/*Error handling, this struct will be used so that an expression will evaluate 
 to a number or an error */
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

/* enum for Lval types */
//Chapter 9: added 2 more types, LVAL_SYM, LVAL_SEXPR, for S-Expressions
enum{LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};


/* Lval Constructors */
lval* lval_num(long x);
lval* lval_err(char err);
lval* lval_sym(char* s);
lval* lval_sexpr(void);

/* Lval Deconstructor */
void lval_del(lval* v);


/* Lval Read Functions */
lval* lval_read_num(lval* v);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* v, lval* y);


/* Lval Print Functions */
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);


/* Lval Evaluate Functions */
//Eval functions can be thought as a transformer, where we take a Lval* and 
//transform it into a new/different Lval*
lval* lval_eval_sexpr(lval* v); 
lval* lval_eval(lval* v);
lval* lval_pop(lval* v, int index);
lval* lval_take(lval*v, int index);
lval* builtin_op(lval*a, char* op);









#endif