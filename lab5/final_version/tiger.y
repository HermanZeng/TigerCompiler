%{
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h" 
#include "errormsg.h"
#include "absyn.h"

int yylex(void); /* function prototype */

A_exp absyn_root;

void yyerror(char *s)
{
 EM_error(EM_tokPos, "%s", s);
 exit(1);
}
%}


%union {
	int pos;
	int ival;
	string sval;
	A_var var;
	A_exp exp;
  A_expList expList;
  A_decList decList;
  A_dec dec;
  A_ty ty;
  A_field field;
  A_fieldList fieldList;
  A_efield efield;
  A_efieldList efieldList;
  A_namety nametyy;
  A_nametyList nametyyList;
  A_fundec fundecc;
  A_fundecList fundeccList;
	/* et cetera */
	}

%token <sval> ID STRING
%token <ival> INT

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE 
  UMINUS

%type <exp> exp program varExp nilExp intExp stringExp callExp opExp andExp orExp recordExp seqExp assignExp ifExp whileExp forExp breakExp arrayExp letExp
%type <expList> exps1 exps2
%type <decList> decs
%type <dec> dec typeDec varDec funcDec
%type <ty> ty nameTy recordTy arrayTy
%type <field> field
%type <fieldList> fields
%type <efield> efield
%type <efieldList> efields
%type <var> var
%type <nametyy> typedec
%type <nametyyList> typedecs
%type <fundecc> funcdec
%type <fundeccList> funcdecs
/* et cetera */

%start program

%left AND OR
%left LE LT GE GT EQ NEQ
%left PLUS MINUS
%left TIMES DIVIDE
%left UMINUS

%%

program:   exp    {absyn_root=$1;}

decs:   dec decs    {$$=A_DecList($1, $2);}
|   dec    {$$=A_DecList($1, NULL);}

dec:   typeDec    {$$=$1;}
| varDec    {$$=$1;}
| funcDec    {$$=$1;}

typeDec:   typedecs    {$$=A_TypeDec(EM_tokPos, $1);}

typedecs:   typedec typedecs    {$$=A_NametyList($1, $2);}
|   typedec    {$$=A_NametyList($1, NULL);}

typedec:   TYPE ID EQ ty    {$$=A_Namety(S_Symbol($2), $4);}

ty:   nameTy    {$$=$1;}
|   recordTy    {$$=$1;}
|   arrayTy    {$$=$1;}

nameTy:   ID    {$$=A_NameTy(EM_tokPos,S_Symbol($1));}
arrayTy:   ARRAY OF ID    {$$=A_ArrayTy(EM_tokPos,S_Symbol($3));}
recordTy:   LBRACE fields RBRACE    {$$=A_RecordTy(EM_tokPos, $2);}/*to be finished*/

field:   ID COLON ID    {$$=A_Field(EM_tokPos,S_Symbol($1),S_Symbol($3));}

fields:   field COMMA fields    {$$=A_FieldList($1, $3);}
| field    {$$=A_FieldList($1, NULL);}
|    {$$=NULL;}

funcDec:   funcdecs     {$$=A_FunctionDec(EM_tokPos, $1);}

funcdecs:   funcdec funcdecs     {$$=A_FundecList($1, $2);}
|   funcdec    {$$=A_FundecList($1, NULL);}

funcdec:   FUNCTION ID LPAREN fields RPAREN EQ exp    {$$=A_Fundec(EM_tokPos, S_Symbol($2), $4, NULL, $7);}
| FUNCTION ID LPAREN fields RPAREN COLON ID EQ exp    {$$=A_Fundec(EM_tokPos, S_Symbol($2), $4, S_Symbol($7), $9);}

varDec:   VAR ID ASSIGN exp    {$$=A_VarDec(EM_tokPos, S_Symbol($2), NULL, $4);}
|   VAR ID COLON ID ASSIGN exp    {$$=A_VarDec(EM_tokPos, S_Symbol($2), S_Symbol($4), $6);}

varExp:   var    {$$=A_VarExp(EM_tokPos, $1);}

var:   ID    {$$=A_SimpleVar(EM_tokPos, S_Symbol($1));}
|   ID LBRACK exp RBRACK    {$$=A_SubscriptVar(EM_tokPos, A_SimpleVar(EM_tokPos, S_Symbol($1)), $3);}
|   var DOT ID    {$$=A_FieldVar(EM_tokPos, $1, S_Symbol($3));}
|   var LBRACK exp RBRACK    {$$=A_SubscriptVar(EM_tokPos, $1, $3);}

