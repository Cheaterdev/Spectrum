grammar Prism;

options
  {
  	language = Cpp;
  }

// FUNC_BODY is only lexable right after a function signature: `)` or
// `) : SEMANTIC`. That position is unambiguous in Prism -- struct/PSO/enum bodies
// follow a name and value lists follow `=` -- so the lexer can decide it alone
// by remembering the last few tokens, without splitting this into separate
// lexer/parser grammars for a lexical mode.
@lexer::members {
	size_t last_types[3] = { 0, 0, 0 };

	bool at_function_body() const
	{
		return last_types[0] == CPAR
		    || (last_types[0] == ID && last_types[1] == COLON && last_types[2] == CPAR);
	}

	std::unique_ptr<antlr4::Token> nextToken() override
	{
		auto token = antlr4::Lexer::nextToken();
		if (token->getChannel() == antlr4::Token::DEFAULT_CHANNEL)
		{
			last_types[2] = last_types[1];
			last_types[1] = last_types[0];
			last_types[0] = token->getType();
		}
		return token;
	}
}

parse
 : (layout_definition|table_definition|rt_definition|workgraph_pso_definition|compute_pso_definition|graphics_pso_definition|rtx_pso_definition|rtx_pass_definition|rtx_raygen_definition|pass_definition|view_definition|pipeline_definition|enum_definition|const_definition|COMMENT)* EOF
 ;

// A top-level named constant: `const Name = value;`. Value reuses bind_option
// (literal, an owner::field context reference, piped flags, or a raw
// backtick-escaped C++ expression) -- whatever a PassNode field's own option
// value already supports, so a computed constant (e.g. `const MaxDispatchEntries
// = \`MaxLevels * 2048\`;`) can reference an earlier const by its generated,
// unqualified name (see constants.jinja -- all consts land in one namespace,
// in declaration order, exactly like ordinary C++ initialization order rules).
const_definition
 : 'const' name_id options_assign SCOL
 ;


bind_option
 : (owner_id '::')? flag_value_holder (PIPE flag_value_holder)+
 | raw_value
 | cond_expr
 ;

// An option value captured as an ORDERED TERM SEQUENCE rather than a
// precedence-parsed tree. Codegen never evaluates these -- it only renders the
// terms back to C++ in source order and reports which Table:: context fields
// were named -- so emitting the author's own tokens verbatim reproduces both
// their parenthesisation and C++'s precedence exactly. A real expression
// grammar would buy nothing here and could silently change the meaning of an
// existing condition, which is the one failure this must not have.
//
// A plain `[Always = Read]` is just a one-term sequence, so this subsumes the
// old `(owner_id '::')? value_id` alternative; the flag alternative stays
// ahead of it because PIPE is deliberately not a cond_op.
cond_expr : cond_term+ ;

cond_term
 : qualified_ref
 | function_id
 | call
 | member_ref
 | value_id
 | cond_op
 ;

// A call whose arguments are themselves expressions:
// `area(tiles(ViewportContext::frame_size, 16))`. The arguments are bounded by
// the call's own parentheses, which keeps their commas from being read as the
// commas between options. function_id stays ahead of it, so `exists(X)` and
// other plain-argument calls parse exactly as before.
call : ID OPAR call_arg (',' call_arg)* CPAR ;
call_arg : cond_term+ ;

// Owner::name -- either a Table:: context field or an enum value. Which one is
// NOT decidable here (both are `ID::ID`); codegen resolves it by looking the
// owner up in parsed.tables vs parsed.enums, and only a tables hit becomes a
// recorded field dependency.
qualified_ref : owner_id '::' value_id ;

// owner.name -- pass-local instance state (`data.pass_index`). Contributes no
// context dependency, but it still has to parse structurally: falling back to a
// raw backtick for it would mark the whole condition's dependency set
// unprovable and cost the pass its field data.
member_ref : name_id DOT name_id ;

// function_id (e.g. `exists(ShadowMask)`) is matched ahead of value_id because
// value_id's own ID alternative would otherwise win and leave the parentheses
// to be eaten as cond_ops.
// Arithmetic is for [Size] expressions (POINTER is the lexer's '*').
cond_op : AND | OR | NOT | EQ | NEQ | GTEQ | LTEQ | GT | LT | OPAR | CPAR | PLUS | MINUS | POINTER | DIV | MOD ;

flag_value_holder: value_id;

// Escape hatch: a backtick-delimited span is captured verbatim (no grammar
// support for arithmetic/expressions) and pasted into the generated C++ as-is
// -- e.g. [Size = `(builder.graph->get_context<Table::ViewportContext>().
// frame_size + ivec2(1)) / 2`]. The .prism author is responsible for writing a
// fully-qualified, valid C++ expression; Prism does not interpret it.
raw_value: RAWEXPR;

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
 : option_block*? type_id  name_id array? (ASSIGN value_id)? SCOL
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

 pointer: POINTER;

pso_param: option_block*? pso_param_id ASSIGN value_id SCOL;
class_no_template:ID;
type_with_template:class_no_template (LT (template_id)* GT)? pointer?;
inherit_id: ID;
name_id: ID;

option_id: ID;
owner_id: ID;
template_id: ID;
function_id: ID OPAR value_id_ignore? (',' value_id_ignore)* CPAR;
value_id:  shader_type | ID |INT_SCALAR|FLOAT_SCALAR | bool_type | function_id | array_value_ids;
value_id_ignore: ID |INT_SCALAR|FLOAT_SCALAR | bool_type ;

type_id: type_with_template ;

insert_block: INSERT_BLOCK;


