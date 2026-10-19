#pragma once

// The Prism grammar, in cpp-peglib syntax. Lower-case rules become AST nodes
// that Parsing.cpp's TreeShapeListener reacts to; UPPER_CASE rules are tokens
// (leaf nodes whose text is the source text, without the trailing whitespace).
//
// Every token is wrapped in < > so whitespace and `#` comments are skipped after
// it, and every keyword ends in !IDCHAR so `struct` does not match the front of
// `structure`. Keywords are contextual: a word is only a keyword where a rule
// asks for it, so `input` or `mesh` are ordinary names elsewhere. The one place
// that relies on ordering for this is value_id (bool/shader words before ID) and
// rt_stat (the DSV declaration before the colour one).
inline constexpr const char* prism_grammar_a = R"PEG(
prism_file <- definition*

definition
  <- layout_definition / table_definition / rt_definition / workgraph_pso_definition
   / compute_pso_definition / graphics_pso_definition / rtx_pso_definition
   / rtx_pass_definition / rtx_raygen_definition / pass_definition / view_definition
   / pipeline_definition / enum_definition / const_definition / namespace_definition

# `[options] namespace Name { definitions }`: groups declarations without renaming
# them. The header is its own rule so the listener can drop the Namespace element
# before the body's declarations, which attach to the Parsed at the top of its stack.
namespace_definition <- namespace_header OBRACE definition* CBRACE
namespace_header     <- option_block* NAMESPACE name_id

# A top-level named constant: `const Name = value;`.
const_definition <- CONST name_id options_assign SCOL

bind_option
  <- (owner_id DCOLON)? flag_value_holder (PIPE flag_value_holder)+
   / raw_value
   / cond_expr

# An option value captured as an ORDERED TERM SEQUENCE rather than a
# precedence-parsed tree. Codegen never evaluates these -- it only renders the
# terms back to C++ in source order and reports which Table:: context fields
# were named -- so emitting the author's own tokens verbatim reproduces both
# their parenthesisation and C++'s precedence exactly.
cond_expr <- cond_term+

cond_term
  <- qualified_ref / function_id / call / member_ref / value_id / cond_op

# A call whose arguments are themselves expressions:
# `area(tiles(ViewportContext::frame_size, 16))`. A PEG has no lookahead into what
# follows the call, so the argument must itself keep its parentheses balanced:
# a ')' that closes the call is never read as an operator.
call        <- ID OPAR call_arg (COMMA call_arg)* CPAR
call_arg    <- (paren_group / !CPAR cond_term)+
paren_group <- OPAR (paren_group / !CPAR cond_term)* CPAR

# Owner::name -- either a Table:: context field or an enum value (codegen decides).
qualified_ref <- owner_id DCOLON value_id

# owner.name -- pass-local instance state (`data.pass_index`).
member_ref <- name_id DOT name_id

# Arithmetic is for [Size] expressions (POINTER is the '*').
cond_op
  <- OR / AND / EQ / NEQ / GTEQ / LTEQ / GT / LT / NOT / OPAR / CPAR
   / PLUS / MINUS / POINTER / DIV / MOD

flag_value_holder <- value_id

# Escape hatch: a backtick-delimited span is captured verbatim and pasted into
# the generated C++ as-is.
raw_value <- RAWEXPR

options_assign <- ASSIGN bind_option

option       <- name_id options_assign?
option_block <- OSBRACE option (COMMA option)* CSBRACE

array_count_id <- INT_SCALAR
array          <- OSBRACE array_count_id? CSBRACE

value_declaration <- option_block* type_id name_id array? (ASSIGN value_id)? SCOL
slot_declaration  <- SLOT name_id SCOL
sampler_declaration <- SAMPLER name_id ASSIGN value_id SCOL
define_declaration  <- option_block* DEFINE name_id (ASSIGN array_value_ids)? SCOL
rtv_formats_declaration <- option_block* RTV_FORMATS ASSIGN array_value_ids SCOL
blends_declaration      <- option_block* BLEND ASSIGN array_value_ids SCOL

pointer <- POINTER
pso_param <- option_block* pso_param_id ASSIGN value_id SCOL
class_no_template <- ID
type_with_template <- class_no_template (LT template_id* GT)? pointer?
inherit_id <- ID
name_id    <- ID
option_id  <- ID
owner_id   <- ID
template_id <- ID
function_id <- ID OPAR value_id_ignore? (COMMA value_id_ignore)* CPAR

