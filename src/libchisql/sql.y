%{

#include <stdio.h>
#include <stdlib.h>
#include <chidb/chidb.h>
#include <chisql/chisql.h>
#include "sql-lexer.h"

#define YYERROR_VERBOSE

void yyerror(const char *s);
int yylex(void);
extern int yylineno;
#define YYDEBUG 0
int yydebug=0;
int to_print = 0;
int num_stmts = 0;

chisql_statement_t *__stmt;

%}

%union {
	double dval;
	int ival;
	char *strval;
	Literal_t *lval;
	Constraint_t *constr;
	ForeignKeyRef_t fkeyref;
	Column_t *col;
	KeyDec_t *kdec;
	StrList_t *slist;
	Insert_t *ins;
	Condition_t *cond;
	Expression_t *expr;
	ColumnReference_t *colref;
	Delete_t *del;
	SRA_t *sra;
	ProjectOption_t *opt;
	TableReference_t *tref;
	Table_t *tbl;
	JoinCondition_t *jcond;
	Index_t *idx;
	Create_t *cre;
}

%token CREATE TABLE INSERT INTO SELECT FROM WHERE FULL
%token PRIMARY FOREIGN KEY DEFAULT CHECK NOT TOKEN_NULL
%token AND OR NEQ GEQ LEQ REFERENCES ORDER BY DELETE
%token AS INT BYTE DOUBLE CHAR VARCHAR TEXT USING CONSTRAINT
%token JOIN INNER OUTER LEFT RIGHT NATURAL CROSS UNION BOWTIE
%token VALUES AUTO_INCREMENT ASC DESC UNIQUE IN ON
%token COUNT SUM AVG MIN MAX INTERSECT EXCEPT DISTINCT
%token CONCAT TRUE FALSE CASE WHEN DECLARE BIT GROUP
%token INDEX EXPLAIN
%token <strval> IDENTIFIER
%token <strval> STRING_LITERAL
%token <dval> DOUBLE_LITERAL
%token <ival> INT_LITERAL

%type <ival> column_type bool_op comp_op select_combo
%type <ival> function_name opt_distinct join opt_unique
%type <strval> column_name table_name opt_alias 
%type <strval> index_name column_name_or_star
%type <slist> column_names_list opt_column_names
%type <constr> opt_constraints constraints constraint
%type <lval> literal_value values_list in_statement
%type <fkeyref> references_stmt
%type <col> column_dec column_dec_list
%type <kdec> key_dec opt_key_dec_list key_dec_list
%type <ins> insert_into
%type <cond> condition bool_term where_condition opt_where_condition
%type <expr> expression mulexp primary expression_list term
%type <colref> column_reference
%type <del> delete_from
%type <sra> select select_statement table
%type <opt> order_by group_by opt_options
%type <tref> table_ref
%type <tbl> create_table
%type <jcond> join_condition opt_join_condition
%type <idx> create_index
%type <cre> create

%start sql_queries

%%

sql_queries
	: sql_query
	| sql_queries sql_query
	;

sql_query
	: sql_line ';'         { __stmt->explain = false; }
	| EXPLAIN sql_line ';' { __stmt->explain = true; }
	;

sql_line
	: create 		{ __stmt->stmt.create = $1; __stmt->type = STMT_CREATE; }
	| select 		{ __stmt->stmt.select = $1; __stmt->type = STMT_SELECT; }
	| insert_into 	{ __stmt->stmt.insert = $1; __stmt->type = STMT_INSERT; }
	| delete_from 	{ __stmt->stmt.delete = $1; __stmt->type = STMT_DELETE; }
	| /* empty */
	;

create
	: create_table { $$ = Create_fromTable($1); }
	| create_index { $$ = Create_fromIndex($1); }
	;

create_index
        : CREATE opt_unique INDEX index_name ON table_name '(' column_name ')'
		{ 
			$$ = Index_make($4, $6, $8); 
		  	if ($2 == UNIQUE) $$ = Index_makeUnique($$); 
		}
	;

opt_unique
	: UNIQUE { $$ = UNIQUE; }
	| /* empty */ { $$ = 0; }
	;

index_name
	: IDENTIFIER
	;

