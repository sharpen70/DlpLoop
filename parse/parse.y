%{
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include "Vocabulary.h"
#include "Rule.h"

extern "C" {
	void yyerror(const char *s);
	extern int yylex(void);
}
 
extern vector<Rule> G_NLP;

int id;

void yyerror(const char* s)
{
	printf("Parser error: %s\n", s);
}

%}

%union {
    char* s;
    int i;
    struct __literals* l;
    struct __rule* r;
    struct __head* h;
}

%token <s> S_ATOM
%token <s> S_NEGA
%token <s> S_IMPL
%token <s> LPAREN
%token <s> RPAREN
%token <s> COMMA
%token <s> PERIOD
%token <s> DIS

%type <s> term terms
%type <i> literal atom
%type <l> literals
%type <r> rule
%type <h> head

%left S_IMPL

%%
nlp 
    : rules {
        printf("dlp\n");
    }
    |
;

rules
    : rules rule {
        Rule rule($2);
        G_NLP.push_back(rule);
    }
    | rule {
        Rule rule($1);
        G_NLP.push_back(rule);
    }
;

rule 
    : head PERIOD{
        $$ = (_rule*)malloc(sizeof(_rule));
        $$->head_length = $1->length;
        for(int i = 0; i < ($1->length); ++i) {
            $$->head[i] = $1->atoms[i];
        }
        $$->body_length = 0;
        $$->type = FACT;
    }
    | head S_IMPL literals PERIOD{
        $$ = (_rule*)malloc(sizeof(_rule));
        $$->type = RULE;
        for(int i = 0; i < ($1->length); ++i) {
            $$->head[i] = $1->atoms[i];
        }
        for(int i = 0; i < ($3->length); ++i) {
            $$->body[i] = $3->atoms[i];
        }
        $$->head_length = $1->length;
        $$->body_length = $3->length;
    }
    | S_IMPL literals PERIOD{
        $$ = (_rule*)malloc(sizeof(_rule));
        $$->head_length = 0;
        $$->type = CONSTRANT;
        for(int i = 0; i < ($2->length); i++) {
            $$->body[i] = $2->atoms[i];
        }
        $$->body_length = $2->length;
    }
    | S_IMPL PERIOD {
    }
;

head
    : head DIS atom {
        $1->atoms[$1->length] = $3;
        $1->length++;        
    }
    | atom {
        $$ = (_head*)malloc(sizeof(_head));
        memset($$->atoms, 0, sizeof(int) * MAX_HEAD_LENGTH);
        $$->atoms[0] = $1;
        $$->length = 1;
    }

literals
    : literals COMMA literal {
        $1->atoms[$1->length] = $3;
        $1->length++;
    }
    | literal {
        $$ = (__literals*)malloc(sizeof(_literals));
        memset($$->atoms, 0, sizeof(int) * MAX_ATOM_LENGTH);
        $$->atoms[0] = $1;
        $$->length = 1;
    }
;

literal
    : S_NEGA atom {
        $$ = -1 * $2;
    }
    | atom {
        $$ = $1;
    }
;

atom
    : S_ATOM LPAREN terms RPAREN {
        char str_buff[512];
        
        sprintf(str_buff, "%s(%s)", $1, $3);
        int id = Vocabulary::instance().queryAtom(strdup(str_buff));
        if(id < 0) id = Vocabulary::instance().addAtom(strdup(str_buff));
        $$ = id;
    } 
    | S_ATOM {
        int id = Vocabulary::instance().queryAtom($1);
        if(id < 0) id = Vocabulary::instance().addAtom($1);
        $$ = id;
    }
;

terms
    : terms COMMA term {
        char str_buff[512];
        
        sprintf(str_buff, "%s,%s", $1, $3);
        $$ = strdup(str_buff);
    }
    | term {
        $$ = strdup($1);
    }
;

term
    : S_ATOM {
        $$ = strdup($1);
    }
;
%%