exp:   varExp | nilExp | intExp | stringExp | callExp | opExp | andExp | orExp | recordExp | seqExp | assignExp | ifExp | whileExp | forExp | breakExp | arrayExp | letExp {$$=$1;}

nilExp:   NIL    {$$=A_NilExp(EM_tokPos);}

intExp:   INT    {$$=A_IntExp(EM_tokPos, $1);}

stringExp:   STRING    {$$=A_StringExp(EM_tokPos, $1);}

callExp:   ID LPAREN exps1 RPAREN    {$$=A_CallExp(EM_tokPos, S_Symbol($1), $3);}

opExp:   exp EQ exp    {$$=A_OpExp(EM_tokPos, A_eqOp, $1, $3);}
|   exp NEQ exp    {$$=A_OpExp(EM_tokPos, A_neqOp, $1, $3);}
|   exp LT exp    {$$=A_OpExp(EM_tokPos, A_ltOp, $1, $3);}
|   exp LE exp    {$$=A_OpExp(EM_tokPos, A_leOp, $1, $3);}
|   exp GT exp    {$$=A_OpExp(EM_tokPos, A_gtOp, $1, $3);}
|   exp GE exp    {$$=A_OpExp(EM_tokPos, A_geOp, $1, $3);}
|   exp PLUS exp    {$$=A_OpExp(EM_tokPos, A_plusOp, $1, $3);}
|   exp MINUS exp    {$$=A_OpExp(EM_tokPos, A_minusOp, $1, $3);}
|   exp TIMES exp    {$$=A_OpExp(EM_tokPos, A_timesOp, $1, $3);}
|   exp DIVIDE exp    {$$=A_OpExp(EM_tokPos, A_divideOp, $1, $3);}
|   MINUS exp %prec UMINUS    {$$=A_OpExp(EM_tokPos, A_minusOp, A_IntExp(EM_tokPos, 0), $2);}

andExp:   exp AND exp    {$$=A_IfExp(EM_tokPos, $1, $3, A_IntExp(EM_tokPos, 0));}
orExp:   exp OR exp    {$$=A_IfExp(EM_tokPos, $1, A_IntExp(EM_tokPos, 1), $3);}

recordExp:   ID LBRACE efields RBRACE    {$$=A_RecordExp(EM_tokPos, S_Symbol($1), $3);}

efield:   ID EQ exp    {$$=A_Efield(S_Symbol($1), $3);}

efields:   efield COMMA efields    {$$=A_EfieldList($1, $3);}
| efield    {$$=A_EfieldList($1, NULL);}
|    {$$=NULL;}

seqExp:   LPAREN exps2 RPAREN    {$$=A_SeqExp(EM_tokPos, $2);}

assignExp:   var ASSIGN exp    {$$=A_AssignExp(EM_tokPos, $1, $3);}

exps1:   exp COMMA exps1    {$$=A_ExpList($1, $3);}
|   exp    {$$=A_ExpList($1, NULL);}
|    {$$=NULL;}

exps2:   exp SEMICOLON exps2    {$$=A_ExpList($1, $3);}
|   exp    {$$=A_ExpList($1, NULL);}
|    {$$=NULL;}

ifExp:   IF exp THEN exp ELSE exp    {$$=A_IfExp(EM_tokPos, $2, $4, $6);}
|   IF exp THEN exp    {$$=A_IfExp(EM_tokPos, $2, $4, NULL);}	

whileExp:   WHILE exp DO exp    {$$=A_WhileExp(EM_tokPos, $2, $4);}

forExp:   FOR ID ASSIGN exp TO exp DO exp    {$$=A_ForExp(EM_tokPos, S_Symbol($2), $4, $6, $8);}

breakExp:   BREAK    {$$=A_BreakExp(EM_tokPos);}

arrayExp:   ID LBRACK exp RBRACK OF exp    {$$=A_ArrayExp(EM_tokPos, S_Symbol($1), $3, $6);}

letExp:   LET decs IN exps2 END    {$$=A_LetExp(EM_tokPos, $2, A_SeqExp(EM_tokPos, $4));}