create_table
	: CREATE TABLE table_name '(' column_dec_list opt_key_dec_list ')' 
		{
			$$ = Table_make($3, $5, $6);
		}
	;

column_dec_list
	: column_dec
	| column_dec_list ',' column_dec { $$ = Column_append($1, $3); }
	;

column_dec
	: column_name column_type opt_constraints 
		{ 
			/*printf("column '%s' (%d)\n", $1, $2);*/
			$$ = Column($1, $2, $3);
		}
	;

column_type
	: INT 			{ $$ = TYPE_INT; }
	| DOUBLE 		{ $$ = TYPE_DOUBLE; }
	| CHAR 			{ $$ = TYPE_CHAR; }
	| VARCHAR 		{ $$ = TYPE_TEXT; }
	| TEXT 			{ $$ = TYPE_TEXT; }
	| column_type '(' INT_LITERAL ')' 
		{ 
			$$ = $1;
			if ($3 <= 0) {
				fprintf(stderr, "Error: sizes must be greater than 0 (line %d).\n", yylineno);
				exit(1);
			}
			Column_setSize($3);
		}
	;

opt_key_dec_list
	: ',' key_dec_list {$$ = $2;}
	| /* empty */		 {$$ = NULL; }
	;

key_dec_list
	: key_dec
	| key_dec_list ',' key_dec { $$ = KeyDec_append($1, $3); }
	;

key_dec
	: PRIMARY KEY '(' column_names_list ')' 
		{ $$ = PrimaryKeyDec($4); }
	| FOREIGN KEY '(' column_name ')' references_stmt 
		{$$ = ForeignKeyDec(ForeignKeyRef_makeFull($4, $6)); }

references_stmt
	: REFERENCES table_name { $$ = ForeignKeyRef_make($2, NULL); }
	| REFERENCES table_name '(' column_name ')' { $$ = ForeignKeyRef_make($2, $4); }
	;

opt_constraints
	: constraints
	| /* empty */ { $$ = NULL; }
	;

constraints
	: constraint { $$ = Constraint_append(NULL, $1); 
						/*printf("new constraint:");
						Constraint_print($1);
						printf("created a vector of constraints\n");
						Constraint_printList($$);*/
					 }
	| constraint constraints { $$ = Constraint_append($2, $1); 
										/*printf("appended a constraint\n");
										Constraint_printList($$);*/
									}
	;

constraint
	: NOT TOKEN_NULL { $$ = NotNull(); }
	| UNIQUE				{ $$ = Unique(); }
	| PRIMARY KEY 		{ $$ = PrimaryKey(); }
	| FOREIGN KEY references_stmt { $$ = ForeignKey($3); }
	| DEFAULT literal_value { $$ = Default($2); }
	| AUTO_INCREMENT { $$ = AutoIncrement(); }
	| CHECK condition { $$ = Check($2); }
	;

select
	: select_statement
	| select select_combo select_statement 
		{ 
			$$ = ($2 == UNION) ? SRAUnion($1, $3) :
				  ($2 == INTERSECT) ? SRAIntersect($1, $3) :
				  SRAExcept($1, $3);
		}
	;

select_combo
	: UNION {$$ = UNION;}
	| INTERSECT {$$ = INTERSECT;}
	| EXCEPT {$$ = EXCEPT;}
	;

select_statement
	: SELECT opt_distinct expression_list FROM table opt_where_condition opt_options
		{
			if ($6 != NULL) 
				$$ = SRAProject(SRASelect($5, $6), $3);
			else
				$$ = SRAProject($5, $3);
			if ($7 != NULL)
				$$ = SRA_applyOption($$, $7); 
			if ($2 == DISTINCT)
				$$ = SRA_makeDistinct($$);
		}
	| '(' select_statement ')' { $$ = $2; }
	;

opt_distinct
	: DISTINCT { $$ = DISTINCT;}
	| /* empty */ { $$ = 0; }
	;

opt_options
	: order_by {$$ = $1; }
	| group_by {$$ = $1; }
	| order_by group_by {$$ = ProjectOption_combine($1, $2);}
	| group_by order_by {$$ = ProjectOption_combine($1, $2);}
	| /* empty */ { $$ = NULL; }
	;

opt_where_condition
	: where_condition {$$ = $1;}
	| /* empty */		{$$ = NULL;}
	;

