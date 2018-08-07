#include "Lval.h"

/* Define Lval Constructors */
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


/* Define Deconstructor for Lval */
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