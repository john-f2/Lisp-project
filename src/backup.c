	//this mpca_lang function allows for language input such as 
	// 1 + 1 + (2 * 2)
	// this was my attempt as making these math operations not polish notation 
	// mpca_lang(MPCA_LANG_DEFAULT, 
	// 	"                                                           \
	// 	number   : /-?[0-9]+/ ;                                     \
	// 	operator : '+' | '-' | '*' | '/';                           \
	// 	expr     : <number> | <operator> | '(' <number> <operator> <expr>+ ')';    \
	// 	jlispy   : /^/ <number> <operator> <expr>+ /$/ ;                     \
	// 	",
	// 	Number, Operator, Expr, Jlispy);


//helpful code to print out the AST tree from mpc parse library
//helps with evaluation and create a recursion for the tree
			// mpc_ast_print(r.output);
			// puts("");

			// puts("----------");
			// puts("AST TREE");
			// puts("----------");
			// //gets the AST output
			// //the AST is formated as a tree
			// //the children nodes are accessed through a double pointer
			// //remeber to access fields from pointers, we use the -> notation rather 
			// //than . (these mpc_ast_t objects are structs, remember, c has no classes)
			// mpc_ast_t* a = r.output;

			// printf("Tag: %s\n", a->tag);
			// printf("Content: %s\n", a->contents);
			// printf("Number of children: %d\n", a->children_num);


			// //getting the first child
			// //remeber double pointers can be accessed like list 
			// mpc_ast_t* c0 = a->children[0];
			// printf("Tag: %s\n", c0->tag);
			// printf("Content: %s\n ", c0->contents);
			// printf("Number of children: %d\n", c0->children_num);