where_condition
	: WHERE condition { $$ = $2; }
	;

group_by
	: GROUP BY expression { $$ = GroupBy_make($3); } 		
	;

order_by
	: ORDER BY expression 		{ $$ = OrderBy_make($3, ORDER_BY_ASC); }
	| ORDER BY expression ASC 	{ $$ = OrderBy_make($3, ORDER_BY_ASC); }
	| ORDER BY expression DESC { $$ = OrderBy_make($3, ORDER_BY_DESC); }
	;

condition
   : bool_term { $$ = $1; /*printf("Found condition: \n"); Condition_print($$); puts(""); */}
   | bool_term bool_op condition 
   	{ 
   		$$ = ($2 == AND) ? And($1, $3) : Or($1, $3); 
   		/* printf("Found condition: \n"); Condition_print($$); puts(""); */
   	}
   ;

bool_term
   : expression comp_op expression 
   	{
   		$$ = ($2 == '=') ? Eq($1, $3) :
   			  ($2 == '>') ? Gt($1, $3) :
   			  ($2 == '<') ? Lt($1, $3) :
   			  ($2 == GEQ) ? Geq($1, $3) :
   			  ($2 == LEQ) ? Leq($1, $3) :
   			  Not(Eq($1, $3));
   	}
   | expression in_statement { $$ = In($1, $2); }
   | '(' condition ')' 	{ $$ = $2; }
   | NOT bool_term 		{ $$ = Not($2); }
   ;

in_statement
	:  IN '(' values_list ')' { $$ = $3; }
	|	IN '(' select ')'
   	{
   		fprintf(stderr, "****WARNING: IN SELECT statement not yet supported\n");
   	}
   ;

bool_op
	: AND { $$ = AND; } 
	| OR { $$ = OR; }
	;

comp_op
	: '=' { $$ = '='; } 
	| '>' { $$ = '>'; } 
	| '<' { $$ = '<'; } 
	| GEQ { $$ = GEQ; } 
	| LEQ { $$ = LEQ; } 
	| NEQ { $$ = NEQ; } 
	;

expression_list
	: expression opt_alias { $$ = add_alias($1, $2); }
	| expression_list ',' expression opt_alias { $$ = append_expression($1, add_alias($3, $4)); }
	;

expression
	: expression '+' mulexp { $$ = Plus($1, $3); }
	| expression '-' mulexp { $$ = Minus($1, $3); }
	| mulexp						{ $$ = $1; }
	;

mulexp
	: mulexp '*' primary 	{ $$ = Multiply($1, $3); }
	| mulexp '/' primary 	{ $$ = Divide($1, $3); }
	| mulexp CONCAT primary { $$ = Concat($1, $3); }
	| primary 					{ $$ = $1; }
	;

primary
	: '(' expression ')' 	{ $$ = $2; }
	| '-' primary 				{ $$ = Neg($2); }
	| term 						{ $$ = $1; } 
	;

term
	: literal_value			{ $$ = TermLiteral($1); }
	| TOKEN_NULL				{ $$ = TermNull(); }
	| column_reference		{ $$ = TermColumnReference($1); }
	| function_name '(' expression ')' { $$ = TermFunction($1, $3); }
	;

column_reference
	: column_name_or_star { $$ = ColumnReference_make(NULL, $1); }
	| table_name '.' column_name_or_star
		 { $$ = ColumnReference_make($1, $3); }
	;

opt_alias
	: AS IDENTIFIER { $$ = $2; }
	| IDENTIFIER
	| /* empty */ { $$ = NULL; }
	;

function_name
	: COUNT { $$ = FUNC_COUNT; }
	| SUM { $$ = FUNC_SUM; }
	| AVG { $$ = FUNC_AVG; }
	| MIN { $$ = FUNC_MIN; }
	| MAX{ $$ = FUNC_MAX; }
	;

column_name_or_star
	: '*' { $$ = strdup("*"); }
	| column_name
	;

column_name
	: IDENTIFIER
	;

table_name
	: IDENTIFIER
	;

