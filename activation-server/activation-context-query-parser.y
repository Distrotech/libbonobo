%{
void yyerror(char *s);
int yylex();

%}

%union
{
  char *val_string;
  char **val_stringv;
  double val_number;
  gboolean val_boolean;
}


%token NOT
%token EQ NEQ LEQ GEQ LE GR
%token OR AND

%type <val_string> STRING
%type <val_string> ID
%type <val_number> NUMBER
%type <val_boolean> BOOLEAN



%%

%%
