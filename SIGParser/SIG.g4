grammar SIG;

options  
  {  
  	language = Cpp;
  }

parse
 : (layout_definition|table_definition|COMMENT)* EOF
 ;

bind_option: (owner_id '::')? name_id;

options_assign: ASSIGN bind_option;

options: name_id options_assign?;

 
option_block: 
OSBRACE 
options*
CSBRACE ;

 array_count_id: INT_SCALAR;
 
 array: OSBRACE array_count_id? CSBRACE;
 
 value_declaration
 : option_block*? type_id name_id array? SCOL
 ;
 
 slot_declaration
 : SLOT name_id SCOL
 ;
 
  sampler_declaration
 : 'Sampler' name_id ASSIGN value_id SCOL
 ;
 
templated: LT template_id GT;
type_with_template:ID templated?;
inherit_id: ID;
name_id: ID;
type_id: type_with_template;
option_id: ID;
owner_id: ID;
template_id: ID;
value_id: ID;

inherit
 : ':' inherit_id (',' inherit_id)*?
 ;
  
layout_stat
 : slot_declaration
 | sampler_declaration
 | COMMENT
 ;
 
layout_block
 : layout_stat*
 ;

layout_definition
 : LAYOUT name_id inherit? OBRACE layout_block CBRACE
 ;


table_stat
 :  value_declaration
 | COMMENT
 ;
 
table_block
 : table_stat*
 ;

table_definition
 : option_block*? STRUCT name_id inherit? OBRACE  table_block CBRACE
 ;

	
OR : '||';
AND : '&&';
EQ : '==';
NEQ : '!=';
GT : '>';
LT : '<';
GTEQ : '>=';
LTEQ : '<=';
PLUS : '+';
MINUS : '-';
MULT : '*';
DIV : '/';
MOD : '%';
POW : '^';
NOT : '!';

SCOL : ';';
ASSIGN : '=';
OPAR : '(';
CPAR : ')';
OBRACE : '{';
CBRACE : '}';

OSBRACE : '[';
CSBRACE : ']';

TRUE : 'true';
FALSE : 'false';
LOG : 'log';

LAYOUT: 'layout';
STRUCT: 'struct';
SLOT: 'slot';

ID
 : [a-zA-Z_] [a-zA-Z_0-9]*
 ;

INT_SCALAR
 : [0-9]+
 ;

FLOAT_SCALAR
 : [0-9]+ '.' [0-9]* 
 | '.' [0-9]+
 ;

STRING
 : '"' (~["\r\n] | '""')* '"'
 ;

COMMENT
 : '#' ~[\r\n]* -> skip
 ;

SPACE
 : [ \t\r\n] -> skip
 ;
 
 