# Words that are also keywords come first so they are not read as a plain ID.
value_id
  <- shader_type / bool_type / FLOAT_SCALAR / INT_SCALAR / function_id / array_value_ids / ID
value_id_ignore <- bool_type / FLOAT_SCALAR / INT_SCALAR / ID

type_id      <- type_with_template
insert_block <- INSERT_BLOCK
shader_path  <- STRING / ID

inherit <- COLON inherit_id (COMMA inherit_id)*

layout_stat <- slot_declaration / sampler_declaration
layout_block <- layout_stat*
layout_definition <- LAYOUT name_id inherit? OBRACE layout_block CBRACE

table_stat <- value_declaration / function_definition / insert_block

# FUNC_BODY follows the signature directly, which is what tells it from the
# OBRACE of a struct or PSO body.
function_definition <- option_block* type_id name_id OPAR function_params CPAR function_semantic? FUNC_BODY
function_params     <- < params_text >
params_text         <- ('(' params_text ')' / '#' [^\r\n]* / [^()])*
function_semantic   <- COLON ID

table_block      <- table_stat*
table_definition <- option_block* STRUCT name_id inherit? OBRACE table_block CBRACE

rt_color_declaration <- type_id name_id SCOL
rt_ds_declaration    <- DSV name_id SCOL
rt_stat  <- rt_ds_declaration / rt_color_declaration
rt_block <- rt_stat*
rt_definition <- RT name_id OBRACE rt_block CBRACE

array_value_holder <- value_id
array_value_ids    <- OBRACE array_value_holder (COMMA array_value_holder)* CBRACE

root_sig <- ROOTSIG ASSIGN name_id SCOL
shader   <- option_block* shader_type ASSIGN shader_path SCOL

compute_pso_stat  <- root_sig / shader / define_declaration
compute_pso_block <- compute_pso_stat*
compute_pso_definition <- option_block* COMPUTE_PSO name_id inherit? OBRACE compute_pso_block CBRACE

graphics_pso_stat
  <- root_sig / shader / define_declaration / rtv_formats_declaration / blends_declaration / pso_param
graphics_pso_block <- graphics_pso_stat*
graphics_pso_definition <- option_block* GRAPHICS_PSO name_id inherit? OBRACE graphics_pso_block CBRACE

rtx_pso_stat  <- root_sig
rtx_pso_block <- rtx_pso_stat*
rtx_pso_definition <- RAYTRACE_PSO name_id inherit? OBRACE rtx_pso_block CBRACE

node_param    <- node_param_id ASSIGN value_id SCOL
node_output_decl <- option_block* NODE_OUTPUT type_id name_id SCOL
node_stat  <- node_param / node_output_decl
node_block <- node_stat*
node_definition <- NODE name_id OBRACE node_block CBRACE

workgraph_pso_stat  <- root_sig / shader / define_declaration / node_definition
workgraph_pso_block <- workgraph_pso_stat*
workgraph_pso_definition <- option_block* WORKGRAPH_PSO name_id inherit? OBRACE workgraph_pso_block CBRACE

rtx_pass_stat  <- shader / pso_param
rtx_pass_block <- rtx_pass_stat*
rtx_pass_definition <- option_block* RAYTRACE_PASS name_id inherit? OBRACE rtx_pass_block CBRACE

rtx_raygen_stat  <- shader
rtx_raygen_block <- rtx_raygen_stat*
rtx_raygen_definition <- option_block* RAYTRACE_RAYGEN name_id inherit? OBRACE rtx_raygen_block CBRACE

view_declaration <- option_block* type_id name_id SCOL
view_stat  <- view_declaration
view_block <- view_stat*
view_definition <- option_block* VIEW name_id inherit? OBRACE view_block CBRACE
pass_definition <- option_block* PASS name_id inherit? OBRACE view_block CBRACE

pipeline_stat  <- option_block* name_id SCOL
pipeline_block <- pipeline_stat*
pipeline_definition <- PIPELINE name_id OBRACE pipeline_block CBRACE

enum_value_declaration <- name_id (ASSIGN value_id)? SCOL
enum_stat  <- enum_value_declaration
enum_block <- enum_stat*
enum_definition <- ENUM name_id OBRACE enum_block CBRACE
)PEG";

inline constexpr const char* prism_grammar_b = R"PEG(
bool_type <- TRUE / FALSE

