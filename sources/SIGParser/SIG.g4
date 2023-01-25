grammar SIG;

options  
  {  
  	language = Cpp;
  }

parse
 : (layout_definition|table_definition|rt_definition|compute_pso_definition|graphics_pso_definition|rtx_pso_definition|rtx_pass_definition|rtx_raygen_definition|COMMENT)* EOF
 ;


bind_option: (owner_id '::')? value_id;

options_assign: ASSIGN bind_option;

option: name_id options_assign?;

 
option_block: 
OSBRACE 
option
(',' option)*
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
 
 
  define_declaration
 : option_block*? 'define' name_id (ASSIGN array_value_ids)? SCOL
 ;

  rtv_formats_declaration
 : option_block*? 'rtv' ASSIGN array_value_ids SCOL
 ;

blends_declaration
 : option_block*? 'blend' ASSIGN array_value_ids SCOL
 ;

pso_param: pso_param_id ASSIGN value_id SCOL;
class_no_template: ID;
type_with_template:class_no_template (LT template_id GT)?;
inherit_id: ID;
name_id: ID;
type_id: type_with_template;
option_id: ID;
owner_id: ID;
template_id: ID;

value_id: ID |INT_SCALAR|FLOAT_SCALAR | bool_type;

insert_block: INSERT_BLOCK;


path_id: (ID '/' )*? ID; 

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
 | insert_block
 | COMMENT

 ;
 
table_block
 : table_stat*
 ;

table_definition
 : option_block*? STRUCT name_id inherit? OBRACE  table_block CBRACE
 ;


rt_color_declaration
 : type_id name_id SCOL
 ;


rt_ds_declaration
 : DSV name_id SCOL
 ;

rt_stat
 : rt_color_declaration
 | rt_ds_declaration
 | COMMENT
 ;

rt_block
 : rt_stat*
 ;


rt_definition
 : RT name_id OBRACE rt_block CBRACE
 ;

array_value_holder: value_id;

array_value_ids: '{' array_value_holder (',' array_value_holder)* '}';

root_sig: ROOTSIG ASSIGN name_id SCOL;

shader: option_block*? shader_type ASSIGN path_id SCOL;

compute_pso_stat
 : root_sig
 | shader
 | define_declaration
 | COMMENT
 ;
compute_pso_block: compute_pso_stat*;
compute_pso_definition: COMPUTE_PSO name_id inherit? OBRACE compute_pso_block CBRACE;


graphics_pso_stat
 : root_sig
 | shader
 | define_declaration
 | rtv_formats_declaration
 | blends_declaration
 | pso_param
 | COMMENT
 ;
graphics_pso_block: graphics_pso_stat*;
graphics_pso_definition: option_block*? GRAPHICS_PSO name_id inherit? OBRACE graphics_pso_block CBRACE;



rtx_pso_stat
 : root_sig
 | COMMENT
 ;
rtx_pso_block: rtx_pso_stat*;
rtx_pso_definition: RAYTRACE_PSO name_id inherit? OBRACE rtx_pso_block CBRACE;



rtx_pass_stat
 : shader
 | COMMENT
 | pso_param
 ;
rtx_pass_block: rtx_pass_stat*;
rtx_pass_definition: option_block*? RAYTRACE_PASS name_id inherit? OBRACE rtx_pass_block CBRACE;


rtx_raygen_stat
 : shader
 | COMMENT
 ;
rtx_raygen_block: rtx_raygen_stat*;
rtx_raygen_definition: option_block*? RAYTRACE_RAYGEN name_id inherit? OBRACE rtx_raygen_block CBRACE;



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
COMPUTE_PSO: 'ComputePSO';
GRAPHICS_PSO: 'GraphicsPSO';
RAYTRACE_PSO: 'RaytracePSO';
RAYTRACE_RAYGEN: 'RaytraceRaygen';
RAYTRACE_PASS: 'RaytracePass';


SLOT: 'slot';
RT: 'rt';
RTV: 'RTV';
DSV: 'DSV';
ROOTSIG: 'root';
shader_type: 
'compute'
|'vertex'
|'pixel'
|'domain'
|'hull'
|'geometry'
|'miss'
|'closest_hit'
|'raygen'
|'amplification'
|'mesh'
;
pso_param_id: 
'ds' 
| 'cull' 
| 'depth_func' 
| 'depth_write'
| 'conservative' 
| 'enable_depth' 
| 'topology'
| 'enable_stencil'
| 'stencil_func'
| 'stencil_pass_op'
| 'stencil_read_mask'
| 'stencil_write_mask'
| 'recursion_depth'
| 'payload'
| 'per_material'
| 'local'
 ;

ID
 : [a-zA-Z_] [a-zA-Z_0-9]*
 ;

bool_type
 : TRUE | FALSE
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
 
INSERT_START: '%{';
INSERT_END: '}%';
INSERT_BLOCK 
    :   INSERT_START (.)*? INSERT_END
    ;