// A shader file, relative to workdir/shaders and with its extension:
// "rtx/raytracing.hlsl". The bare-ID form is only for the sentinels that name
// no file: `none` (a per-material hit shader) and `null` (with [Erase]).
shader_path: STRING | ID;

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
 | function_definition
 | insert_block
 | COMMENT

 ;

// An HLSL function member: `[options] ret name(params) : SEMANTIC { body }`.
// The body is one opaque FUNC_BODY token and the parameters are kept as source
// text; Prism only needs the signature to know the function exists. Emitted into
// the struct's generated HLSL by default ([HLSL]).
function_definition
 : option_block*? type_id name_id OPAR function_params CPAR function_semantic? FUNC_BODY
 ;

function_params
 : ( OPAR function_params CPAR | ~( OPAR | CPAR ) )*
 ;

function_semantic
 : COLON ID
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

shader: option_block*? shader_type ASSIGN shader_path SCOL;

compute_pso_stat
 : root_sig
 | shader
 | define_declaration
 | COMMENT
 ;
compute_pso_block: compute_pso_stat*;
compute_pso_definition: option_block*? COMPUTE_PSO name_id inherit? OBRACE compute_pso_block CBRACE;


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


node_param_id:
 'launch'
 | 'entry'
 | 'num_threads'
 | 'max_dispatch_grid'
 | 'input'
 ;

node_param: node_param_id ASSIGN value_id SCOL;

node_output_decl: option_block*? NODE_OUTPUT type_id name_id SCOL;

node_stat
 : node_param
 | node_output_decl
 | COMMENT
 ;

node_block: node_stat*;

node_definition: NODE name_id OBRACE node_block CBRACE;

workgraph_pso_stat
 : root_sig
 | shader
 | define_declaration
 | node_definition
 | COMMENT
 ;
workgraph_pso_block: workgraph_pso_stat*;
workgraph_pso_definition: option_block*? WORKGRAPH_PSO name_id inherit? OBRACE workgraph_pso_block CBRACE;



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



view_declaration
 :  option_block*? type_id name_id SCOL
 ;



view_stat
 : view_declaration
 | COMMENT
 ;
 view_block: view_stat*;
 view_definition: option_block*? VIEW name_id inherit? OBRACE view_block CBRACE;
 pass_definition: option_block*? PASS name_id inherit? OBRACE view_block CBRACE;

pipeline_stat : option_block* name_id SCOL | COMMENT ;
pipeline_block : pipeline_stat* ;
pipeline_definition : PIPELINE name_id OBRACE pipeline_block CBRACE ;

enum_value_declaration : name_id (ASSIGN value_id)? SCOL;
enum_stat : enum_value_declaration | COMMENT;
enum_block : enum_stat*;
enum_definition : ENUM name_id OBRACE enum_block CBRACE;


OR : '||';
AND : '&&';
PIPE : '|';
EQ : '==';
NEQ : '!=';
GT : '>';
LT : '<';
GTEQ : '>=';
LTEQ : '<=';
PLUS : '+';
MINUS : '-';

DIV : '/';
MOD : '%';
POW : '^';
NOT : '!';

SCOL : ';';
COLON : ':';
// Longest-match lexing keeps FLOAT_SCALAR ('1.5', '.5') intact -- DOT only ever
// wins for a '.' that isn't part of a number, which is exactly member_ref.
DOT : '.';
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
WORKGRAPH_PSO: 'WorkgraphPSO';
NODE: 'Node';
NODE_OUTPUT: 'NodeOutput';
RAYTRACE_RAYGEN: 'RaytraceRaygen';
RAYTRACE_PASS: 'RaytracePass';

PASS: 'PassNode';
VIEW: 'PassView';
PIPELINE: 'Pipeline';

SLOT: 'slot';
RT: 'rt';
RTV: 'RTV';
DSV: 'DSV';
ROOTSIG: 'root';
ENUM: 'enum';
shader_type:
'compute'
|'vertex'
|'pixel'
|'domain'
|'hull'
|'geometry'
|'miss'
|'closest_hit'
|'any_hit'
|'raygen'
|'amplification'
|'mesh'
|'shader'
;
pso_param_id:
'ds'
| 'cull'
| 'depth_func'
| 'depth_write'
| 'conservative'
| 'depth_bias'
| 'depth_bias_clamp'
| 'slope_scaled_depth_bias'
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

RAWEXPR
 : '`' ~[`]* '`'
 ;

COMMENT
 : '#' ~[\r\n]* -> skip
 ;

SPACE
 : [ \t\r\n] -> skip
 ;
 
 POINTER
 :
 '*'
 ;

// The predicate sits after the '{', not at the rule's left edge: a left-edge
// predicate takes part in every token's start decision, which stops the lexer
// caching DFA states and made lexing ~30x slower (0.03s -> 0.95s per full
// revalidation). Here it only affects the decision at a '{'.
FUNC_BODY
 : '{' {at_function_body()}? FUNC_BLOCK_TAIL
 ;

// Balanced braces, skipping braces inside HLSL comments and string literals.
fragment FUNC_BLOCK
 : '{' FUNC_BLOCK_TAIL
 ;

fragment FUNC_BLOCK_TAIL
 : ( FUNC_BLOCK | '//' ~[\r\n]* | '/*' .*? '*/' | '"' ( ~["\\\r\n] | '\\' . )* '"' | ~[{}/"] | '/' )* '}'
 ;

INSERT_START: '%{';
INSERT_END: '}%';
INSERT_BLOCK 
    :   INSERT_START (.)*? INSERT_END
    ;