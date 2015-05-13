%{
#include <chisql/chisql.h>
#include "sql-parser.h"
#include <math.h>

#define YY_NO_INPUT

extern int yydebug;
int comment_start_lineno;
%}

%option nounput
%option noyywrap
%option yylineno
%option case-insensitive

%x BLOCK_COMMENT 
%x LINE_COMMENT

%%

explain                     { return EXPLAIN; }
create 						{ return CREATE; }
table 						{ return TABLE; }
index 						{ return INDEX; }
insert 						{ return INSERT; }
into 							{ return INTO; }
select 						{ return SELECT; }
from 							{ return FROM; }
where 						{ return WHERE; }
primary 						{ return PRIMARY; }
foreign 						{ return FOREIGN; }
key 							{ return KEY; }
default 						{ return DEFAULT; }
check 						{ return CHECK; }
not 							{ return NOT; }
null 							{ return TOKEN_NULL; }
and 							{ return AND; }
or 							{ return OR; }
"!="                    { return NEQ; }
"<>"        				{ return NEQ; }
">="							{ return GEQ; }
"<=" 							{ return LEQ; }
"||"                    { return CONCAT; }
"|><|"                    { return BOWTIE; }
references 					{ return REFERENCES; }
order 						{ return ORDER; }
by 							{ return BY; }
delete 						{ return DELETE; }
as 							{ return AS; }
byte                                                    { return INT; }
int 							{ return INT; }
integer                 { return INT; }
double 						{ return DOUBLE; }
char 							{ return CHAR; }
varchar 						{ return VARCHAR; }
text 							{ return TEXT; }
join 							{ return JOIN; }
inner 						{ return INNER; }
outer 						{ return OUTER; }
full                    { return FULL; }
left 							{ return LEFT; }
right 						{ return RIGHT; }
natural 						{ return NATURAL; }
union 						{ return UNION; }
values 						{ return VALUES; }
auto_increment 			{ return AUTO_INCREMENT; }
asc 							{ return ASC; }
desc 							{ return DESC; }
unique                  { return UNIQUE; }
in                      { return IN; }
count                   { return COUNT; }
sum                     { return SUM; }
min                     { return MIN; }
max                     { return MAX; }
avg                     { return AVG; }
on                      { return ON; }
using                   { return USING; }
true                    { return TRUE; }
false                   { return FALSE; }
case                    { return CASE; }
when                    { return WHEN; }
bit                     { return BIT; }
group                   { return GROUP; }
distinct                { return DISTINCT; }
\/\*                    { BEGIN(BLOCK_COMMENT); comment_start_lineno = yylineno; }
<BLOCK_COMMENT>\*\/     { BEGIN(INITIAL); }
<BLOCK_COMMENT><<EOF>>  { fprintf(stderr, "Warning: unclosed comment beginning on line %d\n",
                                  ++comment_start_lineno); return EOF; }
<BLOCK_COMMENT>\n       { yylineno++; }
<BLOCK_COMMENT>.        { /* ignore */ }
"--"                    { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\n        { BEGIN(INITIAL); yylineno++; }
<LINE_COMMENT>.         { /* ignore */ }
[a-zA-Z][a-zA-Z0-9_]*   { yylval.strval = strdup(yytext); 
                          if (yydebug) printf("lexed identifier '%s'\n", yytext); 
                          return IDENTIFIER; }
((\"[^\"]*\")|(\'[^\']*\')) { yylval.strval = strndup(yytext+1, strlen(yytext) - 2); return STRING_LITERAL; }
[+-]?[0-9]+ 				{ yylval.ival = atoi(yytext); return INT_LITERAL; }
([0-9]+|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)	{ yylval.dval = atof(yytext); return DOUBLE_LITERAL; }
[ \t\r]+                  { /* ignore */ }
\n                      { yylineno++; }
.                       { if (yydebug) printf("lexed single character '%c'\n", yytext[0]); 
                          return yytext[0]; }

%%