shader_type
  <- < ('compute' / 'vertex' / 'pixel' / 'domain' / 'hull' / 'geometry' / 'miss' / 'closest_hit'
      / 'any_hit' / 'raygen' / 'amplification' / 'mesh' / 'shader') !IDCHAR >

pso_param_id
  <- < ('ds' / 'cull' / 'depth_func' / 'depth_write' / 'conservative' / 'depth_bias_clamp' / 'depth_bias'
      / 'slope_scaled_depth_bias' / 'enable_depth' / 'topology' / 'enable_stencil' / 'stencil_func'
      / 'stencil_pass_op' / 'stencil_read_mask' / 'stencil_write_mask' / 'recursion_depth' / 'payload'
      / 'per_material' / 'local') !IDCHAR >

node_param_id <- < ('launch' / 'entry' / 'num_threads' / 'max_dispatch_grid' / 'input') !IDCHAR >

IDCHAR <- [a-zA-Z_0-9]
ID     <- < [a-zA-Z_] IDCHAR* >

INT_SCALAR   <- < [0-9]+ >
FLOAT_SCALAR <- < [0-9]+ '.' [0-9]* / '.' [0-9]+ >
STRING       <- < '"' ([^"\r\n] / '""')* '"' >
RAWEXPR      <- < '`' [^`]* '`' >

# Balanced braces, skipping braces inside HLSL comments and string literals.
FUNC_BODY <- < '{' FUNC_TAIL >
FUNC_TAIL <- ('{' FUNC_TAIL / '//' [^\r\n]* / '/*' (!'*/' .)* '*/'
             / '"' ([^"\\\r\n] / '\\' .)* '"' / [^{}/"] / '/')* '}'

INSERT_BLOCK <- < '%{' (!'}%' .)* '}%' >

OR     <- < '||' >
AND    <- < '&&' >
PIPE   <- < '|' !'|' >
EQ     <- < '==' >
NEQ    <- < '!=' >
GTEQ   <- < '>=' >
LTEQ   <- < '<=' >
GT     <- < '>' >
LT     <- < '<' >
PLUS   <- < '+' >
MINUS  <- < '-' >
DIV    <- < '/' >
MOD    <- < '%' >
NOT    <- < '!' !'=' >
POINTER <- < '*' >

SCOL   <- < ';' >
COMMA  <- < ',' >
DCOLON <- < '::' >
COLON  <- < ':' !':' >
DOT    <- < '.' >
ASSIGN <- < '=' !'=' >
OPAR   <- < '(' >
CPAR   <- < ')' >
OBRACE <- < '{' >
CBRACE <- < '}' >
OSBRACE <- < '[' >
CSBRACE <- < ']' >

TRUE  <- < 'true' !IDCHAR >
FALSE <- < 'false' !IDCHAR >

LAYOUT        <- < 'layout' !IDCHAR >
STRUCT        <- < 'struct' !IDCHAR >
NAMESPACE     <- < 'namespace' !IDCHAR >
CONST         <- < 'const' !IDCHAR >
SAMPLER       <- < 'Sampler' !IDCHAR >
DEFINE        <- < 'define' !IDCHAR >
RTV_FORMATS   <- < 'rtv' !IDCHAR >
BLEND         <- < 'blend' !IDCHAR >
COMPUTE_PSO   <- < 'ComputePSO' !IDCHAR >
GRAPHICS_PSO  <- < 'GraphicsPSO' !IDCHAR >
RAYTRACE_PSO  <- < 'RaytracePSO' !IDCHAR >
WORKGRAPH_PSO <- < 'WorkgraphPSO' !IDCHAR >
NODE          <- < 'Node' !IDCHAR >
NODE_OUTPUT   <- < 'NodeOutput' !IDCHAR >
RAYTRACE_RAYGEN <- < 'RaytraceRaygen' !IDCHAR >
RAYTRACE_PASS <- < 'RaytracePass' !IDCHAR >
PASS          <- < 'PassNode' !IDCHAR >
VIEW          <- < 'PassView' !IDCHAR >
PIPELINE      <- < 'Pipeline' !IDCHAR >
SLOT          <- < 'slot' !IDCHAR >
RT            <- < 'rt' !IDCHAR >
DSV           <- < 'DSV' !IDCHAR >
ROOTSIG       <- < 'root' !IDCHAR >
ENUM          <- < 'enum' !IDCHAR >

%whitespace <- ([ \t\r\n] / '#' [^\r\n]*)*
)PEG";
