%{
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"
#include "y.tab.h"

int charPos=1;
char *content;
char *pos;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}
/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

%}
  /* You can add lex definitions here. */

digits [0-9]+

%s COMMENT STR

%%
  /* 
  * Below are some examples, which you can wipe out
  * and write reguler expressions and actions of your own.
  */ 




<INITIAL>","	 {adjust(); return COMMA;}
<INITIAL>":"  {adjust(); return COLON;}
<INITIAL>";"  {adjust(); return SEMICOLON;}
<INITIAL>"("  {adjust(); return LPAREN;}
<INITIAL>")"  {adjust(); return RPAREN;}
<INITIAL>"["  {adjust(); return LBRACK;}
<INITIAL>"]"  {adjust(); return RBRACK;}
<INITIAL>"{"  {adjust(); return LBRACE;}
<INITIAL>"}"  {adjust(); return RBRACE;}
<INITIAL>"."  {adjust(); return DOT;}
<INITIAL>"+"  {adjust(); return PLUS;}
<INITIAL>"-"  {adjust(); return MINUS;}
<INITIAL>"*"  {adjust(); return TIMES;}
<INITIAL>"/"  {adjust(); return DIVIDE;}
<INITIAL>"="  {adjust(); return EQ;}
<INITIAL>"<>"  {adjust(); return NEQ;}
<INITIAL>"<"  {adjust(); return LT;}
<INITIAL>"<="  {adjust(); return LE;}
<INITIAL>">"  {adjust(); return GT;}
<INITIAL>">="  {adjust(); return GE;}
<INITIAL>"&"  {adjust(); return AND;}
<INITIAL>"|"  {adjust(); return OR;}
<INITIAL>":="  {adjust(); return ASSIGN;}
<INITIAL>array  {adjust(); return ARRAY;}
<INITIAL>if  {adjust(); return IF;}
<INITIAL>then  {adjust(); return THEN;}
<INITIAL>else  {adjust(); return ELSE;}
<INITIAL>while  {adjust(); return WHILE;}
<INITIAL>for  {adjust(); return FOR;}
<INITIAL>to  {adjust(); return TO;}
<INITIAL>do  {adjust(); return DO;}
<INITIAL>let  {adjust(); return LET;}
<INITIAL>in  {adjust(); return IN;}
<INITIAL>end  {adjust(); return END;}
<INITIAL>of  {adjust(); return OF;}
<INITIAL>break  {adjust(); return BREAK;}
<INITIAL>nil  {adjust(); return NIL;}
<INITIAL>function  {adjust(); return FUNCTION;}
<INITIAL>var  {adjust(); return VAR;}
<INITIAL>type  {adjust(); return TYPE;}


<INITIAL>(" "|"\t")+  {adjust(); continue;} 
<INITIAL>\n	 {adjust(); EM_newline(); continue;}

<INITIAL>[_a-zA-Z][_a-zA-Z0-9]* {adjust(); yylval.sval=String(yytext); return ID;}
<INITIAL>[0-9]+	 {adjust(); yylval.ival=atoi(yytext); return INT;}
<INITIAL>"/*"	 {adjust(); BEGIN COMMENT;}
<COMMENT>"*/"	 {adjust(); BEGIN INITIAL;}
<COMMENT>.	 {adjust();}
<COMMENT>\n	 {adjust();}

<INITIAL>\"  {adjust(); content = (char*)malloc(100); pos = content; BEGIN STR;}
<STR>\"  {charPos+=yyleng; pos++; *pos='\0';if(strcmp(content, "") == 0) yylval.sval=""; else yylval.sval=String(content); BEGIN INITIAL; return STRING; adjust();}
<STR>\\n  {charPos+=yyleng; *pos = '\n'; pos++;}
<STR>"\t"  {charPos+=yyleng; *pos = '\t'; pos++;}
<STR>.  {charPos+=yyleng; strncpy(pos, yytext, 1); pos++;}


<INITIAL>.	 {adjust(); EM_error(EM_tokPos,"illegal token");}