table
	: table_ref { $$ = SRATable($1); }
	| table default_join table_ref opt_join_condition { $$ = SRAJoin($1, SRATable($3), $4); }
	| table join table_ref opt_join_condition
		{
			switch ($2) {
				case SRA_NATURAL_JOIN:
					$$ = SRANaturalJoin($1, SRATable($3)); 
					if ($4) {
						fprintf(stderr, 
								  "Line %d: WARNING: a NATURAL join cannot have an ON "
								  "or USING clause. This will be ignored.\n", yylineno);
					}
					break;
				case SRA_LEFT_OUTER_JOIN:
					$$ = SRALeftOuterJoin($1, SRATable($3), $4); break;
				case SRA_RIGHT_OUTER_JOIN:
					$$ = SRARightOuterJoin($1, SRATable($3), $4); break;
				case SRA_FULL_OUTER_JOIN:
					$$ = SRAFullOuterJoin($1, SRATable($3), $4); break;
			}
		}
	;

opt_join_condition
	: join_condition
	| /* empty */	  { $$ = NULL; }
	;

join_condition
	: ON condition { $$ = On($2); }
	| USING '(' column_names_list ')' { $$ = Using($3); }
	;

table_ref
	: table_name opt_alias { $$ = TableReference_make($1, $2);}
	;

join
	: LEFT opt_outer JOIN {$$ = SRA_LEFT_OUTER_JOIN; }
	| RIGHT opt_outer JOIN { $$ = SRA_RIGHT_OUTER_JOIN; }
	| FULL opt_outer JOIN { $$ = SRA_FULL_OUTER_JOIN; }
	| NATURAL JOIN { $$ = SRA_NATURAL_JOIN; }
	| BOWTIE { $$ = SRA_NATURAL_JOIN; }
	;

default_join
	: ',' | JOIN | CROSS JOIN | INNER JOIN
	;

opt_outer
	: OUTER
	| /* empty */
	;

insert_into
	: INSERT INTO table_name opt_column_names VALUES '(' values_list ')'
		{
			$$ = Insert_make($3, $4, $7);
		}
	;

opt_column_names
	: '(' column_names_list ')' { $$ = $2; }
	| /* empty */					 { $$ = NULL; }
	;

column_names_list
	: column_name { $$ = StrList_make($1); }
	| column_names_list ',' column_name { $$ = StrList_append($1, StrList_make($3)); }
	;

values_list
	: literal_value
	| values_list ',' literal_value 
		{ 
			$$ = Literal_append($1, $3); 

		}
	;

literal_value
	: INT_LITERAL { $$ = litInt($1); }
	| DOUBLE_LITERAL { $$ = litDouble($1); }
	| STRING_LITERAL
		{
			if (strlen($1) == 1)
				$$ = litChar($1[0]);
			else
				$$ = litText($1);
		}
	;

delete_from
	: DELETE FROM table_name where_condition
		{
			$$ = Delete_make($3, $4);
		}
	;

%%

void yyerror(const char *s) {
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
}

int chisql_stmt_print(chisql_statement_t *stmt)
{
    switch(stmt->type)
    {
    case STMT_CREATE:
        Create_print(stmt->stmt.create);
        break;
    case STMT_SELECT:
        SRA_print(stmt->stmt.select);
        break;
    case STMT_INSERT:
        Insert_print(stmt->stmt.insert);
        break;
    case STMT_DELETE:
        Delete_print(stmt->stmt.delete);
        break;
    }

    return 0;
}


char *__sql_semicolon(const char *sql)
{
  int len = strlen(sql);
  char *t = strdup(sql);
  if (t[len-1]!=';') {
    t = realloc(t,len+2);
    t[len]=';';
    t[len+1]=0;
  }
  return t;
}

int chisql_parser(const char *sql, chisql_statement_t **stmt)
{
  int rc;
  
  __stmt = malloc(sizeof(chisql_statement_t));
  char *tsql = __sql_semicolon(sql);
    
  YY_BUFFER_STATE my_string_buffer = yy_scan_string (tsql);
  rc = yyparse();
  yy_delete_buffer (my_string_buffer);
  
  if (rc == 0) {
    __stmt->text = tsql; /* strdup(sql); */
    *stmt = __stmt;
    return CHIDB_OK;
  } else {
    fprintf(stderr,"invalid sql: \"%s\"\n", tsql);
    free(__stmt);
    return CHIDB_EINVALIDSQL;
  }

}
