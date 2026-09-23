
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1


#include "PrismListener.h"
#include "PrismVisitor.h"

#include "PrismParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct PrismParserStaticData final {
  PrismParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PrismParserStaticData(const PrismParserStaticData&) = delete;
  PrismParserStaticData(PrismParserStaticData&&) = delete;
  PrismParserStaticData& operator=(const PrismParserStaticData&) = delete;
  PrismParserStaticData& operator=(PrismParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag prismParserOnceFlag;
PrismParserStaticData *prismParserStaticData = nullptr;

void prismParserInitialize() {
  assert(prismParserStaticData == nullptr);
  auto staticData = std::make_unique<PrismParserStaticData>(
    std::vector<std::string>{
      "parse", "const_definition", "bind_option", "cond_expr", "cond_term", 
      "call", "call_arg", "qualified_ref", "member_ref", "cond_op", "flag_value_holder", 
      "raw_value", "options_assign", "option", "option_block", "array_count_id", 
      "array", "value_declaration", "slot_declaration", "sampler_declaration", 
      "define_declaration", "rtv_formats_declaration", "blends_declaration", 
      "pointer", "pso_param", "class_no_template", "type_with_template", 
      "inherit_id", "name_id", "option_id", "owner_id", "template_id", "function_id", 
      "value_id", "value_id_ignore", "type_id", "insert_block", "shader_path", 
      "inherit", "layout_stat", "layout_block", "layout_definition", "table_stat", 
      "function_definition", "function_params", "function_semantic", "table_block", 
      "table_definition", "rt_color_declaration", "rt_ds_declaration", "rt_stat", 
      "rt_block", "rt_definition", "array_value_holder", "array_value_ids", 
      "root_sig", "shader", "compute_pso_stat", "compute_pso_block", "compute_pso_definition", 
      "graphics_pso_stat", "graphics_pso_block", "graphics_pso_definition", 
      "rtx_pso_stat", "rtx_pso_block", "rtx_pso_definition", "node_param_id", 
      "node_param", "node_output_decl", "node_stat", "node_block", "node_definition", 
      "workgraph_pso_stat", "workgraph_pso_block", "workgraph_pso_definition", 
      "rtx_pass_stat", "rtx_pass_block", "rtx_pass_definition", "rtx_raygen_stat", 
      "rtx_raygen_block", "rtx_raygen_definition", "view_declaration", "view_stat", 
      "view_block", "view_definition", "pass_definition", "pipeline_stat", 
      "pipeline_block", "pipeline_definition", "enum_value_declaration", 
      "enum_stat", "enum_block", "enum_definition", "shader_type", "pso_param_id", 
      "bool_type"
    },
    std::vector<std::string>{
      "", "'const'", "'::'", "','", "'Sampler'", "'define'", "'rtv'", "'blend'", 
      "'launch'", "'entry'", "'num_threads'", "'max_dispatch_grid'", "'input'", 
      "'compute'", "'vertex'", "'pixel'", "'domain'", "'hull'", "'geometry'", 
      "'miss'", "'closest_hit'", "'any_hit'", "'raygen'", "'amplification'", 
      "'mesh'", "'shader'", "'ds'", "'cull'", "'depth_func'", "'depth_write'", 
      "'conservative'", "'depth_bias'", "'depth_bias_clamp'", "'slope_scaled_depth_bias'", 
      "'enable_depth'", "'topology'", "'enable_stencil'", "'stencil_func'", 
      "'stencil_pass_op'", "'stencil_read_mask'", "'stencil_write_mask'", 
      "'recursion_depth'", "'payload'", "'per_material'", "'local'", "'||'", 
      "'&&'", "'|'", "'=='", "'!='", "'>'", "'<'", "'>='", "'<='", "'+'", 
      "'-'", "'/'", "'%'", "'^'", "'!'", "';'", "':'", "'.'", "'='", "'('", 
      "')'", "'{'", "'}'", "'['", "']'", "'true'", "'false'", "'log'", "'layout'", 
      "'struct'", "'ComputePSO'", "'GraphicsPSO'", "'RaytracePSO'", "'WorkgraphPSO'", 
      "'Node'", "'NodeOutput'", "'RaytraceRaygen'", "'RaytracePass'", "'PassNode'", 
      "'PassView'", "'Pipeline'", "'slot'", "'rt'", "'RTV'", "'DSV'", "'root'", 
      "'enum'", "", "", "", "", "", "", "", "'*'", "", "'%{'", "'}%'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "OR", "AND", "PIPE", "EQ", 
      "NEQ", "GT", "LT", "GTEQ", "LTEQ", "PLUS", "MINUS", "DIV", "MOD", 
      "POW", "NOT", "SCOL", "COLON", "DOT", "ASSIGN", "OPAR", "CPAR", "OBRACE", 
      "CBRACE", "OSBRACE", "CSBRACE", "TRUE", "FALSE", "LOG", "LAYOUT", 
      "STRUCT", "COMPUTE_PSO", "GRAPHICS_PSO", "RAYTRACE_PSO", "WORKGRAPH_PSO", 
      "NODE", "NODE_OUTPUT", "RAYTRACE_RAYGEN", "RAYTRACE_PASS", "PASS", 
      "VIEW", "PIPELINE", "SLOT", "RT", "RTV", "DSV", "ROOTSIG", "ENUM", 
      "ID", "INT_SCALAR", "FLOAT_SCALAR", "STRING", "RAWEXPR", "COMMENT", 
      "SPACE", "POINTER", "FUNC_BODY", "INSERT_START", "INSERT_END", "INSERT_BLOCK"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,103,875,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,7,
  	63,2,64,7,64,2,65,7,65,2,66,7,66,2,67,7,67,2,68,7,68,2,69,7,69,2,70,7,
  	70,2,71,7,71,2,72,7,72,2,73,7,73,2,74,7,74,2,75,7,75,2,76,7,76,2,77,7,
  	77,2,78,7,78,2,79,7,79,2,80,7,80,2,81,7,81,2,82,7,82,2,83,7,83,2,84,7,
  	84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,2,89,7,89,2,90,7,90,2,91,7,
  	91,2,92,7,92,2,93,7,93,2,94,7,94,2,95,7,95,1,0,1,0,1,0,1,0,1,0,1,0,1,
  	0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,5,0,208,8,0,10,0,12,0,211,9,0,1,0,1,
  	0,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,2,3,2,223,8,2,1,2,1,2,1,2,4,2,228,8,2,
  	11,2,12,2,229,1,2,1,2,3,2,234,8,2,1,3,4,3,237,8,3,11,3,12,3,238,1,4,1,
  	4,1,4,1,4,1,4,1,4,3,4,247,8,4,1,5,1,5,1,5,1,5,1,5,5,5,254,8,5,10,5,12,
  	5,257,9,5,1,5,1,5,1,6,4,6,262,8,6,11,6,12,6,263,1,7,1,7,1,7,1,7,1,8,1,
  	8,1,8,1,8,1,9,1,9,1,10,1,10,1,11,1,11,1,12,1,12,1,12,1,13,1,13,3,13,285,
  	8,13,1,14,1,14,1,14,1,14,5,14,291,8,14,10,14,12,14,294,9,14,1,14,1,14,
  	1,15,1,15,1,16,1,16,3,16,302,8,16,1,16,1,16,1,17,5,17,307,8,17,10,17,
  	12,17,310,9,17,1,17,1,17,1,17,3,17,315,8,17,1,17,1,17,3,17,319,8,17,1,
  	17,1,17,1,18,1,18,1,18,1,18,1,19,1,19,1,19,1,19,1,19,1,19,1,20,5,20,334,
  	8,20,10,20,12,20,337,9,20,1,20,1,20,1,20,1,20,3,20,343,8,20,1,20,1,20,
  	1,21,5,21,348,8,21,10,21,12,21,351,9,21,1,21,1,21,1,21,1,21,1,21,1,22,
  	5,22,359,8,22,10,22,12,22,362,9,22,1,22,1,22,1,22,1,22,1,22,1,23,1,23,
  	1,24,5,24,372,8,24,10,24,12,24,375,9,24,1,24,1,24,1,24,1,24,1,24,1,25,
  	1,25,1,26,1,26,1,26,5,26,387,8,26,10,26,12,26,390,9,26,1,26,3,26,393,
  	8,26,1,26,3,26,396,8,26,1,27,1,27,1,28,1,28,1,29,1,29,1,30,1,30,1,31,
  	1,31,1,32,1,32,1,32,3,32,411,8,32,1,32,1,32,5,32,415,8,32,10,32,12,32,
  	418,9,32,1,32,1,32,1,33,1,33,1,33,1,33,1,33,1,33,1,33,3,33,429,8,33,1,
  	34,1,34,1,34,1,34,3,34,435,8,34,1,35,1,35,1,36,1,36,1,37,1,37,1,38,1,
  	38,1,38,1,38,5,38,447,8,38,10,38,12,38,450,9,38,1,39,1,39,1,39,3,39,455,
  	8,39,1,40,5,40,458,8,40,10,40,12,40,461,9,40,1,41,1,41,1,41,3,41,466,
  	8,41,1,41,1,41,1,41,1,41,1,42,1,42,1,42,1,42,3,42,476,8,42,1,43,5,43,
  	479,8,43,10,43,12,43,482,9,43,1,43,1,43,1,43,1,43,1,43,1,43,3,43,490,
  	8,43,1,43,1,43,1,44,1,44,1,44,1,44,1,44,5,44,499,8,44,10,44,12,44,502,
  	9,44,1,45,1,45,1,45,1,46,5,46,508,8,46,10,46,12,46,511,9,46,1,47,5,47,
  	514,8,47,10,47,12,47,517,9,47,1,47,1,47,1,47,3,47,522,8,47,1,47,1,47,
  	1,47,1,47,1,48,1,48,1,48,1,48,1,49,1,49,1,49,1,49,1,50,1,50,1,50,3,50,
  	539,8,50,1,51,5,51,542,8,51,10,51,12,51,545,9,51,1,52,1,52,1,52,1,52,
  	1,52,1,52,1,53,1,53,1,54,1,54,1,54,1,54,5,54,559,8,54,10,54,12,54,562,
  	9,54,1,54,1,54,1,55,1,55,1,55,1,55,1,55,1,56,5,56,572,8,56,10,56,12,56,
  	575,9,56,1,56,1,56,1,56,1,56,1,56,1,57,1,57,1,57,1,57,3,57,586,8,57,1,
  	58,5,58,589,8,58,10,58,12,58,592,9,58,1,59,5,59,595,8,59,10,59,12,59,
  	598,9,59,1,59,1,59,1,59,3,59,603,8,59,1,59,1,59,1,59,1,59,1,60,1,60,1,
  	60,1,60,1,60,1,60,1,60,3,60,616,8,60,1,61,5,61,619,8,61,10,61,12,61,622,
  	9,61,1,62,5,62,625,8,62,10,62,12,62,628,9,62,1,62,1,62,1,62,3,62,633,
  	8,62,1,62,1,62,1,62,1,62,1,63,1,63,3,63,641,8,63,1,64,5,64,644,8,64,10,
  	64,12,64,647,9,64,1,65,1,65,1,65,3,65,652,8,65,1,65,1,65,1,65,1,65,1,
  	66,1,66,1,67,1,67,1,67,1,67,1,67,1,68,5,68,666,8,68,10,68,12,68,669,9,
  	68,1,68,1,68,1,68,1,68,1,68,1,69,1,69,1,69,3,69,679,8,69,1,70,5,70,682,
  	8,70,10,70,12,70,685,9,70,1,71,1,71,1,71,1,71,1,71,1,71,1,72,1,72,1,72,
  	1,72,1,72,3,72,698,8,72,1,73,5,73,701,8,73,10,73,12,73,704,9,73,1,74,
  	5,74,707,8,74,10,74,12,74,710,9,74,1,74,1,74,1,74,3,74,715,8,74,1,74,
  	1,74,1,74,1,74,1,75,1,75,1,75,3,75,724,8,75,1,76,5,76,727,8,76,10,76,
  	12,76,730,9,76,1,77,5,77,733,8,77,10,77,12,77,736,9,77,1,77,1,77,1,77,
  	3,77,741,8,77,1,77,1,77,1,77,1,77,1,78,1,78,3,78,749,8,78,1,79,5,79,752,
  	8,79,10,79,12,79,755,9,79,1,80,5,80,758,8,80,10,80,12,80,761,9,80,1,80,
  	1,80,1,80,3,80,766,8,80,1,80,1,80,1,80,1,80,1,81,5,81,773,8,81,10,81,
  	12,81,776,9,81,1,81,1,81,1,81,1,81,1,82,1,82,3,82,784,8,82,1,83,5,83,
  	787,8,83,10,83,12,83,790,9,83,1,84,5,84,793,8,84,10,84,12,84,796,9,84,
  	1,84,1,84,1,84,3,84,801,8,84,1,84,1,84,1,84,1,84,1,85,5,85,808,8,85,10,
  	85,12,85,811,9,85,1,85,1,85,1,85,3,85,816,8,85,1,85,1,85,1,85,1,85,1,
  	86,5,86,823,8,86,10,86,12,86,826,9,86,1,86,1,86,1,86,1,86,3,86,832,8,
  	86,1,87,5,87,835,8,87,10,87,12,87,838,9,87,1,88,1,88,1,88,1,88,1,88,1,
  	88,1,89,1,89,1,89,3,89,849,8,89,1,89,1,89,1,90,1,90,3,90,855,8,90,1,91,
  	5,91,858,8,91,10,91,12,91,861,9,91,1,92,1,92,1,92,1,92,1,92,1,92,1,93,
  	1,93,1,94,1,94,1,95,1,95,1,95,18,308,335,349,360,373,448,480,515,573,
  	596,626,667,708,734,759,774,794,809,0,96,0,2,4,6,8,10,12,14,16,18,20,
  	22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,
  	68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,
  	112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,
  	148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,
  	184,186,188,190,0,7,5,0,45,46,48,57,59,59,64,65,99,99,2,0,92,92,95,95,
  	1,0,64,65,1,0,8,12,1,0,13,25,1,0,26,44,1,0,70,71,901,0,209,1,0,0,0,2,
  	214,1,0,0,0,4,233,1,0,0,0,6,236,1,0,0,0,8,246,1,0,0,0,10,248,1,0,0,0,
  	12,261,1,0,0,0,14,265,1,0,0,0,16,269,1,0,0,0,18,273,1,0,0,0,20,275,1,
  	0,0,0,22,277,1,0,0,0,24,279,1,0,0,0,26,282,1,0,0,0,28,286,1,0,0,0,30,
  	297,1,0,0,0,32,299,1,0,0,0,34,308,1,0,0,0,36,322,1,0,0,0,38,326,1,0,0,
  	0,40,335,1,0,0,0,42,349,1,0,0,0,44,360,1,0,0,0,46,368,1,0,0,0,48,373,
  	1,0,0,0,50,381,1,0,0,0,52,383,1,0,0,0,54,397,1,0,0,0,56,399,1,0,0,0,58,
  	401,1,0,0,0,60,403,1,0,0,0,62,405,1,0,0,0,64,407,1,0,0,0,66,428,1,0,0,
  	0,68,434,1,0,0,0,70,436,1,0,0,0,72,438,1,0,0,0,74,440,1,0,0,0,76,442,
  	1,0,0,0,78,454,1,0,0,0,80,459,1,0,0,0,82,462,1,0,0,0,84,475,1,0,0,0,86,
  	480,1,0,0,0,88,500,1,0,0,0,90,503,1,0,0,0,92,509,1,0,0,0,94,515,1,0,0,
  	0,96,527,1,0,0,0,98,531,1,0,0,0,100,538,1,0,0,0,102,543,1,0,0,0,104,546,
  	1,0,0,0,106,552,1,0,0,0,108,554,1,0,0,0,110,565,1,0,0,0,112,573,1,0,0,
  	0,114,585,1,0,0,0,116,590,1,0,0,0,118,596,1,0,0,0,120,615,1,0,0,0,122,
  	620,1,0,0,0,124,626,1,0,0,0,126,640,1,0,0,0,128,645,1,0,0,0,130,648,1,
  	0,0,0,132,657,1,0,0,0,134,659,1,0,0,0,136,667,1,0,0,0,138,678,1,0,0,0,
  	140,683,1,0,0,0,142,686,1,0,0,0,144,697,1,0,0,0,146,702,1,0,0,0,148,708,
  	1,0,0,0,150,723,1,0,0,0,152,728,1,0,0,0,154,734,1,0,0,0,156,748,1,0,0,
  	0,158,753,1,0,0,0,160,759,1,0,0,0,162,774,1,0,0,0,164,783,1,0,0,0,166,
  	788,1,0,0,0,168,794,1,0,0,0,170,809,1,0,0,0,172,831,1,0,0,0,174,836,1,
  	0,0,0,176,839,1,0,0,0,178,845,1,0,0,0,180,854,1,0,0,0,182,859,1,0,0,0,
  	184,862,1,0,0,0,186,868,1,0,0,0,188,870,1,0,0,0,190,872,1,0,0,0,192,208,
  	3,82,41,0,193,208,3,94,47,0,194,208,3,104,52,0,195,208,3,148,74,0,196,
  	208,3,118,59,0,197,208,3,124,62,0,198,208,3,130,65,0,199,208,3,154,77,
  	0,200,208,3,160,80,0,201,208,3,170,85,0,202,208,3,168,84,0,203,208,3,
  	176,88,0,204,208,3,184,92,0,205,208,3,2,1,0,206,208,5,97,0,0,207,192,
  	1,0,0,0,207,193,1,0,0,0,207,194,1,0,0,0,207,195,1,0,0,0,207,196,1,0,0,
  	0,207,197,1,0,0,0,207,198,1,0,0,0,207,199,1,0,0,0,207,200,1,0,0,0,207,
  	201,1,0,0,0,207,202,1,0,0,0,207,203,1,0,0,0,207,204,1,0,0,0,207,205,1,
  	0,0,0,207,206,1,0,0,0,208,211,1,0,0,0,209,207,1,0,0,0,209,210,1,0,0,0,
  	210,212,1,0,0,0,211,209,1,0,0,0,212,213,5,0,0,1,213,1,1,0,0,0,214,215,
  	5,1,0,0,215,216,3,56,28,0,216,217,3,24,12,0,217,218,5,60,0,0,218,3,1,
  	0,0,0,219,220,3,60,30,0,220,221,5,2,0,0,221,223,1,0,0,0,222,219,1,0,0,
  	0,222,223,1,0,0,0,223,224,1,0,0,0,224,227,3,20,10,0,225,226,5,47,0,0,
  	226,228,3,20,10,0,227,225,1,0,0,0,228,229,1,0,0,0,229,227,1,0,0,0,229,
  	230,1,0,0,0,230,234,1,0,0,0,231,234,3,22,11,0,232,234,3,6,3,0,233,222,
  	1,0,0,0,233,231,1,0,0,0,233,232,1,0,0,0,234,5,1,0,0,0,235,237,3,8,4,0,
  	236,235,1,0,0,0,237,238,1,0,0,0,238,236,1,0,0,0,238,239,1,0,0,0,239,7,
  	1,0,0,0,240,247,3,14,7,0,241,247,3,64,32,0,242,247,3,10,5,0,243,247,3,
  	16,8,0,244,247,3,66,33,0,245,247,3,18,9,0,246,240,1,0,0,0,246,241,1,0,
  	0,0,246,242,1,0,0,0,246,243,1,0,0,0,246,244,1,0,0,0,246,245,1,0,0,0,247,
  	9,1,0,0,0,248,249,5,92,0,0,249,250,5,64,0,0,250,255,3,12,6,0,251,252,
  	5,3,0,0,252,254,3,12,6,0,253,251,1,0,0,0,254,257,1,0,0,0,255,253,1,0,
  	0,0,255,256,1,0,0,0,256,258,1,0,0,0,257,255,1,0,0,0,258,259,5,65,0,0,
  	259,11,1,0,0,0,260,262,3,8,4,0,261,260,1,0,0,0,262,263,1,0,0,0,263,261,
  	1,0,0,0,263,264,1,0,0,0,264,13,1,0,0,0,265,266,3,60,30,0,266,267,5,2,
  	0,0,267,268,3,66,33,0,268,15,1,0,0,0,269,270,3,56,28,0,270,271,5,62,0,
  	0,271,272,3,56,28,0,272,17,1,0,0,0,273,274,7,0,0,0,274,19,1,0,0,0,275,
  	276,3,66,33,0,276,21,1,0,0,0,277,278,5,96,0,0,278,23,1,0,0,0,279,280,
  	5,63,0,0,280,281,3,4,2,0,281,25,1,0,0,0,282,284,3,56,28,0,283,285,3,24,
  	12,0,284,283,1,0,0,0,284,285,1,0,0,0,285,27,1,0,0,0,286,287,5,68,0,0,
  	287,292,3,26,13,0,288,289,5,3,0,0,289,291,3,26,13,0,290,288,1,0,0,0,291,
  	294,1,0,0,0,292,290,1,0,0,0,292,293,1,0,0,0,293,295,1,0,0,0,294,292,1,
  	0,0,0,295,296,5,69,0,0,296,29,1,0,0,0,297,298,5,93,0,0,298,31,1,0,0,0,
  	299,301,5,68,0,0,300,302,3,30,15,0,301,300,1,0,0,0,301,302,1,0,0,0,302,
  	303,1,0,0,0,303,304,5,69,0,0,304,33,1,0,0,0,305,307,3,28,14,0,306,305,
  	1,0,0,0,307,310,1,0,0,0,308,309,1,0,0,0,308,306,1,0,0,0,309,311,1,0,0,
  	0,310,308,1,0,0,0,311,312,3,70,35,0,312,314,3,56,28,0,313,315,3,32,16,
  	0,314,313,1,0,0,0,314,315,1,0,0,0,315,318,1,0,0,0,316,317,5,63,0,0,317,
  	319,3,66,33,0,318,316,1,0,0,0,318,319,1,0,0,0,319,320,1,0,0,0,320,321,
  	5,60,0,0,321,35,1,0,0,0,322,323,5,86,0,0,323,324,3,56,28,0,324,325,5,
  	60,0,0,325,37,1,0,0,0,326,327,5,4,0,0,327,328,3,56,28,0,328,329,5,63,
  	0,0,329,330,3,66,33,0,330,331,5,60,0,0,331,39,1,0,0,0,332,334,3,28,14,
  	0,333,332,1,0,0,0,334,337,1,0,0,0,335,336,1,0,0,0,335,333,1,0,0,0,336,
  	338,1,0,0,0,337,335,1,0,0,0,338,339,5,5,0,0,339,342,3,56,28,0,340,341,
  	5,63,0,0,341,343,3,108,54,0,342,340,1,0,0,0,342,343,1,0,0,0,343,344,1,
  	0,0,0,344,345,5,60,0,0,345,41,1,0,0,0,346,348,3,28,14,0,347,346,1,0,0,
  	0,348,351,1,0,0,0,349,350,1,0,0,0,349,347,1,0,0,0,350,352,1,0,0,0,351,
  	349,1,0,0,0,352,353,5,6,0,0,353,354,5,63,0,0,354,355,3,108,54,0,355,356,
  	5,60,0,0,356,43,1,0,0,0,357,359,3,28,14,0,358,357,1,0,0,0,359,362,1,0,
  	0,0,360,361,1,0,0,0,360,358,1,0,0,0,361,363,1,0,0,0,362,360,1,0,0,0,363,
  	364,5,7,0,0,364,365,5,63,0,0,365,366,3,108,54,0,366,367,5,60,0,0,367,
  	45,1,0,0,0,368,369,5,99,0,0,369,47,1,0,0,0,370,372,3,28,14,0,371,370,
  	1,0,0,0,372,375,1,0,0,0,373,374,1,0,0,0,373,371,1,0,0,0,374,376,1,0,0,
  	0,375,373,1,0,0,0,376,377,3,188,94,0,377,378,5,63,0,0,378,379,3,66,33,
  	0,379,380,5,60,0,0,380,49,1,0,0,0,381,382,5,92,0,0,382,51,1,0,0,0,383,
  	392,3,50,25,0,384,388,5,51,0,0,385,387,3,62,31,0,386,385,1,0,0,0,387,
  	390,1,0,0,0,388,386,1,0,0,0,388,389,1,0,0,0,389,391,1,0,0,0,390,388,1,
  	0,0,0,391,393,5,50,0,0,392,384,1,0,0,0,392,393,1,0,0,0,393,395,1,0,0,
  	0,394,396,3,46,23,0,395,394,1,0,0,0,395,396,1,0,0,0,396,53,1,0,0,0,397,
  	398,5,92,0,0,398,55,1,0,0,0,399,400,5,92,0,0,400,57,1,0,0,0,401,402,5,
  	92,0,0,402,59,1,0,0,0,403,404,5,92,0,0,404,61,1,0,0,0,405,406,5,92,0,
  	0,406,63,1,0,0,0,407,408,5,92,0,0,408,410,5,64,0,0,409,411,3,68,34,0,
  	410,409,1,0,0,0,410,411,1,0,0,0,411,416,1,0,0,0,412,413,5,3,0,0,413,415,
  	3,68,34,0,414,412,1,0,0,0,415,418,1,0,0,0,416,414,1,0,0,0,416,417,1,0,
  	0,0,417,419,1,0,0,0,418,416,1,0,0,0,419,420,5,65,0,0,420,65,1,0,0,0,421,
  	429,3,186,93,0,422,429,5,92,0,0,423,429,5,93,0,0,424,429,5,94,0,0,425,
  	429,3,190,95,0,426,429,3,64,32,0,427,429,3,108,54,0,428,421,1,0,0,0,428,
  	422,1,0,0,0,428,423,1,0,0,0,428,424,1,0,0,0,428,425,1,0,0,0,428,426,1,
  	0,0,0,428,427,1,0,0,0,429,67,1,0,0,0,430,435,5,92,0,0,431,435,5,93,0,
  	0,432,435,5,94,0,0,433,435,3,190,95,0,434,430,1,0,0,0,434,431,1,0,0,0,
  	434,432,1,0,0,0,434,433,1,0,0,0,435,69,1,0,0,0,436,437,3,52,26,0,437,
  	71,1,0,0,0,438,439,5,103,0,0,439,73,1,0,0,0,440,441,7,1,0,0,441,75,1,
  	0,0,0,442,443,5,61,0,0,443,448,3,54,27,0,444,445,5,3,0,0,445,447,3,54,
  	27,0,446,444,1,0,0,0,447,450,1,0,0,0,448,449,1,0,0,0,448,446,1,0,0,0,
  	449,77,1,0,0,0,450,448,1,0,0,0,451,455,3,36,18,0,452,455,3,38,19,0,453,
  	455,5,97,0,0,454,451,1,0,0,0,454,452,1,0,0,0,454,453,1,0,0,0,455,79,1,
  	0,0,0,456,458,3,78,39,0,457,456,1,0,0,0,458,461,1,0,0,0,459,457,1,0,0,
  	0,459,460,1,0,0,0,460,81,1,0,0,0,461,459,1,0,0,0,462,463,5,73,0,0,463,
  	465,3,56,28,0,464,466,3,76,38,0,465,464,1,0,0,0,465,466,1,0,0,0,466,467,
  	1,0,0,0,467,468,5,66,0,0,468,469,3,80,40,0,469,470,5,67,0,0,470,83,1,
  	0,0,0,471,476,3,34,17,0,472,476,3,86,43,0,473,476,3,72,36,0,474,476,5,
  	97,0,0,475,471,1,0,0,0,475,472,1,0,0,0,475,473,1,0,0,0,475,474,1,0,0,
  	0,476,85,1,0,0,0,477,479,3,28,14,0,478,477,1,0,0,0,479,482,1,0,0,0,480,
  	481,1,0,0,0,480,478,1,0,0,0,481,483,1,0,0,0,482,480,1,0,0,0,483,484,3,
  	70,35,0,484,485,3,56,28,0,485,486,5,64,0,0,486,487,3,88,44,0,487,489,
  	5,65,0,0,488,490,3,90,45,0,489,488,1,0,0,0,489,490,1,0,0,0,490,491,1,
  	0,0,0,491,492,5,100,0,0,492,87,1,0,0,0,493,494,5,64,0,0,494,495,3,88,
  	44,0,495,496,5,65,0,0,496,499,1,0,0,0,497,499,8,2,0,0,498,493,1,0,0,0,
  	498,497,1,0,0,0,499,502,1,0,0,0,500,498,1,0,0,0,500,501,1,0,0,0,501,89,
  	1,0,0,0,502,500,1,0,0,0,503,504,5,61,0,0,504,505,5,92,0,0,505,91,1,0,
  	0,0,506,508,3,84,42,0,507,506,1,0,0,0,508,511,1,0,0,0,509,507,1,0,0,0,
  	509,510,1,0,0,0,510,93,1,0,0,0,511,509,1,0,0,0,512,514,3,28,14,0,513,
  	512,1,0,0,0,514,517,1,0,0,0,515,516,1,0,0,0,515,513,1,0,0,0,516,518,1,
  	0,0,0,517,515,1,0,0,0,518,519,5,74,0,0,519,521,3,56,28,0,520,522,3,76,
  	38,0,521,520,1,0,0,0,521,522,1,0,0,0,522,523,1,0,0,0,523,524,5,66,0,0,
  	524,525,3,92,46,0,525,526,5,67,0,0,526,95,1,0,0,0,527,528,3,70,35,0,528,
  	529,3,56,28,0,529,530,5,60,0,0,530,97,1,0,0,0,531,532,5,89,0,0,532,533,
  	3,56,28,0,533,534,5,60,0,0,534,99,1,0,0,0,535,539,3,96,48,0,536,539,3,
  	98,49,0,537,539,5,97,0,0,538,535,1,0,0,0,538,536,1,0,0,0,538,537,1,0,
  	0,0,539,101,1,0,0,0,540,542,3,100,50,0,541,540,1,0,0,0,542,545,1,0,0,
  	0,543,541,1,0,0,0,543,544,1,0,0,0,544,103,1,0,0,0,545,543,1,0,0,0,546,
  	547,5,87,0,0,547,548,3,56,28,0,548,549,5,66,0,0,549,550,3,102,51,0,550,
  	551,5,67,0,0,551,105,1,0,0,0,552,553,3,66,33,0,553,107,1,0,0,0,554,555,
  	5,66,0,0,555,560,3,106,53,0,556,557,5,3,0,0,557,559,3,106,53,0,558,556,
  	1,0,0,0,559,562,1,0,0,0,560,558,1,0,0,0,560,561,1,0,0,0,561,563,1,0,0,
  	0,562,560,1,0,0,0,563,564,5,67,0,0,564,109,1,0,0,0,565,566,5,90,0,0,566,
  	567,5,63,0,0,567,568,3,56,28,0,568,569,5,60,0,0,569,111,1,0,0,0,570,572,
  	3,28,14,0,571,570,1,0,0,0,572,575,1,0,0,0,573,574,1,0,0,0,573,571,1,0,
  	0,0,574,576,1,0,0,0,575,573,1,0,0,0,576,577,3,186,93,0,577,578,5,63,0,
  	0,578,579,3,74,37,0,579,580,5,60,0,0,580,113,1,0,0,0,581,586,3,110,55,
  	0,582,586,3,112,56,0,583,586,3,40,20,0,584,586,5,97,0,0,585,581,1,0,0,
  	0,585,582,1,0,0,0,585,583,1,0,0,0,585,584,1,0,0,0,586,115,1,0,0,0,587,
  	589,3,114,57,0,588,587,1,0,0,0,589,592,1,0,0,0,590,588,1,0,0,0,590,591,
  	1,0,0,0,591,117,1,0,0,0,592,590,1,0,0,0,593,595,3,28,14,0,594,593,1,0,
  	0,0,595,598,1,0,0,0,596,597,1,0,0,0,596,594,1,0,0,0,597,599,1,0,0,0,598,
  	596,1,0,0,0,599,600,5,75,0,0,600,602,3,56,28,0,601,603,3,76,38,0,602,
  	601,1,0,0,0,602,603,1,0,0,0,603,604,1,0,0,0,604,605,5,66,0,0,605,606,
  	3,116,58,0,606,607,5,67,0,0,607,119,1,0,0,0,608,616,3,110,55,0,609,616,
  	3,112,56,0,610,616,3,40,20,0,611,616,3,42,21,0,612,616,3,44,22,0,613,
  	616,3,48,24,0,614,616,5,97,0,0,615,608,1,0,0,0,615,609,1,0,0,0,615,610,
  	1,0,0,0,615,611,1,0,0,0,615,612,1,0,0,0,615,613,1,0,0,0,615,614,1,0,0,
  	0,616,121,1,0,0,0,617,619,3,120,60,0,618,617,1,0,0,0,619,622,1,0,0,0,
  	620,618,1,0,0,0,620,621,1,0,0,0,621,123,1,0,0,0,622,620,1,0,0,0,623,625,
  	3,28,14,0,624,623,1,0,0,0,625,628,1,0,0,0,626,627,1,0,0,0,626,624,1,0,
  	0,0,627,629,1,0,0,0,628,626,1,0,0,0,629,630,5,76,0,0,630,632,3,56,28,
  	0,631,633,3,76,38,0,632,631,1,0,0,0,632,633,1,0,0,0,633,634,1,0,0,0,634,
  	635,5,66,0,0,635,636,3,122,61,0,636,637,5,67,0,0,637,125,1,0,0,0,638,
  	641,3,110,55,0,639,641,5,97,0,0,640,638,1,0,0,0,640,639,1,0,0,0,641,127,
  	1,0,0,0,642,644,3,126,63,0,643,642,1,0,0,0,644,647,1,0,0,0,645,643,1,
  	0,0,0,645,646,1,0,0,0,646,129,1,0,0,0,647,645,1,0,0,0,648,649,5,77,0,
  	0,649,651,3,56,28,0,650,652,3,76,38,0,651,650,1,0,0,0,651,652,1,0,0,0,
  	652,653,1,0,0,0,653,654,5,66,0,0,654,655,3,128,64,0,655,656,5,67,0,0,
  	656,131,1,0,0,0,657,658,7,3,0,0,658,133,1,0,0,0,659,660,3,132,66,0,660,
  	661,5,63,0,0,661,662,3,66,33,0,662,663,5,60,0,0,663,135,1,0,0,0,664,666,
  	3,28,14,0,665,664,1,0,0,0,666,669,1,0,0,0,667,668,1,0,0,0,667,665,1,0,
  	0,0,668,670,1,0,0,0,669,667,1,0,0,0,670,671,5,80,0,0,671,672,3,70,35,
  	0,672,673,3,56,28,0,673,674,5,60,0,0,674,137,1,0,0,0,675,679,3,134,67,
  	0,676,679,3,136,68,0,677,679,5,97,0,0,678,675,1,0,0,0,678,676,1,0,0,0,
  	678,677,1,0,0,0,679,139,1,0,0,0,680,682,3,138,69,0,681,680,1,0,0,0,682,
  	685,1,0,0,0,683,681,1,0,0,0,683,684,1,0,0,0,684,141,1,0,0,0,685,683,1,
  	0,0,0,686,687,5,79,0,0,687,688,3,56,28,0,688,689,5,66,0,0,689,690,3,140,
  	70,0,690,691,5,67,0,0,691,143,1,0,0,0,692,698,3,110,55,0,693,698,3,112,
  	56,0,694,698,3,40,20,0,695,698,3,142,71,0,696,698,5,97,0,0,697,692,1,
  	0,0,0,697,693,1,0,0,0,697,694,1,0,0,0,697,695,1,0,0,0,697,696,1,0,0,0,
  	698,145,1,0,0,0,699,701,3,144,72,0,700,699,1,0,0,0,701,704,1,0,0,0,702,
  	700,1,0,0,0,702,703,1,0,0,0,703,147,1,0,0,0,704,702,1,0,0,0,705,707,3,
  	28,14,0,706,705,1,0,0,0,707,710,1,0,0,0,708,709,1,0,0,0,708,706,1,0,0,
  	0,709,711,1,0,0,0,710,708,1,0,0,0,711,712,5,78,0,0,712,714,3,56,28,0,
  	713,715,3,76,38,0,714,713,1,0,0,0,714,715,1,0,0,0,715,716,1,0,0,0,716,
  	717,5,66,0,0,717,718,3,146,73,0,718,719,5,67,0,0,719,149,1,0,0,0,720,
  	724,3,112,56,0,721,724,5,97,0,0,722,724,3,48,24,0,723,720,1,0,0,0,723,
  	721,1,0,0,0,723,722,1,0,0,0,724,151,1,0,0,0,725,727,3,150,75,0,726,725,
  	1,0,0,0,727,730,1,0,0,0,728,726,1,0,0,0,728,729,1,0,0,0,729,153,1,0,0,
  	0,730,728,1,0,0,0,731,733,3,28,14,0,732,731,1,0,0,0,733,736,1,0,0,0,734,
  	735,1,0,0,0,734,732,1,0,0,0,735,737,1,0,0,0,736,734,1,0,0,0,737,738,5,
  	82,0,0,738,740,3,56,28,0,739,741,3,76,38,0,740,739,1,0,0,0,740,741,1,
  	0,0,0,741,742,1,0,0,0,742,743,5,66,0,0,743,744,3,152,76,0,744,745,5,67,
  	0,0,745,155,1,0,0,0,746,749,3,112,56,0,747,749,5,97,0,0,748,746,1,0,0,
  	0,748,747,1,0,0,0,749,157,1,0,0,0,750,752,3,156,78,0,751,750,1,0,0,0,
  	752,755,1,0,0,0,753,751,1,0,0,0,753,754,1,0,0,0,754,159,1,0,0,0,755,753,
  	1,0,0,0,756,758,3,28,14,0,757,756,1,0,0,0,758,761,1,0,0,0,759,760,1,0,
  	0,0,759,757,1,0,0,0,760,762,1,0,0,0,761,759,1,0,0,0,762,763,5,81,0,0,
  	763,765,3,56,28,0,764,766,3,76,38,0,765,764,1,0,0,0,765,766,1,0,0,0,766,
  	767,1,0,0,0,767,768,5,66,0,0,768,769,3,158,79,0,769,770,5,67,0,0,770,
  	161,1,0,0,0,771,773,3,28,14,0,772,771,1,0,0,0,773,776,1,0,0,0,774,775,
  	1,0,0,0,774,772,1,0,0,0,775,777,1,0,0,0,776,774,1,0,0,0,777,778,3,70,
  	35,0,778,779,3,56,28,0,779,780,5,60,0,0,780,163,1,0,0,0,781,784,3,162,
  	81,0,782,784,5,97,0,0,783,781,1,0,0,0,783,782,1,0,0,0,784,165,1,0,0,0,
  	785,787,3,164,82,0,786,785,1,0,0,0,787,790,1,0,0,0,788,786,1,0,0,0,788,
  	789,1,0,0,0,789,167,1,0,0,0,790,788,1,0,0,0,791,793,3,28,14,0,792,791,
  	1,0,0,0,793,796,1,0,0,0,794,795,1,0,0,0,794,792,1,0,0,0,795,797,1,0,0,
  	0,796,794,1,0,0,0,797,798,5,84,0,0,798,800,3,56,28,0,799,801,3,76,38,
  	0,800,799,1,0,0,0,800,801,1,0,0,0,801,802,1,0,0,0,802,803,5,66,0,0,803,
  	804,3,166,83,0,804,805,5,67,0,0,805,169,1,0,0,0,806,808,3,28,14,0,807,
  	806,1,0,0,0,808,811,1,0,0,0,809,810,1,0,0,0,809,807,1,0,0,0,810,812,1,
  	0,0,0,811,809,1,0,0,0,812,813,5,83,0,0,813,815,3,56,28,0,814,816,3,76,
  	38,0,815,814,1,0,0,0,815,816,1,0,0,0,816,817,1,0,0,0,817,818,5,66,0,0,
  	818,819,3,166,83,0,819,820,5,67,0,0,820,171,1,0,0,0,821,823,3,28,14,0,
  	822,821,1,0,0,0,823,826,1,0,0,0,824,822,1,0,0,0,824,825,1,0,0,0,825,827,
  	1,0,0,0,826,824,1,0,0,0,827,828,3,56,28,0,828,829,5,60,0,0,829,832,1,
  	0,0,0,830,832,5,97,0,0,831,824,1,0,0,0,831,830,1,0,0,0,832,173,1,0,0,
  	0,833,835,3,172,86,0,834,833,1,0,0,0,835,838,1,0,0,0,836,834,1,0,0,0,
  	836,837,1,0,0,0,837,175,1,0,0,0,838,836,1,0,0,0,839,840,5,85,0,0,840,
  	841,3,56,28,0,841,842,5,66,0,0,842,843,3,174,87,0,843,844,5,67,0,0,844,
  	177,1,0,0,0,845,848,3,56,28,0,846,847,5,63,0,0,847,849,3,66,33,0,848,
  	846,1,0,0,0,848,849,1,0,0,0,849,850,1,0,0,0,850,851,5,60,0,0,851,179,
  	1,0,0,0,852,855,3,178,89,0,853,855,5,97,0,0,854,852,1,0,0,0,854,853,1,
  	0,0,0,855,181,1,0,0,0,856,858,3,180,90,0,857,856,1,0,0,0,858,861,1,0,
  	0,0,859,857,1,0,0,0,859,860,1,0,0,0,860,183,1,0,0,0,861,859,1,0,0,0,862,
  	863,5,91,0,0,863,864,3,56,28,0,864,865,5,66,0,0,865,866,3,182,91,0,866,
  	867,5,67,0,0,867,185,1,0,0,0,868,869,7,4,0,0,869,187,1,0,0,0,870,871,
  	7,5,0,0,871,189,1,0,0,0,872,873,7,6,0,0,873,191,1,0,0,0,82,207,209,222,
  	229,233,238,246,255,263,284,292,301,308,314,318,335,342,349,360,373,388,
  	392,395,410,416,428,434,448,454,459,465,475,480,489,498,500,509,515,521,
  	538,543,560,573,585,590,596,602,615,620,626,632,640,645,651,667,678,683,
  	697,702,708,714,723,728,734,740,748,753,759,765,774,783,788,794,800,809,
  	815,824,831,836,848,854,859
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  prismParserStaticData = staticData.release();
}

}

PrismParser::PrismParser(TokenStream *input) : PrismParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

PrismParser::PrismParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  PrismParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *prismParserStaticData->atn, prismParserStaticData->decisionToDFA, prismParserStaticData->sharedContextCache, options);
}

PrismParser::~PrismParser() {
  delete _interpreter;
}

const atn::ATN& PrismParser::getATN() const {
  return *prismParserStaticData->atn;
}

std::string PrismParser::getGrammarFileName() const {
  return "Prism.g4";
}

const std::vector<std::string>& PrismParser::getRuleNames() const {
  return prismParserStaticData->ruleNames;
}

const dfa::Vocabulary& PrismParser::getVocabulary() const {
  return prismParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView PrismParser::getSerializedATN() const {
  return prismParserStaticData->serializedATN;
}


//----------------- ParseContext ------------------------------------------------------------------

PrismParser::ParseContext::ParseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::ParseContext::EOF() {
  return getToken(PrismParser::EOF, 0);
}

std::vector<PrismParser::Layout_definitionContext *> PrismParser::ParseContext::layout_definition() {
  return getRuleContexts<PrismParser::Layout_definitionContext>();
}

PrismParser::Layout_definitionContext* PrismParser::ParseContext::layout_definition(size_t i) {
  return getRuleContext<PrismParser::Layout_definitionContext>(i);
}

std::vector<PrismParser::Table_definitionContext *> PrismParser::ParseContext::table_definition() {
  return getRuleContexts<PrismParser::Table_definitionContext>();
}

PrismParser::Table_definitionContext* PrismParser::ParseContext::table_definition(size_t i) {
  return getRuleContext<PrismParser::Table_definitionContext>(i);
}

std::vector<PrismParser::Rt_definitionContext *> PrismParser::ParseContext::rt_definition() {
  return getRuleContexts<PrismParser::Rt_definitionContext>();
}

PrismParser::Rt_definitionContext* PrismParser::ParseContext::rt_definition(size_t i) {
  return getRuleContext<PrismParser::Rt_definitionContext>(i);
}

std::vector<PrismParser::Workgraph_pso_definitionContext *> PrismParser::ParseContext::workgraph_pso_definition() {
  return getRuleContexts<PrismParser::Workgraph_pso_definitionContext>();
}

PrismParser::Workgraph_pso_definitionContext* PrismParser::ParseContext::workgraph_pso_definition(size_t i) {
  return getRuleContext<PrismParser::Workgraph_pso_definitionContext>(i);
}

std::vector<PrismParser::Compute_pso_definitionContext *> PrismParser::ParseContext::compute_pso_definition() {
  return getRuleContexts<PrismParser::Compute_pso_definitionContext>();
}

PrismParser::Compute_pso_definitionContext* PrismParser::ParseContext::compute_pso_definition(size_t i) {
  return getRuleContext<PrismParser::Compute_pso_definitionContext>(i);
}

std::vector<PrismParser::Graphics_pso_definitionContext *> PrismParser::ParseContext::graphics_pso_definition() {
  return getRuleContexts<PrismParser::Graphics_pso_definitionContext>();
}

PrismParser::Graphics_pso_definitionContext* PrismParser::ParseContext::graphics_pso_definition(size_t i) {
  return getRuleContext<PrismParser::Graphics_pso_definitionContext>(i);
}

std::vector<PrismParser::Rtx_pso_definitionContext *> PrismParser::ParseContext::rtx_pso_definition() {
  return getRuleContexts<PrismParser::Rtx_pso_definitionContext>();
}

PrismParser::Rtx_pso_definitionContext* PrismParser::ParseContext::rtx_pso_definition(size_t i) {
  return getRuleContext<PrismParser::Rtx_pso_definitionContext>(i);
}

std::vector<PrismParser::Rtx_pass_definitionContext *> PrismParser::ParseContext::rtx_pass_definition() {
  return getRuleContexts<PrismParser::Rtx_pass_definitionContext>();
}

PrismParser::Rtx_pass_definitionContext* PrismParser::ParseContext::rtx_pass_definition(size_t i) {
  return getRuleContext<PrismParser::Rtx_pass_definitionContext>(i);
}

std::vector<PrismParser::Rtx_raygen_definitionContext *> PrismParser::ParseContext::rtx_raygen_definition() {
  return getRuleContexts<PrismParser::Rtx_raygen_definitionContext>();
}

PrismParser::Rtx_raygen_definitionContext* PrismParser::ParseContext::rtx_raygen_definition(size_t i) {
  return getRuleContext<PrismParser::Rtx_raygen_definitionContext>(i);
}

std::vector<PrismParser::Pass_definitionContext *> PrismParser::ParseContext::pass_definition() {
  return getRuleContexts<PrismParser::Pass_definitionContext>();
}

PrismParser::Pass_definitionContext* PrismParser::ParseContext::pass_definition(size_t i) {
  return getRuleContext<PrismParser::Pass_definitionContext>(i);
}

std::vector<PrismParser::View_definitionContext *> PrismParser::ParseContext::view_definition() {
  return getRuleContexts<PrismParser::View_definitionContext>();
}

PrismParser::View_definitionContext* PrismParser::ParseContext::view_definition(size_t i) {
  return getRuleContext<PrismParser::View_definitionContext>(i);
}

std::vector<PrismParser::Pipeline_definitionContext *> PrismParser::ParseContext::pipeline_definition() {
  return getRuleContexts<PrismParser::Pipeline_definitionContext>();
}

PrismParser::Pipeline_definitionContext* PrismParser::ParseContext::pipeline_definition(size_t i) {
  return getRuleContext<PrismParser::Pipeline_definitionContext>(i);
}

std::vector<PrismParser::Enum_definitionContext *> PrismParser::ParseContext::enum_definition() {
  return getRuleContexts<PrismParser::Enum_definitionContext>();
}

PrismParser::Enum_definitionContext* PrismParser::ParseContext::enum_definition(size_t i) {
  return getRuleContext<PrismParser::Enum_definitionContext>(i);
}

std::vector<PrismParser::Const_definitionContext *> PrismParser::ParseContext::const_definition() {
  return getRuleContexts<PrismParser::Const_definitionContext>();
}

PrismParser::Const_definitionContext* PrismParser::ParseContext::const_definition(size_t i) {
  return getRuleContext<PrismParser::Const_definitionContext>(i);
}

std::vector<tree::TerminalNode *> PrismParser::ParseContext::COMMENT() {
  return getTokens(PrismParser::COMMENT);
}

tree::TerminalNode* PrismParser::ParseContext::COMMENT(size_t i) {
  return getToken(PrismParser::COMMENT, i);
}


size_t PrismParser::ParseContext::getRuleIndex() const {
  return PrismParser::RuleParse;
}

void PrismParser::ParseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParse(this);
}

void PrismParser::ParseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParse(this);
}


std::any PrismParser::ParseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitParse(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::ParseContext* PrismParser::parse() {
  ParseContext *_localctx = _tracker.createInstance<ParseContext>(_ctx, getState());
  enterRule(_localctx, 0, PrismParser::RuleParse);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(209);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 546039777) != 0) {
      setState(207);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(192);
        layout_definition();
        break;
      }

      case 2: {
        setState(193);
        table_definition();
        break;
      }

      case 3: {
        setState(194);
        rt_definition();
        break;
      }

      case 4: {
        setState(195);
        workgraph_pso_definition();
        break;
      }

      case 5: {
        setState(196);
        compute_pso_definition();
        break;
      }

      case 6: {
        setState(197);
        graphics_pso_definition();
        break;
      }

      case 7: {
        setState(198);
        rtx_pso_definition();
        break;
      }

      case 8: {
        setState(199);
        rtx_pass_definition();
        break;
      }

      case 9: {
        setState(200);
        rtx_raygen_definition();
        break;
      }

      case 10: {
        setState(201);
        pass_definition();
        break;
      }

      case 11: {
        setState(202);
        view_definition();
        break;
      }

      case 12: {
        setState(203);
        pipeline_definition();
        break;
      }

      case 13: {
        setState(204);
        enum_definition();
        break;
      }

      case 14: {
        setState(205);
        const_definition();
        break;
      }

      case 15: {
        setState(206);
        match(PrismParser::COMMENT);
        break;
      }

      default:
        break;
      }
      setState(211);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(212);
    match(PrismParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Const_definitionContext ------------------------------------------------------------------

PrismParser::Const_definitionContext::Const_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::Const_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

PrismParser::Options_assignContext* PrismParser::Const_definitionContext::options_assign() {
  return getRuleContext<PrismParser::Options_assignContext>(0);
}

tree::TerminalNode* PrismParser::Const_definitionContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Const_definitionContext::getRuleIndex() const {
  return PrismParser::RuleConst_definition;
}

void PrismParser::Const_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConst_definition(this);
}

void PrismParser::Const_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConst_definition(this);
}


std::any PrismParser::Const_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitConst_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Const_definitionContext* PrismParser::const_definition() {
  Const_definitionContext *_localctx = _tracker.createInstance<Const_definitionContext>(_ctx, getState());
  enterRule(_localctx, 2, PrismParser::RuleConst_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(214);
    match(PrismParser::T__0);
    setState(215);
    name_id();
    setState(216);
    options_assign();
    setState(217);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bind_optionContext ------------------------------------------------------------------

PrismParser::Bind_optionContext::Bind_optionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Flag_value_holderContext *> PrismParser::Bind_optionContext::flag_value_holder() {
  return getRuleContexts<PrismParser::Flag_value_holderContext>();
}

PrismParser::Flag_value_holderContext* PrismParser::Bind_optionContext::flag_value_holder(size_t i) {
  return getRuleContext<PrismParser::Flag_value_holderContext>(i);
}

PrismParser::Owner_idContext* PrismParser::Bind_optionContext::owner_id() {
  return getRuleContext<PrismParser::Owner_idContext>(0);
}

std::vector<tree::TerminalNode *> PrismParser::Bind_optionContext::PIPE() {
  return getTokens(PrismParser::PIPE);
}

tree::TerminalNode* PrismParser::Bind_optionContext::PIPE(size_t i) {
  return getToken(PrismParser::PIPE, i);
}

PrismParser::Raw_valueContext* PrismParser::Bind_optionContext::raw_value() {
  return getRuleContext<PrismParser::Raw_valueContext>(0);
}

PrismParser::Cond_exprContext* PrismParser::Bind_optionContext::cond_expr() {
  return getRuleContext<PrismParser::Cond_exprContext>(0);
}


size_t PrismParser::Bind_optionContext::getRuleIndex() const {
  return PrismParser::RuleBind_option;
}

void PrismParser::Bind_optionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBind_option(this);
}

void PrismParser::Bind_optionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBind_option(this);
}


std::any PrismParser::Bind_optionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitBind_option(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Bind_optionContext* PrismParser::bind_option() {
  Bind_optionContext *_localctx = _tracker.createInstance<Bind_optionContext>(_ctx, getState());
  enterRule(_localctx, 4, PrismParser::RuleBind_option);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(233);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(222);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
      case 1: {
        setState(219);
        owner_id();
        setState(220);
        match(PrismParser::T__1);
        break;
      }

      default:
        break;
      }
      setState(224);
      flag_value_holder();
      setState(227); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(225);
        match(PrismParser::PIPE);
        setState(226);
        flag_value_holder();
        setState(229); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == PrismParser::PIPE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(231);
      raw_value();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(232);
      cond_expr();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Cond_exprContext ------------------------------------------------------------------

PrismParser::Cond_exprContext::Cond_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Cond_termContext *> PrismParser::Cond_exprContext::cond_term() {
  return getRuleContexts<PrismParser::Cond_termContext>();
}

PrismParser::Cond_termContext* PrismParser::Cond_exprContext::cond_term(size_t i) {
  return getRuleContext<PrismParser::Cond_termContext>(i);
}


size_t PrismParser::Cond_exprContext::getRuleIndex() const {
  return PrismParser::RuleCond_expr;
}

void PrismParser::Cond_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_expr(this);
}

void PrismParser::Cond_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_expr(this);
}


std::any PrismParser::Cond_exprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCond_expr(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Cond_exprContext* PrismParser::cond_expr() {
  Cond_exprContext *_localctx = _tracker.createInstance<Cond_exprContext>(_ctx, getState());
  enterRule(_localctx, 6, PrismParser::RuleCond_expr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(236); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(235);
      cond_term();
      setState(238); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 864515206661791744) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 36238786759) != 0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Cond_termContext ------------------------------------------------------------------

PrismParser::Cond_termContext::Cond_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Qualified_refContext* PrismParser::Cond_termContext::qualified_ref() {
  return getRuleContext<PrismParser::Qualified_refContext>(0);
}

PrismParser::Function_idContext* PrismParser::Cond_termContext::function_id() {
  return getRuleContext<PrismParser::Function_idContext>(0);
}

PrismParser::CallContext* PrismParser::Cond_termContext::call() {
  return getRuleContext<PrismParser::CallContext>(0);
}

PrismParser::Member_refContext* PrismParser::Cond_termContext::member_ref() {
  return getRuleContext<PrismParser::Member_refContext>(0);
}

PrismParser::Value_idContext* PrismParser::Cond_termContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}

PrismParser::Cond_opContext* PrismParser::Cond_termContext::cond_op() {
  return getRuleContext<PrismParser::Cond_opContext>(0);
}


size_t PrismParser::Cond_termContext::getRuleIndex() const {
  return PrismParser::RuleCond_term;
}

void PrismParser::Cond_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_term(this);
}

void PrismParser::Cond_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_term(this);
}


std::any PrismParser::Cond_termContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCond_term(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Cond_termContext* PrismParser::cond_term() {
  Cond_termContext *_localctx = _tracker.createInstance<Cond_termContext>(_ctx, getState());
  enterRule(_localctx, 8, PrismParser::RuleCond_term);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(246);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(240);
      qualified_ref();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(241);
      function_id();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(242);
      call();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(243);
      member_ref();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(244);
      value_id();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(245);
      cond_op();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallContext ------------------------------------------------------------------

PrismParser::CallContext::CallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::CallContext::ID() {
  return getToken(PrismParser::ID, 0);
}

tree::TerminalNode* PrismParser::CallContext::OPAR() {
  return getToken(PrismParser::OPAR, 0);
}

std::vector<PrismParser::Call_argContext *> PrismParser::CallContext::call_arg() {
  return getRuleContexts<PrismParser::Call_argContext>();
}

PrismParser::Call_argContext* PrismParser::CallContext::call_arg(size_t i) {
  return getRuleContext<PrismParser::Call_argContext>(i);
}

tree::TerminalNode* PrismParser::CallContext::CPAR() {
  return getToken(PrismParser::CPAR, 0);
}


size_t PrismParser::CallContext::getRuleIndex() const {
  return PrismParser::RuleCall;
}

void PrismParser::CallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCall(this);
}

void PrismParser::CallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCall(this);
}


std::any PrismParser::CallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCall(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::CallContext* PrismParser::call() {
  CallContext *_localctx = _tracker.createInstance<CallContext>(_ctx, getState());
  enterRule(_localctx, 10, PrismParser::RuleCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(248);
    match(PrismParser::ID);
    setState(249);
    match(PrismParser::OPAR);
    setState(250);
    call_arg();
    setState(255);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(251);
      match(PrismParser::T__2);
      setState(252);
      call_arg();
      setState(257);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(258);
    match(PrismParser::CPAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Call_argContext ------------------------------------------------------------------

PrismParser::Call_argContext::Call_argContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Cond_termContext *> PrismParser::Call_argContext::cond_term() {
  return getRuleContexts<PrismParser::Cond_termContext>();
}

PrismParser::Cond_termContext* PrismParser::Call_argContext::cond_term(size_t i) {
  return getRuleContext<PrismParser::Cond_termContext>(i);
}


size_t PrismParser::Call_argContext::getRuleIndex() const {
  return PrismParser::RuleCall_arg;
}

void PrismParser::Call_argContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCall_arg(this);
}

void PrismParser::Call_argContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCall_arg(this);
}


std::any PrismParser::Call_argContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCall_arg(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Call_argContext* PrismParser::call_arg() {
  Call_argContext *_localctx = _tracker.createInstance<Call_argContext>(_ctx, getState());
  enterRule(_localctx, 12, PrismParser::RuleCall_arg);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(261); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(260);
              cond_term();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(263); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Qualified_refContext ------------------------------------------------------------------

PrismParser::Qualified_refContext::Qualified_refContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Owner_idContext* PrismParser::Qualified_refContext::owner_id() {
  return getRuleContext<PrismParser::Owner_idContext>(0);
}

PrismParser::Value_idContext* PrismParser::Qualified_refContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}


size_t PrismParser::Qualified_refContext::getRuleIndex() const {
  return PrismParser::RuleQualified_ref;
}

void PrismParser::Qualified_refContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQualified_ref(this);
}

void PrismParser::Qualified_refContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQualified_ref(this);
}


std::any PrismParser::Qualified_refContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitQualified_ref(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Qualified_refContext* PrismParser::qualified_ref() {
  Qualified_refContext *_localctx = _tracker.createInstance<Qualified_refContext>(_ctx, getState());
  enterRule(_localctx, 14, PrismParser::RuleQualified_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(265);
    owner_id();
    setState(266);
    match(PrismParser::T__1);
    setState(267);
    value_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Member_refContext ------------------------------------------------------------------

PrismParser::Member_refContext::Member_refContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Name_idContext *> PrismParser::Member_refContext::name_id() {
  return getRuleContexts<PrismParser::Name_idContext>();
}

PrismParser::Name_idContext* PrismParser::Member_refContext::name_id(size_t i) {
  return getRuleContext<PrismParser::Name_idContext>(i);
}

tree::TerminalNode* PrismParser::Member_refContext::DOT() {
  return getToken(PrismParser::DOT, 0);
}


size_t PrismParser::Member_refContext::getRuleIndex() const {
  return PrismParser::RuleMember_ref;
}

void PrismParser::Member_refContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMember_ref(this);
}

void PrismParser::Member_refContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMember_ref(this);
}


std::any PrismParser::Member_refContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitMember_ref(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Member_refContext* PrismParser::member_ref() {
  Member_refContext *_localctx = _tracker.createInstance<Member_refContext>(_ctx, getState());
  enterRule(_localctx, 16, PrismParser::RuleMember_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(269);
    name_id();
    setState(270);
    match(PrismParser::DOT);
    setState(271);
    name_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Cond_opContext ------------------------------------------------------------------

PrismParser::Cond_opContext::Cond_opContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Cond_opContext::AND() {
  return getToken(PrismParser::AND, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::OR() {
  return getToken(PrismParser::OR, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::NOT() {
  return getToken(PrismParser::NOT, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::EQ() {
  return getToken(PrismParser::EQ, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::NEQ() {
  return getToken(PrismParser::NEQ, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::GTEQ() {
  return getToken(PrismParser::GTEQ, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::LTEQ() {
  return getToken(PrismParser::LTEQ, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::GT() {
  return getToken(PrismParser::GT, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::LT() {
  return getToken(PrismParser::LT, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::OPAR() {
  return getToken(PrismParser::OPAR, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::CPAR() {
  return getToken(PrismParser::CPAR, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::PLUS() {
  return getToken(PrismParser::PLUS, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::MINUS() {
  return getToken(PrismParser::MINUS, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::POINTER() {
  return getToken(PrismParser::POINTER, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::DIV() {
  return getToken(PrismParser::DIV, 0);
}

tree::TerminalNode* PrismParser::Cond_opContext::MOD() {
  return getToken(PrismParser::MOD, 0);
}


size_t PrismParser::Cond_opContext::getRuleIndex() const {
  return PrismParser::RuleCond_op;
}

void PrismParser::Cond_opContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_op(this);
}

void PrismParser::Cond_opContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_op(this);
}


std::any PrismParser::Cond_opContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCond_op(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Cond_opContext* PrismParser::cond_op() {
  Cond_opContext *_localctx = _tracker.createInstance<Cond_opContext>(_ctx, getState());
  enterRule(_localctx, 18, PrismParser::RuleCond_op);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(273);
    _la = _input->LA(1);
    if (!((((_la - 45) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 45)) & 18014398511079419) != 0)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Flag_value_holderContext ------------------------------------------------------------------

PrismParser::Flag_value_holderContext::Flag_value_holderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Value_idContext* PrismParser::Flag_value_holderContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}


size_t PrismParser::Flag_value_holderContext::getRuleIndex() const {
  return PrismParser::RuleFlag_value_holder;
}

void PrismParser::Flag_value_holderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFlag_value_holder(this);
}

void PrismParser::Flag_value_holderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFlag_value_holder(this);
}


std::any PrismParser::Flag_value_holderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitFlag_value_holder(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Flag_value_holderContext* PrismParser::flag_value_holder() {
  Flag_value_holderContext *_localctx = _tracker.createInstance<Flag_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 20, PrismParser::RuleFlag_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(275);
    value_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Raw_valueContext ------------------------------------------------------------------

PrismParser::Raw_valueContext::Raw_valueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Raw_valueContext::RAWEXPR() {
  return getToken(PrismParser::RAWEXPR, 0);
}


size_t PrismParser::Raw_valueContext::getRuleIndex() const {
  return PrismParser::RuleRaw_value;
}

void PrismParser::Raw_valueContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRaw_value(this);
}

void PrismParser::Raw_valueContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRaw_value(this);
}


std::any PrismParser::Raw_valueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRaw_value(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Raw_valueContext* PrismParser::raw_value() {
  Raw_valueContext *_localctx = _tracker.createInstance<Raw_valueContext>(_ctx, getState());
  enterRule(_localctx, 22, PrismParser::RuleRaw_value);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(277);
    match(PrismParser::RAWEXPR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Options_assignContext ------------------------------------------------------------------

PrismParser::Options_assignContext::Options_assignContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Options_assignContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Bind_optionContext* PrismParser::Options_assignContext::bind_option() {
  return getRuleContext<PrismParser::Bind_optionContext>(0);
}


size_t PrismParser::Options_assignContext::getRuleIndex() const {
  return PrismParser::RuleOptions_assign;
}

void PrismParser::Options_assignContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOptions_assign(this);
}

void PrismParser::Options_assignContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOptions_assign(this);
}


std::any PrismParser::Options_assignContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitOptions_assign(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Options_assignContext* PrismParser::options_assign() {
  Options_assignContext *_localctx = _tracker.createInstance<Options_assignContext>(_ctx, getState());
  enterRule(_localctx, 24, PrismParser::RuleOptions_assign);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(279);
    match(PrismParser::ASSIGN);
    setState(280);
    bind_option();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OptionContext ------------------------------------------------------------------

PrismParser::OptionContext::OptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::OptionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

PrismParser::Options_assignContext* PrismParser::OptionContext::options_assign() {
  return getRuleContext<PrismParser::Options_assignContext>(0);
}


size_t PrismParser::OptionContext::getRuleIndex() const {
  return PrismParser::RuleOption;
}

void PrismParser::OptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption(this);
}

void PrismParser::OptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption(this);
}


std::any PrismParser::OptionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitOption(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::OptionContext* PrismParser::option() {
  OptionContext *_localctx = _tracker.createInstance<OptionContext>(_ctx, getState());
  enterRule(_localctx, 26, PrismParser::RuleOption);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(282);
    name_id();
    setState(284);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(283);
      options_assign();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Option_blockContext ------------------------------------------------------------------

PrismParser::Option_blockContext::Option_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Option_blockContext::OSBRACE() {
  return getToken(PrismParser::OSBRACE, 0);
}

std::vector<PrismParser::OptionContext *> PrismParser::Option_blockContext::option() {
  return getRuleContexts<PrismParser::OptionContext>();
}

PrismParser::OptionContext* PrismParser::Option_blockContext::option(size_t i) {
  return getRuleContext<PrismParser::OptionContext>(i);
}

tree::TerminalNode* PrismParser::Option_blockContext::CSBRACE() {
  return getToken(PrismParser::CSBRACE, 0);
}


size_t PrismParser::Option_blockContext::getRuleIndex() const {
  return PrismParser::RuleOption_block;
}

void PrismParser::Option_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_block(this);
}

void PrismParser::Option_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_block(this);
}


std::any PrismParser::Option_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitOption_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Option_blockContext* PrismParser::option_block() {
  Option_blockContext *_localctx = _tracker.createInstance<Option_blockContext>(_ctx, getState());
  enterRule(_localctx, 28, PrismParser::RuleOption_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(286);
    match(PrismParser::OSBRACE);
    setState(287);
    option();
    setState(292);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(288);
      match(PrismParser::T__2);
      setState(289);
      option();
      setState(294);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(295);
    match(PrismParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_count_idContext ------------------------------------------------------------------

PrismParser::Array_count_idContext::Array_count_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Array_count_idContext::INT_SCALAR() {
  return getToken(PrismParser::INT_SCALAR, 0);
}


size_t PrismParser::Array_count_idContext::getRuleIndex() const {
  return PrismParser::RuleArray_count_id;
}

void PrismParser::Array_count_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_count_id(this);
}

void PrismParser::Array_count_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_count_id(this);
}


std::any PrismParser::Array_count_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitArray_count_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Array_count_idContext* PrismParser::array_count_id() {
  Array_count_idContext *_localctx = _tracker.createInstance<Array_count_idContext>(_ctx, getState());
  enterRule(_localctx, 30, PrismParser::RuleArray_count_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(297);
    match(PrismParser::INT_SCALAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayContext ------------------------------------------------------------------

PrismParser::ArrayContext::ArrayContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::ArrayContext::OSBRACE() {
  return getToken(PrismParser::OSBRACE, 0);
}

tree::TerminalNode* PrismParser::ArrayContext::CSBRACE() {
  return getToken(PrismParser::CSBRACE, 0);
}

PrismParser::Array_count_idContext* PrismParser::ArrayContext::array_count_id() {
  return getRuleContext<PrismParser::Array_count_idContext>(0);
}


size_t PrismParser::ArrayContext::getRuleIndex() const {
  return PrismParser::RuleArray;
}

void PrismParser::ArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray(this);
}

void PrismParser::ArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray(this);
}


std::any PrismParser::ArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitArray(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::ArrayContext* PrismParser::array() {
  ArrayContext *_localctx = _tracker.createInstance<ArrayContext>(_ctx, getState());
  enterRule(_localctx, 32, PrismParser::RuleArray);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(299);
    match(PrismParser::OSBRACE);
    setState(301);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::INT_SCALAR) {
      setState(300);
      array_count_id();
    }
    setState(303);
    match(PrismParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_declarationContext ------------------------------------------------------------------

PrismParser::Value_declarationContext::Value_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Type_idContext* PrismParser::Value_declarationContext::type_id() {
  return getRuleContext<PrismParser::Type_idContext>(0);
}

PrismParser::Name_idContext* PrismParser::Value_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Value_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Value_declarationContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Value_declarationContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::ArrayContext* PrismParser::Value_declarationContext::array() {
  return getRuleContext<PrismParser::ArrayContext>(0);
}

tree::TerminalNode* PrismParser::Value_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Value_idContext* PrismParser::Value_declarationContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}


size_t PrismParser::Value_declarationContext::getRuleIndex() const {
  return PrismParser::RuleValue_declaration;
}

void PrismParser::Value_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_declaration(this);
}

void PrismParser::Value_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_declaration(this);
}


std::any PrismParser::Value_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitValue_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Value_declarationContext* PrismParser::value_declaration() {
  Value_declarationContext *_localctx = _tracker.createInstance<Value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 34, PrismParser::RuleValue_declaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(308);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(305);
        option_block(); 
      }
      setState(310);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    }
    setState(311);
    type_id();
    setState(312);
    name_id();
    setState(314);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::OSBRACE) {
      setState(313);
      array();
    }
    setState(318);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(316);
      match(PrismParser::ASSIGN);
      setState(317);
      value_id();
    }
    setState(320);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Slot_declarationContext ------------------------------------------------------------------

PrismParser::Slot_declarationContext::Slot_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Slot_declarationContext::SLOT() {
  return getToken(PrismParser::SLOT, 0);
}

PrismParser::Name_idContext* PrismParser::Slot_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Slot_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Slot_declarationContext::getRuleIndex() const {
  return PrismParser::RuleSlot_declaration;
}

void PrismParser::Slot_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSlot_declaration(this);
}

void PrismParser::Slot_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSlot_declaration(this);
}


std::any PrismParser::Slot_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitSlot_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Slot_declarationContext* PrismParser::slot_declaration() {
  Slot_declarationContext *_localctx = _tracker.createInstance<Slot_declarationContext>(_ctx, getState());
  enterRule(_localctx, 36, PrismParser::RuleSlot_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(322);
    match(PrismParser::SLOT);
    setState(323);
    name_id();
    setState(324);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Sampler_declarationContext ------------------------------------------------------------------

PrismParser::Sampler_declarationContext::Sampler_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::Sampler_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Sampler_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Value_idContext* PrismParser::Sampler_declarationContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}

tree::TerminalNode* PrismParser::Sampler_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Sampler_declarationContext::getRuleIndex() const {
  return PrismParser::RuleSampler_declaration;
}

void PrismParser::Sampler_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSampler_declaration(this);
}

void PrismParser::Sampler_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSampler_declaration(this);
}


std::any PrismParser::Sampler_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitSampler_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Sampler_declarationContext* PrismParser::sampler_declaration() {
  Sampler_declarationContext *_localctx = _tracker.createInstance<Sampler_declarationContext>(_ctx, getState());
  enterRule(_localctx, 38, PrismParser::RuleSampler_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(326);
    match(PrismParser::T__3);
    setState(327);
    name_id();
    setState(328);
    match(PrismParser::ASSIGN);
    setState(329);
    value_id();
    setState(330);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Define_declarationContext ------------------------------------------------------------------

PrismParser::Define_declarationContext::Define_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::Define_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Define_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Define_declarationContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Define_declarationContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

tree::TerminalNode* PrismParser::Define_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Array_value_idsContext* PrismParser::Define_declarationContext::array_value_ids() {
  return getRuleContext<PrismParser::Array_value_idsContext>(0);
}


size_t PrismParser::Define_declarationContext::getRuleIndex() const {
  return PrismParser::RuleDefine_declaration;
}

void PrismParser::Define_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefine_declaration(this);
}

void PrismParser::Define_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefine_declaration(this);
}


std::any PrismParser::Define_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitDefine_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Define_declarationContext* PrismParser::define_declaration() {
  Define_declarationContext *_localctx = _tracker.createInstance<Define_declarationContext>(_ctx, getState());
  enterRule(_localctx, 40, PrismParser::RuleDefine_declaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(335);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(332);
        option_block(); 
      }
      setState(337);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    }
    setState(338);
    match(PrismParser::T__4);
    setState(339);
    name_id();
    setState(342);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(340);
      match(PrismParser::ASSIGN);
      setState(341);
      array_value_ids();
    }
    setState(344);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtv_formats_declarationContext ------------------------------------------------------------------

PrismParser::Rtv_formats_declarationContext::Rtv_formats_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rtv_formats_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Array_value_idsContext* PrismParser::Rtv_formats_declarationContext::array_value_ids() {
  return getRuleContext<PrismParser::Array_value_idsContext>(0);
}

tree::TerminalNode* PrismParser::Rtv_formats_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Rtv_formats_declarationContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Rtv_formats_declarationContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::Rtv_formats_declarationContext::getRuleIndex() const {
  return PrismParser::RuleRtv_formats_declaration;
}

void PrismParser::Rtv_formats_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtv_formats_declaration(this);
}

void PrismParser::Rtv_formats_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtv_formats_declaration(this);
}


std::any PrismParser::Rtv_formats_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtv_formats_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtv_formats_declarationContext* PrismParser::rtv_formats_declaration() {
  Rtv_formats_declarationContext *_localctx = _tracker.createInstance<Rtv_formats_declarationContext>(_ctx, getState());
  enterRule(_localctx, 42, PrismParser::RuleRtv_formats_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(349);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(346);
        option_block(); 
      }
      setState(351);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    }
    setState(352);
    match(PrismParser::T__5);
    setState(353);
    match(PrismParser::ASSIGN);
    setState(354);
    array_value_ids();
    setState(355);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Blends_declarationContext ------------------------------------------------------------------

PrismParser::Blends_declarationContext::Blends_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Blends_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Array_value_idsContext* PrismParser::Blends_declarationContext::array_value_ids() {
  return getRuleContext<PrismParser::Array_value_idsContext>(0);
}

tree::TerminalNode* PrismParser::Blends_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Blends_declarationContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Blends_declarationContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::Blends_declarationContext::getRuleIndex() const {
  return PrismParser::RuleBlends_declaration;
}

void PrismParser::Blends_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlends_declaration(this);
}

void PrismParser::Blends_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlends_declaration(this);
}


std::any PrismParser::Blends_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitBlends_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Blends_declarationContext* PrismParser::blends_declaration() {
  Blends_declarationContext *_localctx = _tracker.createInstance<Blends_declarationContext>(_ctx, getState());
  enterRule(_localctx, 44, PrismParser::RuleBlends_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(360);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(357);
        option_block(); 
      }
      setState(362);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx);
    }
    setState(363);
    match(PrismParser::T__6);
    setState(364);
    match(PrismParser::ASSIGN);
    setState(365);
    array_value_ids();
    setState(366);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PointerContext ------------------------------------------------------------------

PrismParser::PointerContext::PointerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::PointerContext::POINTER() {
  return getToken(PrismParser::POINTER, 0);
}


size_t PrismParser::PointerContext::getRuleIndex() const {
  return PrismParser::RulePointer;
}

void PrismParser::PointerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPointer(this);
}

void PrismParser::PointerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPointer(this);
}


std::any PrismParser::PointerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPointer(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::PointerContext* PrismParser::pointer() {
  PointerContext *_localctx = _tracker.createInstance<PointerContext>(_ctx, getState());
  enterRule(_localctx, 46, PrismParser::RulePointer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(368);
    match(PrismParser::POINTER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pso_paramContext ------------------------------------------------------------------

PrismParser::Pso_paramContext::Pso_paramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Pso_param_idContext* PrismParser::Pso_paramContext::pso_param_id() {
  return getRuleContext<PrismParser::Pso_param_idContext>(0);
}

tree::TerminalNode* PrismParser::Pso_paramContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Value_idContext* PrismParser::Pso_paramContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}

tree::TerminalNode* PrismParser::Pso_paramContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Pso_paramContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Pso_paramContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::Pso_paramContext::getRuleIndex() const {
  return PrismParser::RulePso_param;
}

void PrismParser::Pso_paramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param(this);
}

void PrismParser::Pso_paramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param(this);
}


std::any PrismParser::Pso_paramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPso_param(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pso_paramContext* PrismParser::pso_param() {
  Pso_paramContext *_localctx = _tracker.createInstance<Pso_paramContext>(_ctx, getState());
  enterRule(_localctx, 48, PrismParser::RulePso_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(373);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(370);
        option_block(); 
      }
      setState(375);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    }
    setState(376);
    pso_param_id();
    setState(377);
    match(PrismParser::ASSIGN);
    setState(378);
    value_id();
    setState(379);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Class_no_templateContext ------------------------------------------------------------------

PrismParser::Class_no_templateContext::Class_no_templateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Class_no_templateContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Class_no_templateContext::getRuleIndex() const {
  return PrismParser::RuleClass_no_template;
}

void PrismParser::Class_no_templateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClass_no_template(this);
}

void PrismParser::Class_no_templateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClass_no_template(this);
}


std::any PrismParser::Class_no_templateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitClass_no_template(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Class_no_templateContext* PrismParser::class_no_template() {
  Class_no_templateContext *_localctx = _tracker.createInstance<Class_no_templateContext>(_ctx, getState());
  enterRule(_localctx, 50, PrismParser::RuleClass_no_template);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(381);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_with_templateContext ------------------------------------------------------------------

PrismParser::Type_with_templateContext::Type_with_templateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Class_no_templateContext* PrismParser::Type_with_templateContext::class_no_template() {
  return getRuleContext<PrismParser::Class_no_templateContext>(0);
}

tree::TerminalNode* PrismParser::Type_with_templateContext::LT() {
  return getToken(PrismParser::LT, 0);
}

tree::TerminalNode* PrismParser::Type_with_templateContext::GT() {
  return getToken(PrismParser::GT, 0);
}

PrismParser::PointerContext* PrismParser::Type_with_templateContext::pointer() {
  return getRuleContext<PrismParser::PointerContext>(0);
}

std::vector<PrismParser::Template_idContext *> PrismParser::Type_with_templateContext::template_id() {
  return getRuleContexts<PrismParser::Template_idContext>();
}

PrismParser::Template_idContext* PrismParser::Type_with_templateContext::template_id(size_t i) {
  return getRuleContext<PrismParser::Template_idContext>(i);
}


size_t PrismParser::Type_with_templateContext::getRuleIndex() const {
  return PrismParser::RuleType_with_template;
}

void PrismParser::Type_with_templateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_with_template(this);
}

void PrismParser::Type_with_templateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_with_template(this);
}


std::any PrismParser::Type_with_templateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitType_with_template(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Type_with_templateContext* PrismParser::type_with_template() {
  Type_with_templateContext *_localctx = _tracker.createInstance<Type_with_templateContext>(_ctx, getState());
  enterRule(_localctx, 52, PrismParser::RuleType_with_template);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(383);
    class_no_template();
    setState(392);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::LT) {
      setState(384);
      match(PrismParser::LT);
      setState(388);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == PrismParser::ID) {
        setState(385);
        template_id();
        setState(390);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(391);
      match(PrismParser::GT);
    }
    setState(395);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::POINTER) {
      setState(394);
      pointer();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Inherit_idContext ------------------------------------------------------------------

PrismParser::Inherit_idContext::Inherit_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Inherit_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Inherit_idContext::getRuleIndex() const {
  return PrismParser::RuleInherit_id;
}

void PrismParser::Inherit_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit_id(this);
}

void PrismParser::Inherit_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit_id(this);
}


std::any PrismParser::Inherit_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitInherit_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Inherit_idContext* PrismParser::inherit_id() {
  Inherit_idContext *_localctx = _tracker.createInstance<Inherit_idContext>(_ctx, getState());
  enterRule(_localctx, 54, PrismParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(397);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Name_idContext ------------------------------------------------------------------

PrismParser::Name_idContext::Name_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Name_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Name_idContext::getRuleIndex() const {
  return PrismParser::RuleName_id;
}

void PrismParser::Name_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterName_id(this);
}

void PrismParser::Name_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitName_id(this);
}


std::any PrismParser::Name_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitName_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Name_idContext* PrismParser::name_id() {
  Name_idContext *_localctx = _tracker.createInstance<Name_idContext>(_ctx, getState());
  enterRule(_localctx, 56, PrismParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(399);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Option_idContext ------------------------------------------------------------------

PrismParser::Option_idContext::Option_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Option_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Option_idContext::getRuleIndex() const {
  return PrismParser::RuleOption_id;
}

void PrismParser::Option_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_id(this);
}

void PrismParser::Option_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_id(this);
}


std::any PrismParser::Option_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitOption_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Option_idContext* PrismParser::option_id() {
  Option_idContext *_localctx = _tracker.createInstance<Option_idContext>(_ctx, getState());
  enterRule(_localctx, 58, PrismParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(401);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Owner_idContext ------------------------------------------------------------------

PrismParser::Owner_idContext::Owner_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Owner_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Owner_idContext::getRuleIndex() const {
  return PrismParser::RuleOwner_id;
}

void PrismParser::Owner_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOwner_id(this);
}

void PrismParser::Owner_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOwner_id(this);
}


std::any PrismParser::Owner_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitOwner_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Owner_idContext* PrismParser::owner_id() {
  Owner_idContext *_localctx = _tracker.createInstance<Owner_idContext>(_ctx, getState());
  enterRule(_localctx, 60, PrismParser::RuleOwner_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(403);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Template_idContext ------------------------------------------------------------------

PrismParser::Template_idContext::Template_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Template_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Template_idContext::getRuleIndex() const {
  return PrismParser::RuleTemplate_id;
}

void PrismParser::Template_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTemplate_id(this);
}

void PrismParser::Template_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTemplate_id(this);
}


std::any PrismParser::Template_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitTemplate_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Template_idContext* PrismParser::template_id() {
  Template_idContext *_localctx = _tracker.createInstance<Template_idContext>(_ctx, getState());
  enterRule(_localctx, 62, PrismParser::RuleTemplate_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(405);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_idContext ------------------------------------------------------------------

PrismParser::Function_idContext::Function_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Function_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}

tree::TerminalNode* PrismParser::Function_idContext::OPAR() {
  return getToken(PrismParser::OPAR, 0);
}

tree::TerminalNode* PrismParser::Function_idContext::CPAR() {
  return getToken(PrismParser::CPAR, 0);
}

std::vector<PrismParser::Value_id_ignoreContext *> PrismParser::Function_idContext::value_id_ignore() {
  return getRuleContexts<PrismParser::Value_id_ignoreContext>();
}

PrismParser::Value_id_ignoreContext* PrismParser::Function_idContext::value_id_ignore(size_t i) {
  return getRuleContext<PrismParser::Value_id_ignoreContext>(i);
}


size_t PrismParser::Function_idContext::getRuleIndex() const {
  return PrismParser::RuleFunction_id;
}

void PrismParser::Function_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction_id(this);
}

void PrismParser::Function_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction_id(this);
}


std::any PrismParser::Function_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitFunction_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Function_idContext* PrismParser::function_id() {
  Function_idContext *_localctx = _tracker.createInstance<Function_idContext>(_ctx, getState());
  enterRule(_localctx, 64, PrismParser::RuleFunction_id);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(407);
    match(PrismParser::ID);
    setState(408);
    match(PrismParser::OPAR);
    setState(410);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la - 70) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 70)) & 29360131) != 0) {
      setState(409);
      value_id_ignore();
    }
    setState(416);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(412);
      match(PrismParser::T__2);
      setState(413);
      value_id_ignore();
      setState(418);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(419);
    match(PrismParser::CPAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_idContext ------------------------------------------------------------------

PrismParser::Value_idContext::Value_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Shader_typeContext* PrismParser::Value_idContext::shader_type() {
  return getRuleContext<PrismParser::Shader_typeContext>(0);
}

tree::TerminalNode* PrismParser::Value_idContext::ID() {
  return getToken(PrismParser::ID, 0);
}

tree::TerminalNode* PrismParser::Value_idContext::INT_SCALAR() {
  return getToken(PrismParser::INT_SCALAR, 0);
}

tree::TerminalNode* PrismParser::Value_idContext::FLOAT_SCALAR() {
  return getToken(PrismParser::FLOAT_SCALAR, 0);
}

PrismParser::Bool_typeContext* PrismParser::Value_idContext::bool_type() {
  return getRuleContext<PrismParser::Bool_typeContext>(0);
}

PrismParser::Function_idContext* PrismParser::Value_idContext::function_id() {
  return getRuleContext<PrismParser::Function_idContext>(0);
}

PrismParser::Array_value_idsContext* PrismParser::Value_idContext::array_value_ids() {
  return getRuleContext<PrismParser::Array_value_idsContext>(0);
}


size_t PrismParser::Value_idContext::getRuleIndex() const {
  return PrismParser::RuleValue_id;
}

void PrismParser::Value_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_id(this);
}

void PrismParser::Value_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_id(this);
}


std::any PrismParser::Value_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitValue_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Value_idContext* PrismParser::value_id() {
  Value_idContext *_localctx = _tracker.createInstance<Value_idContext>(_ctx, getState());
  enterRule(_localctx, 66, PrismParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(428);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(421);
      shader_type();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(422);
      match(PrismParser::ID);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(423);
      match(PrismParser::INT_SCALAR);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(424);
      match(PrismParser::FLOAT_SCALAR);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(425);
      bool_type();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(426);
      function_id();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(427);
      array_value_ids();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_id_ignoreContext ------------------------------------------------------------------

PrismParser::Value_id_ignoreContext::Value_id_ignoreContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Value_id_ignoreContext::ID() {
  return getToken(PrismParser::ID, 0);
}

tree::TerminalNode* PrismParser::Value_id_ignoreContext::INT_SCALAR() {
  return getToken(PrismParser::INT_SCALAR, 0);
}

tree::TerminalNode* PrismParser::Value_id_ignoreContext::FLOAT_SCALAR() {
  return getToken(PrismParser::FLOAT_SCALAR, 0);
}

PrismParser::Bool_typeContext* PrismParser::Value_id_ignoreContext::bool_type() {
  return getRuleContext<PrismParser::Bool_typeContext>(0);
}


size_t PrismParser::Value_id_ignoreContext::getRuleIndex() const {
  return PrismParser::RuleValue_id_ignore;
}

void PrismParser::Value_id_ignoreContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_id_ignore(this);
}

void PrismParser::Value_id_ignoreContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_id_ignore(this);
}


std::any PrismParser::Value_id_ignoreContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitValue_id_ignore(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Value_id_ignoreContext* PrismParser::value_id_ignore() {
  Value_id_ignoreContext *_localctx = _tracker.createInstance<Value_id_ignoreContext>(_ctx, getState());
  enterRule(_localctx, 68, PrismParser::RuleValue_id_ignore);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(434);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(430);
        match(PrismParser::ID);
        break;
      }

      case PrismParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(431);
        match(PrismParser::INT_SCALAR);
        break;
      }

      case PrismParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(432);
        match(PrismParser::FLOAT_SCALAR);
        break;
      }

      case PrismParser::TRUE:
      case PrismParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(433);
        bool_type();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_idContext ------------------------------------------------------------------

PrismParser::Type_idContext::Type_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Type_with_templateContext* PrismParser::Type_idContext::type_with_template() {
  return getRuleContext<PrismParser::Type_with_templateContext>(0);
}


size_t PrismParser::Type_idContext::getRuleIndex() const {
  return PrismParser::RuleType_id;
}

void PrismParser::Type_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_id(this);
}

void PrismParser::Type_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_id(this);
}


std::any PrismParser::Type_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitType_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Type_idContext* PrismParser::type_id() {
  Type_idContext *_localctx = _tracker.createInstance<Type_idContext>(_ctx, getState());
  enterRule(_localctx, 70, PrismParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(436);
    type_with_template();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Insert_blockContext ------------------------------------------------------------------

PrismParser::Insert_blockContext::Insert_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Insert_blockContext::INSERT_BLOCK() {
  return getToken(PrismParser::INSERT_BLOCK, 0);
}


size_t PrismParser::Insert_blockContext::getRuleIndex() const {
  return PrismParser::RuleInsert_block;
}

void PrismParser::Insert_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInsert_block(this);
}

void PrismParser::Insert_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInsert_block(this);
}


std::any PrismParser::Insert_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitInsert_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Insert_blockContext* PrismParser::insert_block() {
  Insert_blockContext *_localctx = _tracker.createInstance<Insert_blockContext>(_ctx, getState());
  enterRule(_localctx, 72, PrismParser::RuleInsert_block);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(438);
    match(PrismParser::INSERT_BLOCK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Shader_pathContext ------------------------------------------------------------------

PrismParser::Shader_pathContext::Shader_pathContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Shader_pathContext::STRING() {
  return getToken(PrismParser::STRING, 0);
}

tree::TerminalNode* PrismParser::Shader_pathContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Shader_pathContext::getRuleIndex() const {
  return PrismParser::RuleShader_path;
}

void PrismParser::Shader_pathContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader_path(this);
}

void PrismParser::Shader_pathContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader_path(this);
}


std::any PrismParser::Shader_pathContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitShader_path(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Shader_pathContext* PrismParser::shader_path() {
  Shader_pathContext *_localctx = _tracker.createInstance<Shader_pathContext>(_ctx, getState());
  enterRule(_localctx, 74, PrismParser::RuleShader_path);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(440);
    _la = _input->LA(1);
    if (!(_la == PrismParser::ID

    || _la == PrismParser::STRING)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InheritContext ------------------------------------------------------------------

PrismParser::InheritContext::InheritContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::InheritContext::COLON() {
  return getToken(PrismParser::COLON, 0);
}

std::vector<PrismParser::Inherit_idContext *> PrismParser::InheritContext::inherit_id() {
  return getRuleContexts<PrismParser::Inherit_idContext>();
}

PrismParser::Inherit_idContext* PrismParser::InheritContext::inherit_id(size_t i) {
  return getRuleContext<PrismParser::Inherit_idContext>(i);
}


size_t PrismParser::InheritContext::getRuleIndex() const {
  return PrismParser::RuleInherit;
}

void PrismParser::InheritContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit(this);
}

void PrismParser::InheritContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit(this);
}


std::any PrismParser::InheritContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitInherit(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::InheritContext* PrismParser::inherit() {
  InheritContext *_localctx = _tracker.createInstance<InheritContext>(_ctx, getState());
  enterRule(_localctx, 76, PrismParser::RuleInherit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(442);
    match(PrismParser::COLON);
    setState(443);
    inherit_id();
    setState(448);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(444);
        match(PrismParser::T__2);
        setState(445);
        inherit_id(); 
      }
      setState(450);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_statContext ------------------------------------------------------------------

PrismParser::Layout_statContext::Layout_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Slot_declarationContext* PrismParser::Layout_statContext::slot_declaration() {
  return getRuleContext<PrismParser::Slot_declarationContext>(0);
}

PrismParser::Sampler_declarationContext* PrismParser::Layout_statContext::sampler_declaration() {
  return getRuleContext<PrismParser::Sampler_declarationContext>(0);
}

tree::TerminalNode* PrismParser::Layout_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Layout_statContext::getRuleIndex() const {
  return PrismParser::RuleLayout_stat;
}

void PrismParser::Layout_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_stat(this);
}

void PrismParser::Layout_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_stat(this);
}


std::any PrismParser::Layout_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitLayout_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Layout_statContext* PrismParser::layout_stat() {
  Layout_statContext *_localctx = _tracker.createInstance<Layout_statContext>(_ctx, getState());
  enterRule(_localctx, 78, PrismParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(454);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(451);
        slot_declaration();
        break;
      }

      case PrismParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(452);
        sampler_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(453);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_blockContext ------------------------------------------------------------------

PrismParser::Layout_blockContext::Layout_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Layout_statContext *> PrismParser::Layout_blockContext::layout_stat() {
  return getRuleContexts<PrismParser::Layout_statContext>();
}

PrismParser::Layout_statContext* PrismParser::Layout_blockContext::layout_stat(size_t i) {
  return getRuleContext<PrismParser::Layout_statContext>(i);
}


size_t PrismParser::Layout_blockContext::getRuleIndex() const {
  return PrismParser::RuleLayout_block;
}

void PrismParser::Layout_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_block(this);
}

void PrismParser::Layout_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_block(this);
}


std::any PrismParser::Layout_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitLayout_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Layout_blockContext* PrismParser::layout_block() {
  Layout_blockContext *_localctx = _tracker.createInstance<Layout_blockContext>(_ctx, getState());
  enterRule(_localctx, 80, PrismParser::RuleLayout_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(459);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__3 || _la == PrismParser::SLOT

    || _la == PrismParser::COMMENT) {
      setState(456);
      layout_stat();
      setState(461);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_definitionContext ------------------------------------------------------------------

PrismParser::Layout_definitionContext::Layout_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Layout_definitionContext::LAYOUT() {
  return getToken(PrismParser::LAYOUT, 0);
}

PrismParser::Name_idContext* PrismParser::Layout_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Layout_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Layout_blockContext* PrismParser::Layout_definitionContext::layout_block() {
  return getRuleContext<PrismParser::Layout_blockContext>(0);
}

tree::TerminalNode* PrismParser::Layout_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

PrismParser::InheritContext* PrismParser::Layout_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Layout_definitionContext::getRuleIndex() const {
  return PrismParser::RuleLayout_definition;
}

void PrismParser::Layout_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_definition(this);
}

void PrismParser::Layout_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_definition(this);
}


std::any PrismParser::Layout_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitLayout_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Layout_definitionContext* PrismParser::layout_definition() {
  Layout_definitionContext *_localctx = _tracker.createInstance<Layout_definitionContext>(_ctx, getState());
  enterRule(_localctx, 82, PrismParser::RuleLayout_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(462);
    match(PrismParser::LAYOUT);
    setState(463);
    name_id();
    setState(465);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(464);
      inherit();
    }
    setState(467);
    match(PrismParser::OBRACE);
    setState(468);
    layout_block();
    setState(469);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_statContext ------------------------------------------------------------------

PrismParser::Table_statContext::Table_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Value_declarationContext* PrismParser::Table_statContext::value_declaration() {
  return getRuleContext<PrismParser::Value_declarationContext>(0);
}

PrismParser::Function_definitionContext* PrismParser::Table_statContext::function_definition() {
  return getRuleContext<PrismParser::Function_definitionContext>(0);
}

PrismParser::Insert_blockContext* PrismParser::Table_statContext::insert_block() {
  return getRuleContext<PrismParser::Insert_blockContext>(0);
}

tree::TerminalNode* PrismParser::Table_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Table_statContext::getRuleIndex() const {
  return PrismParser::RuleTable_stat;
}

void PrismParser::Table_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_stat(this);
}

void PrismParser::Table_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_stat(this);
}


std::any PrismParser::Table_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitTable_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Table_statContext* PrismParser::table_stat() {
  Table_statContext *_localctx = _tracker.createInstance<Table_statContext>(_ctx, getState());
  enterRule(_localctx, 84, PrismParser::RuleTable_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(475);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(471);
      value_declaration();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(472);
      function_definition();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(473);
      insert_block();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(474);
      match(PrismParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_definitionContext ------------------------------------------------------------------

PrismParser::Function_definitionContext::Function_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Type_idContext* PrismParser::Function_definitionContext::type_id() {
  return getRuleContext<PrismParser::Type_idContext>(0);
}

PrismParser::Name_idContext* PrismParser::Function_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Function_definitionContext::OPAR() {
  return getToken(PrismParser::OPAR, 0);
}

PrismParser::Function_paramsContext* PrismParser::Function_definitionContext::function_params() {
  return getRuleContext<PrismParser::Function_paramsContext>(0);
}

tree::TerminalNode* PrismParser::Function_definitionContext::CPAR() {
  return getToken(PrismParser::CPAR, 0);
}

tree::TerminalNode* PrismParser::Function_definitionContext::FUNC_BODY() {
  return getToken(PrismParser::FUNC_BODY, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Function_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Function_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::Function_semanticContext* PrismParser::Function_definitionContext::function_semantic() {
  return getRuleContext<PrismParser::Function_semanticContext>(0);
}


size_t PrismParser::Function_definitionContext::getRuleIndex() const {
  return PrismParser::RuleFunction_definition;
}

void PrismParser::Function_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction_definition(this);
}

void PrismParser::Function_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction_definition(this);
}


std::any PrismParser::Function_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitFunction_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Function_definitionContext* PrismParser::function_definition() {
  Function_definitionContext *_localctx = _tracker.createInstance<Function_definitionContext>(_ctx, getState());
  enterRule(_localctx, 86, PrismParser::RuleFunction_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(480);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(477);
        option_block(); 
      }
      setState(482);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
    setState(483);
    type_id();
    setState(484);
    name_id();
    setState(485);
    match(PrismParser::OPAR);
    setState(486);
    function_params();
    setState(487);
    match(PrismParser::CPAR);
    setState(489);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(488);
      function_semantic();
    }
    setState(491);
    match(PrismParser::FUNC_BODY);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_paramsContext ------------------------------------------------------------------

PrismParser::Function_paramsContext::Function_paramsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> PrismParser::Function_paramsContext::OPAR() {
  return getTokens(PrismParser::OPAR);
}

tree::TerminalNode* PrismParser::Function_paramsContext::OPAR(size_t i) {
  return getToken(PrismParser::OPAR, i);
}

std::vector<PrismParser::Function_paramsContext *> PrismParser::Function_paramsContext::function_params() {
  return getRuleContexts<PrismParser::Function_paramsContext>();
}

PrismParser::Function_paramsContext* PrismParser::Function_paramsContext::function_params(size_t i) {
  return getRuleContext<PrismParser::Function_paramsContext>(i);
}

std::vector<tree::TerminalNode *> PrismParser::Function_paramsContext::CPAR() {
  return getTokens(PrismParser::CPAR);
}

tree::TerminalNode* PrismParser::Function_paramsContext::CPAR(size_t i) {
  return getToken(PrismParser::CPAR, i);
}


size_t PrismParser::Function_paramsContext::getRuleIndex() const {
  return PrismParser::RuleFunction_params;
}

void PrismParser::Function_paramsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction_params(this);
}

void PrismParser::Function_paramsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction_params(this);
}


std::any PrismParser::Function_paramsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitFunction_params(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Function_paramsContext* PrismParser::function_params() {
  Function_paramsContext *_localctx = _tracker.createInstance<Function_paramsContext>(_ctx, getState());
  enterRule(_localctx, 88, PrismParser::RuleFunction_params);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(500);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & -2) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 1099511627773) != 0) {
      setState(498);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case PrismParser::OPAR: {
          setState(493);
          match(PrismParser::OPAR);
          setState(494);
          function_params();
          setState(495);
          match(PrismParser::CPAR);
          break;
        }

        case PrismParser::T__0:
        case PrismParser::T__1:
        case PrismParser::T__2:
        case PrismParser::T__3:
        case PrismParser::T__4:
        case PrismParser::T__5:
        case PrismParser::T__6:
        case PrismParser::T__7:
        case PrismParser::T__8:
        case PrismParser::T__9:
        case PrismParser::T__10:
        case PrismParser::T__11:
        case PrismParser::T__12:
        case PrismParser::T__13:
        case PrismParser::T__14:
        case PrismParser::T__15:
        case PrismParser::T__16:
        case PrismParser::T__17:
        case PrismParser::T__18:
        case PrismParser::T__19:
        case PrismParser::T__20:
        case PrismParser::T__21:
        case PrismParser::T__22:
        case PrismParser::T__23:
        case PrismParser::T__24:
        case PrismParser::T__25:
        case PrismParser::T__26:
        case PrismParser::T__27:
        case PrismParser::T__28:
        case PrismParser::T__29:
        case PrismParser::T__30:
        case PrismParser::T__31:
        case PrismParser::T__32:
        case PrismParser::T__33:
        case PrismParser::T__34:
        case PrismParser::T__35:
        case PrismParser::T__36:
        case PrismParser::T__37:
        case PrismParser::T__38:
        case PrismParser::T__39:
        case PrismParser::T__40:
        case PrismParser::T__41:
        case PrismParser::T__42:
        case PrismParser::T__43:
        case PrismParser::OR:
        case PrismParser::AND:
        case PrismParser::PIPE:
        case PrismParser::EQ:
        case PrismParser::NEQ:
        case PrismParser::GT:
        case PrismParser::LT:
        case PrismParser::GTEQ:
        case PrismParser::LTEQ:
        case PrismParser::PLUS:
        case PrismParser::MINUS:
        case PrismParser::DIV:
        case PrismParser::MOD:
        case PrismParser::POW:
        case PrismParser::NOT:
        case PrismParser::SCOL:
        case PrismParser::COLON:
        case PrismParser::DOT:
        case PrismParser::ASSIGN:
        case PrismParser::OBRACE:
        case PrismParser::CBRACE:
        case PrismParser::OSBRACE:
        case PrismParser::CSBRACE:
        case PrismParser::TRUE:
        case PrismParser::FALSE:
        case PrismParser::LOG:
        case PrismParser::LAYOUT:
        case PrismParser::STRUCT:
        case PrismParser::COMPUTE_PSO:
        case PrismParser::GRAPHICS_PSO:
        case PrismParser::RAYTRACE_PSO:
        case PrismParser::WORKGRAPH_PSO:
        case PrismParser::NODE:
        case PrismParser::NODE_OUTPUT:
        case PrismParser::RAYTRACE_RAYGEN:
        case PrismParser::RAYTRACE_PASS:
        case PrismParser::PASS:
        case PrismParser::VIEW:
        case PrismParser::PIPELINE:
        case PrismParser::SLOT:
        case PrismParser::RT:
        case PrismParser::RTV:
        case PrismParser::DSV:
        case PrismParser::ROOTSIG:
        case PrismParser::ENUM:
        case PrismParser::ID:
        case PrismParser::INT_SCALAR:
        case PrismParser::FLOAT_SCALAR:
        case PrismParser::STRING:
        case PrismParser::RAWEXPR:
        case PrismParser::COMMENT:
        case PrismParser::SPACE:
        case PrismParser::POINTER:
        case PrismParser::FUNC_BODY:
        case PrismParser::INSERT_START:
        case PrismParser::INSERT_END:
        case PrismParser::INSERT_BLOCK: {
          setState(497);
          _la = _input->LA(1);
          if (_la == 0 || _la == Token::EOF || (_la == PrismParser::OPAR

          || _la == PrismParser::CPAR)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(502);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_semanticContext ------------------------------------------------------------------

PrismParser::Function_semanticContext::Function_semanticContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Function_semanticContext::COLON() {
  return getToken(PrismParser::COLON, 0);
}

tree::TerminalNode* PrismParser::Function_semanticContext::ID() {
  return getToken(PrismParser::ID, 0);
}


size_t PrismParser::Function_semanticContext::getRuleIndex() const {
  return PrismParser::RuleFunction_semantic;
}

void PrismParser::Function_semanticContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction_semantic(this);
}

void PrismParser::Function_semanticContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction_semantic(this);
}


std::any PrismParser::Function_semanticContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitFunction_semantic(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Function_semanticContext* PrismParser::function_semantic() {
  Function_semanticContext *_localctx = _tracker.createInstance<Function_semanticContext>(_ctx, getState());
  enterRule(_localctx, 90, PrismParser::RuleFunction_semantic);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(503);
    match(PrismParser::COLON);
    setState(504);
    match(PrismParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_blockContext ------------------------------------------------------------------

PrismParser::Table_blockContext::Table_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Table_statContext *> PrismParser::Table_blockContext::table_stat() {
  return getRuleContexts<PrismParser::Table_statContext>();
}

PrismParser::Table_statContext* PrismParser::Table_blockContext::table_stat(size_t i) {
  return getRuleContext<PrismParser::Table_statContext>(i);
}


size_t PrismParser::Table_blockContext::getRuleIndex() const {
  return PrismParser::RuleTable_block;
}

void PrismParser::Table_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_block(this);
}

void PrismParser::Table_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_block(this);
}


std::any PrismParser::Table_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitTable_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Table_blockContext* PrismParser::table_block() {
  Table_blockContext *_localctx = _tracker.createInstance<Table_blockContext>(_ctx, getState());
  enterRule(_localctx, 92, PrismParser::RuleTable_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(509);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 34913386497) != 0) {
      setState(506);
      table_stat();
      setState(511);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_definitionContext ------------------------------------------------------------------

PrismParser::Table_definitionContext::Table_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Table_definitionContext::STRUCT() {
  return getToken(PrismParser::STRUCT, 0);
}

PrismParser::Name_idContext* PrismParser::Table_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Table_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Table_blockContext* PrismParser::Table_definitionContext::table_block() {
  return getRuleContext<PrismParser::Table_blockContext>(0);
}

tree::TerminalNode* PrismParser::Table_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Table_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Table_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Table_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Table_definitionContext::getRuleIndex() const {
  return PrismParser::RuleTable_definition;
}

void PrismParser::Table_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_definition(this);
}

void PrismParser::Table_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_definition(this);
}


std::any PrismParser::Table_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitTable_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Table_definitionContext* PrismParser::table_definition() {
  Table_definitionContext *_localctx = _tracker.createInstance<Table_definitionContext>(_ctx, getState());
  enterRule(_localctx, 94, PrismParser::RuleTable_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(515);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(512);
        option_block(); 
      }
      setState(517);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
    setState(518);
    match(PrismParser::STRUCT);
    setState(519);
    name_id();
    setState(521);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(520);
      inherit();
    }
    setState(523);
    match(PrismParser::OBRACE);
    setState(524);
    table_block();
    setState(525);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_color_declarationContext ------------------------------------------------------------------

PrismParser::Rt_color_declarationContext::Rt_color_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Type_idContext* PrismParser::Rt_color_declarationContext::type_id() {
  return getRuleContext<PrismParser::Type_idContext>(0);
}

PrismParser::Name_idContext* PrismParser::Rt_color_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rt_color_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Rt_color_declarationContext::getRuleIndex() const {
  return PrismParser::RuleRt_color_declaration;
}

void PrismParser::Rt_color_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_color_declaration(this);
}

void PrismParser::Rt_color_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_color_declaration(this);
}


std::any PrismParser::Rt_color_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRt_color_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rt_color_declarationContext* PrismParser::rt_color_declaration() {
  Rt_color_declarationContext *_localctx = _tracker.createInstance<Rt_color_declarationContext>(_ctx, getState());
  enterRule(_localctx, 96, PrismParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(527);
    type_id();
    setState(528);
    name_id();
    setState(529);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_ds_declarationContext ------------------------------------------------------------------

PrismParser::Rt_ds_declarationContext::Rt_ds_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rt_ds_declarationContext::DSV() {
  return getToken(PrismParser::DSV, 0);
}

PrismParser::Name_idContext* PrismParser::Rt_ds_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rt_ds_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Rt_ds_declarationContext::getRuleIndex() const {
  return PrismParser::RuleRt_ds_declaration;
}

void PrismParser::Rt_ds_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_ds_declaration(this);
}

void PrismParser::Rt_ds_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_ds_declaration(this);
}


std::any PrismParser::Rt_ds_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRt_ds_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rt_ds_declarationContext* PrismParser::rt_ds_declaration() {
  Rt_ds_declarationContext *_localctx = _tracker.createInstance<Rt_ds_declarationContext>(_ctx, getState());
  enterRule(_localctx, 98, PrismParser::RuleRt_ds_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(531);
    match(PrismParser::DSV);
    setState(532);
    name_id();
    setState(533);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_statContext ------------------------------------------------------------------

PrismParser::Rt_statContext::Rt_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Rt_color_declarationContext* PrismParser::Rt_statContext::rt_color_declaration() {
  return getRuleContext<PrismParser::Rt_color_declarationContext>(0);
}

PrismParser::Rt_ds_declarationContext* PrismParser::Rt_statContext::rt_ds_declaration() {
  return getRuleContext<PrismParser::Rt_ds_declarationContext>(0);
}

tree::TerminalNode* PrismParser::Rt_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Rt_statContext::getRuleIndex() const {
  return PrismParser::RuleRt_stat;
}

void PrismParser::Rt_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_stat(this);
}

void PrismParser::Rt_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_stat(this);
}


std::any PrismParser::Rt_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRt_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rt_statContext* PrismParser::rt_stat() {
  Rt_statContext *_localctx = _tracker.createInstance<Rt_statContext>(_ctx, getState());
  enterRule(_localctx, 100, PrismParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(538);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(535);
        rt_color_declaration();
        break;
      }

      case PrismParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(536);
        rt_ds_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(537);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_blockContext ------------------------------------------------------------------

PrismParser::Rt_blockContext::Rt_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Rt_statContext *> PrismParser::Rt_blockContext::rt_stat() {
  return getRuleContexts<PrismParser::Rt_statContext>();
}

PrismParser::Rt_statContext* PrismParser::Rt_blockContext::rt_stat(size_t i) {
  return getRuleContext<PrismParser::Rt_statContext>(i);
}


size_t PrismParser::Rt_blockContext::getRuleIndex() const {
  return PrismParser::RuleRt_block;
}

void PrismParser::Rt_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_block(this);
}

void PrismParser::Rt_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_block(this);
}


std::any PrismParser::Rt_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRt_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rt_blockContext* PrismParser::rt_block() {
  Rt_blockContext *_localctx = _tracker.createInstance<Rt_blockContext>(_ctx, getState());
  enterRule(_localctx, 102, PrismParser::RuleRt_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(543);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & 265) != 0) {
      setState(540);
      rt_stat();
      setState(545);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_definitionContext ------------------------------------------------------------------

PrismParser::Rt_definitionContext::Rt_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rt_definitionContext::RT() {
  return getToken(PrismParser::RT, 0);
}

PrismParser::Name_idContext* PrismParser::Rt_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rt_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Rt_blockContext* PrismParser::Rt_definitionContext::rt_block() {
  return getRuleContext<PrismParser::Rt_blockContext>(0);
}

tree::TerminalNode* PrismParser::Rt_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}


size_t PrismParser::Rt_definitionContext::getRuleIndex() const {
  return PrismParser::RuleRt_definition;
}

void PrismParser::Rt_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_definition(this);
}

void PrismParser::Rt_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_definition(this);
}


std::any PrismParser::Rt_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRt_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rt_definitionContext* PrismParser::rt_definition() {
  Rt_definitionContext *_localctx = _tracker.createInstance<Rt_definitionContext>(_ctx, getState());
  enterRule(_localctx, 104, PrismParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(546);
    match(PrismParser::RT);
    setState(547);
    name_id();
    setState(548);
    match(PrismParser::OBRACE);
    setState(549);
    rt_block();
    setState(550);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_value_holderContext ------------------------------------------------------------------

PrismParser::Array_value_holderContext::Array_value_holderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Value_idContext* PrismParser::Array_value_holderContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}


size_t PrismParser::Array_value_holderContext::getRuleIndex() const {
  return PrismParser::RuleArray_value_holder;
}

void PrismParser::Array_value_holderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_holder(this);
}

void PrismParser::Array_value_holderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_holder(this);
}


std::any PrismParser::Array_value_holderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitArray_value_holder(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Array_value_holderContext* PrismParser::array_value_holder() {
  Array_value_holderContext *_localctx = _tracker.createInstance<Array_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 106, PrismParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(552);
    value_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_value_idsContext ------------------------------------------------------------------

PrismParser::Array_value_idsContext::Array_value_idsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Array_value_idsContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

std::vector<PrismParser::Array_value_holderContext *> PrismParser::Array_value_idsContext::array_value_holder() {
  return getRuleContexts<PrismParser::Array_value_holderContext>();
}

PrismParser::Array_value_holderContext* PrismParser::Array_value_idsContext::array_value_holder(size_t i) {
  return getRuleContext<PrismParser::Array_value_holderContext>(i);
}

tree::TerminalNode* PrismParser::Array_value_idsContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}


size_t PrismParser::Array_value_idsContext::getRuleIndex() const {
  return PrismParser::RuleArray_value_ids;
}

void PrismParser::Array_value_idsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_ids(this);
}

void PrismParser::Array_value_idsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_ids(this);
}


std::any PrismParser::Array_value_idsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitArray_value_ids(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Array_value_idsContext* PrismParser::array_value_ids() {
  Array_value_idsContext *_localctx = _tracker.createInstance<Array_value_idsContext>(_ctx, getState());
  enterRule(_localctx, 108, PrismParser::RuleArray_value_ids);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(554);
    match(PrismParser::OBRACE);
    setState(555);
    array_value_holder();
    setState(560);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(556);
      match(PrismParser::T__2);
      setState(557);
      array_value_holder();
      setState(562);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(563);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Root_sigContext ------------------------------------------------------------------

PrismParser::Root_sigContext::Root_sigContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Root_sigContext::ROOTSIG() {
  return getToken(PrismParser::ROOTSIG, 0);
}

tree::TerminalNode* PrismParser::Root_sigContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Name_idContext* PrismParser::Root_sigContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Root_sigContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Root_sigContext::getRuleIndex() const {
  return PrismParser::RuleRoot_sig;
}

void PrismParser::Root_sigContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRoot_sig(this);
}

void PrismParser::Root_sigContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRoot_sig(this);
}


std::any PrismParser::Root_sigContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRoot_sig(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Root_sigContext* PrismParser::root_sig() {
  Root_sigContext *_localctx = _tracker.createInstance<Root_sigContext>(_ctx, getState());
  enterRule(_localctx, 110, PrismParser::RuleRoot_sig);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(565);
    match(PrismParser::ROOTSIG);
    setState(566);
    match(PrismParser::ASSIGN);
    setState(567);
    name_id();
    setState(568);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShaderContext ------------------------------------------------------------------

PrismParser::ShaderContext::ShaderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Shader_typeContext* PrismParser::ShaderContext::shader_type() {
  return getRuleContext<PrismParser::Shader_typeContext>(0);
}

tree::TerminalNode* PrismParser::ShaderContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Shader_pathContext* PrismParser::ShaderContext::shader_path() {
  return getRuleContext<PrismParser::Shader_pathContext>(0);
}

tree::TerminalNode* PrismParser::ShaderContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::ShaderContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::ShaderContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::ShaderContext::getRuleIndex() const {
  return PrismParser::RuleShader;
}

void PrismParser::ShaderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader(this);
}

void PrismParser::ShaderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader(this);
}


std::any PrismParser::ShaderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitShader(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::ShaderContext* PrismParser::shader() {
  ShaderContext *_localctx = _tracker.createInstance<ShaderContext>(_ctx, getState());
  enterRule(_localctx, 112, PrismParser::RuleShader);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(573);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(570);
        option_block(); 
      }
      setState(575);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    }
    setState(576);
    shader_type();
    setState(577);
    match(PrismParser::ASSIGN);
    setState(578);
    shader_path();
    setState(579);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_statContext ------------------------------------------------------------------

PrismParser::Compute_pso_statContext::Compute_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Root_sigContext* PrismParser::Compute_pso_statContext::root_sig() {
  return getRuleContext<PrismParser::Root_sigContext>(0);
}

PrismParser::ShaderContext* PrismParser::Compute_pso_statContext::shader() {
  return getRuleContext<PrismParser::ShaderContext>(0);
}

PrismParser::Define_declarationContext* PrismParser::Compute_pso_statContext::define_declaration() {
  return getRuleContext<PrismParser::Define_declarationContext>(0);
}

tree::TerminalNode* PrismParser::Compute_pso_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Compute_pso_statContext::getRuleIndex() const {
  return PrismParser::RuleCompute_pso_stat;
}

void PrismParser::Compute_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_stat(this);
}

void PrismParser::Compute_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_stat(this);
}


std::any PrismParser::Compute_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Compute_pso_statContext* PrismParser::compute_pso_stat() {
  Compute_pso_statContext *_localctx = _tracker.createInstance<Compute_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 114, PrismParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(585);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(581);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(582);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(583);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(584);
      match(PrismParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_blockContext ------------------------------------------------------------------

PrismParser::Compute_pso_blockContext::Compute_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Compute_pso_statContext *> PrismParser::Compute_pso_blockContext::compute_pso_stat() {
  return getRuleContexts<PrismParser::Compute_pso_statContext>();
}

PrismParser::Compute_pso_statContext* PrismParser::Compute_pso_blockContext::compute_pso_stat(size_t i) {
  return getRuleContext<PrismParser::Compute_pso_statContext>(i);
}


size_t PrismParser::Compute_pso_blockContext::getRuleIndex() const {
  return PrismParser::RuleCompute_pso_block;
}

void PrismParser::Compute_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_block(this);
}

void PrismParser::Compute_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_block(this);
}


std::any PrismParser::Compute_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Compute_pso_blockContext* PrismParser::compute_pso_block() {
  Compute_pso_blockContext *_localctx = _tracker.createInstance<Compute_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 116, PrismParser::RuleCompute_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(590);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(587);
      compute_pso_stat();
      setState(592);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_definitionContext ------------------------------------------------------------------

PrismParser::Compute_pso_definitionContext::Compute_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Compute_pso_definitionContext::COMPUTE_PSO() {
  return getToken(PrismParser::COMPUTE_PSO, 0);
}

PrismParser::Name_idContext* PrismParser::Compute_pso_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Compute_pso_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Compute_pso_blockContext* PrismParser::Compute_pso_definitionContext::compute_pso_block() {
  return getRuleContext<PrismParser::Compute_pso_blockContext>(0);
}

tree::TerminalNode* PrismParser::Compute_pso_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Compute_pso_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Compute_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Compute_pso_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Compute_pso_definitionContext::getRuleIndex() const {
  return PrismParser::RuleCompute_pso_definition;
}

void PrismParser::Compute_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_definition(this);
}

void PrismParser::Compute_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_definition(this);
}


std::any PrismParser::Compute_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Compute_pso_definitionContext* PrismParser::compute_pso_definition() {
  Compute_pso_definitionContext *_localctx = _tracker.createInstance<Compute_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 118, PrismParser::RuleCompute_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(596);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(593);
        option_block(); 
      }
      setState(598);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx);
    }
    setState(599);
    match(PrismParser::COMPUTE_PSO);
    setState(600);
    name_id();
    setState(602);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(601);
      inherit();
    }
    setState(604);
    match(PrismParser::OBRACE);
    setState(605);
    compute_pso_block();
    setState(606);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_statContext ------------------------------------------------------------------

PrismParser::Graphics_pso_statContext::Graphics_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Root_sigContext* PrismParser::Graphics_pso_statContext::root_sig() {
  return getRuleContext<PrismParser::Root_sigContext>(0);
}

PrismParser::ShaderContext* PrismParser::Graphics_pso_statContext::shader() {
  return getRuleContext<PrismParser::ShaderContext>(0);
}

PrismParser::Define_declarationContext* PrismParser::Graphics_pso_statContext::define_declaration() {
  return getRuleContext<PrismParser::Define_declarationContext>(0);
}

PrismParser::Rtv_formats_declarationContext* PrismParser::Graphics_pso_statContext::rtv_formats_declaration() {
  return getRuleContext<PrismParser::Rtv_formats_declarationContext>(0);
}

PrismParser::Blends_declarationContext* PrismParser::Graphics_pso_statContext::blends_declaration() {
  return getRuleContext<PrismParser::Blends_declarationContext>(0);
}

PrismParser::Pso_paramContext* PrismParser::Graphics_pso_statContext::pso_param() {
  return getRuleContext<PrismParser::Pso_paramContext>(0);
}

tree::TerminalNode* PrismParser::Graphics_pso_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Graphics_pso_statContext::getRuleIndex() const {
  return PrismParser::RuleGraphics_pso_stat;
}

void PrismParser::Graphics_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_stat(this);
}

void PrismParser::Graphics_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_stat(this);
}


std::any PrismParser::Graphics_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Graphics_pso_statContext* PrismParser::graphics_pso_stat() {
  Graphics_pso_statContext *_localctx = _tracker.createInstance<Graphics_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 120, PrismParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(615);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(608);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(609);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(610);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(611);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(612);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(613);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(614);
      match(PrismParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_blockContext ------------------------------------------------------------------

PrismParser::Graphics_pso_blockContext::Graphics_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Graphics_pso_statContext *> PrismParser::Graphics_pso_blockContext::graphics_pso_stat() {
  return getRuleContexts<PrismParser::Graphics_pso_statContext>();
}

PrismParser::Graphics_pso_statContext* PrismParser::Graphics_pso_blockContext::graphics_pso_stat(size_t i) {
  return getRuleContext<PrismParser::Graphics_pso_statContext>(i);
}


size_t PrismParser::Graphics_pso_blockContext::getRuleIndex() const {
  return PrismParser::RuleGraphics_pso_block;
}

void PrismParser::Graphics_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_block(this);
}

void PrismParser::Graphics_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_block(this);
}


std::any PrismParser::Graphics_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Graphics_pso_blockContext* PrismParser::graphics_pso_block() {
  Graphics_pso_blockContext *_localctx = _tracker.createInstance<Graphics_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 122, PrismParser::RuleGraphics_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(620);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080864) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(617);
      graphics_pso_stat();
      setState(622);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_definitionContext ------------------------------------------------------------------

PrismParser::Graphics_pso_definitionContext::Graphics_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Graphics_pso_definitionContext::GRAPHICS_PSO() {
  return getToken(PrismParser::GRAPHICS_PSO, 0);
}

PrismParser::Name_idContext* PrismParser::Graphics_pso_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Graphics_pso_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Graphics_pso_blockContext* PrismParser::Graphics_pso_definitionContext::graphics_pso_block() {
  return getRuleContext<PrismParser::Graphics_pso_blockContext>(0);
}

tree::TerminalNode* PrismParser::Graphics_pso_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Graphics_pso_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Graphics_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Graphics_pso_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Graphics_pso_definitionContext::getRuleIndex() const {
  return PrismParser::RuleGraphics_pso_definition;
}

void PrismParser::Graphics_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_definition(this);
}

void PrismParser::Graphics_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_definition(this);
}


std::any PrismParser::Graphics_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Graphics_pso_definitionContext* PrismParser::graphics_pso_definition() {
  Graphics_pso_definitionContext *_localctx = _tracker.createInstance<Graphics_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 124, PrismParser::RuleGraphics_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(626);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(623);
        option_block(); 
      }
      setState(628);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    }
    setState(629);
    match(PrismParser::GRAPHICS_PSO);
    setState(630);
    name_id();
    setState(632);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(631);
      inherit();
    }
    setState(634);
    match(PrismParser::OBRACE);
    setState(635);
    graphics_pso_block();
    setState(636);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_statContext ------------------------------------------------------------------

PrismParser::Rtx_pso_statContext::Rtx_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Root_sigContext* PrismParser::Rtx_pso_statContext::root_sig() {
  return getRuleContext<PrismParser::Root_sigContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pso_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Rtx_pso_statContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pso_stat;
}

void PrismParser::Rtx_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_stat(this);
}

void PrismParser::Rtx_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_stat(this);
}


std::any PrismParser::Rtx_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pso_statContext* PrismParser::rtx_pso_stat() {
  Rtx_pso_statContext *_localctx = _tracker.createInstance<Rtx_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 126, PrismParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(640);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(638);
        root_sig();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(639);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_blockContext ------------------------------------------------------------------

PrismParser::Rtx_pso_blockContext::Rtx_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Rtx_pso_statContext *> PrismParser::Rtx_pso_blockContext::rtx_pso_stat() {
  return getRuleContexts<PrismParser::Rtx_pso_statContext>();
}

PrismParser::Rtx_pso_statContext* PrismParser::Rtx_pso_blockContext::rtx_pso_stat(size_t i) {
  return getRuleContext<PrismParser::Rtx_pso_statContext>(i);
}


size_t PrismParser::Rtx_pso_blockContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pso_block;
}

void PrismParser::Rtx_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_block(this);
}

void PrismParser::Rtx_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_block(this);
}


std::any PrismParser::Rtx_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pso_blockContext* PrismParser::rtx_pso_block() {
  Rtx_pso_blockContext *_localctx = _tracker.createInstance<Rtx_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 128, PrismParser::RuleRtx_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(645);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ROOTSIG

    || _la == PrismParser::COMMENT) {
      setState(642);
      rtx_pso_stat();
      setState(647);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_definitionContext ------------------------------------------------------------------

PrismParser::Rtx_pso_definitionContext::Rtx_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rtx_pso_definitionContext::RAYTRACE_PSO() {
  return getToken(PrismParser::RAYTRACE_PSO, 0);
}

PrismParser::Name_idContext* PrismParser::Rtx_pso_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pso_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Rtx_pso_blockContext* PrismParser::Rtx_pso_definitionContext::rtx_pso_block() {
  return getRuleContext<PrismParser::Rtx_pso_blockContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pso_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

PrismParser::InheritContext* PrismParser::Rtx_pso_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Rtx_pso_definitionContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pso_definition;
}

void PrismParser::Rtx_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_definition(this);
}

void PrismParser::Rtx_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_definition(this);
}


std::any PrismParser::Rtx_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pso_definitionContext* PrismParser::rtx_pso_definition() {
  Rtx_pso_definitionContext *_localctx = _tracker.createInstance<Rtx_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 130, PrismParser::RuleRtx_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(648);
    match(PrismParser::RAYTRACE_PSO);
    setState(649);
    name_id();
    setState(651);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(650);
      inherit();
    }
    setState(653);
    match(PrismParser::OBRACE);
    setState(654);
    rtx_pso_block();
    setState(655);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_param_idContext ------------------------------------------------------------------

PrismParser::Node_param_idContext::Node_param_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PrismParser::Node_param_idContext::getRuleIndex() const {
  return PrismParser::RuleNode_param_id;
}

void PrismParser::Node_param_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_param_id(this);
}

void PrismParser::Node_param_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_param_id(this);
}


std::any PrismParser::Node_param_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_param_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_param_idContext* PrismParser::node_param_id() {
  Node_param_idContext *_localctx = _tracker.createInstance<Node_param_idContext>(_ctx, getState());
  enterRule(_localctx, 132, PrismParser::RuleNode_param_id);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(657);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7936) != 0)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_paramContext ------------------------------------------------------------------

PrismParser::Node_paramContext::Node_paramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Node_param_idContext* PrismParser::Node_paramContext::node_param_id() {
  return getRuleContext<PrismParser::Node_param_idContext>(0);
}

tree::TerminalNode* PrismParser::Node_paramContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Value_idContext* PrismParser::Node_paramContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}

tree::TerminalNode* PrismParser::Node_paramContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}


size_t PrismParser::Node_paramContext::getRuleIndex() const {
  return PrismParser::RuleNode_param;
}

void PrismParser::Node_paramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_param(this);
}

void PrismParser::Node_paramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_param(this);
}


std::any PrismParser::Node_paramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_param(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_paramContext* PrismParser::node_param() {
  Node_paramContext *_localctx = _tracker.createInstance<Node_paramContext>(_ctx, getState());
  enterRule(_localctx, 134, PrismParser::RuleNode_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(659);
    node_param_id();
    setState(660);
    match(PrismParser::ASSIGN);
    setState(661);
    value_id();
    setState(662);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_output_declContext ------------------------------------------------------------------

PrismParser::Node_output_declContext::Node_output_declContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Node_output_declContext::NODE_OUTPUT() {
  return getToken(PrismParser::NODE_OUTPUT, 0);
}

PrismParser::Type_idContext* PrismParser::Node_output_declContext::type_id() {
  return getRuleContext<PrismParser::Type_idContext>(0);
}

PrismParser::Name_idContext* PrismParser::Node_output_declContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Node_output_declContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Node_output_declContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Node_output_declContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::Node_output_declContext::getRuleIndex() const {
  return PrismParser::RuleNode_output_decl;
}

void PrismParser::Node_output_declContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_output_decl(this);
}

void PrismParser::Node_output_declContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_output_decl(this);
}


std::any PrismParser::Node_output_declContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_output_decl(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_output_declContext* PrismParser::node_output_decl() {
  Node_output_declContext *_localctx = _tracker.createInstance<Node_output_declContext>(_ctx, getState());
  enterRule(_localctx, 136, PrismParser::RuleNode_output_decl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(667);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(664);
        option_block(); 
      }
      setState(669);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    }
    setState(670);
    match(PrismParser::NODE_OUTPUT);
    setState(671);
    type_id();
    setState(672);
    name_id();
    setState(673);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_statContext ------------------------------------------------------------------

PrismParser::Node_statContext::Node_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Node_paramContext* PrismParser::Node_statContext::node_param() {
  return getRuleContext<PrismParser::Node_paramContext>(0);
}

PrismParser::Node_output_declContext* PrismParser::Node_statContext::node_output_decl() {
  return getRuleContext<PrismParser::Node_output_declContext>(0);
}

tree::TerminalNode* PrismParser::Node_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Node_statContext::getRuleIndex() const {
  return PrismParser::RuleNode_stat;
}

void PrismParser::Node_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_stat(this);
}

void PrismParser::Node_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_stat(this);
}


std::any PrismParser::Node_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_statContext* PrismParser::node_stat() {
  Node_statContext *_localctx = _tracker.createInstance<Node_statContext>(_ctx, getState());
  enterRule(_localctx, 138, PrismParser::RuleNode_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(678);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::T__7:
      case PrismParser::T__8:
      case PrismParser::T__9:
      case PrismParser::T__10:
      case PrismParser::T__11: {
        enterOuterAlt(_localctx, 1);
        setState(675);
        node_param();
        break;
      }

      case PrismParser::OSBRACE:
      case PrismParser::NODE_OUTPUT: {
        enterOuterAlt(_localctx, 2);
        setState(676);
        node_output_decl();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(677);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_blockContext ------------------------------------------------------------------

PrismParser::Node_blockContext::Node_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Node_statContext *> PrismParser::Node_blockContext::node_stat() {
  return getRuleContexts<PrismParser::Node_statContext>();
}

PrismParser::Node_statContext* PrismParser::Node_blockContext::node_stat(size_t i) {
  return getRuleContext<PrismParser::Node_statContext>(i);
}


size_t PrismParser::Node_blockContext::getRuleIndex() const {
  return PrismParser::RuleNode_block;
}

void PrismParser::Node_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_block(this);
}

void PrismParser::Node_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_block(this);
}


std::any PrismParser::Node_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_blockContext* PrismParser::node_block() {
  Node_blockContext *_localctx = _tracker.createInstance<Node_blockContext>(_ctx, getState());
  enterRule(_localctx, 140, PrismParser::RuleNode_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(683);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7936) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 536875009) != 0) {
      setState(680);
      node_stat();
      setState(685);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_definitionContext ------------------------------------------------------------------

PrismParser::Node_definitionContext::Node_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Node_definitionContext::NODE() {
  return getToken(PrismParser::NODE, 0);
}

PrismParser::Name_idContext* PrismParser::Node_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Node_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Node_blockContext* PrismParser::Node_definitionContext::node_block() {
  return getRuleContext<PrismParser::Node_blockContext>(0);
}

tree::TerminalNode* PrismParser::Node_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}


size_t PrismParser::Node_definitionContext::getRuleIndex() const {
  return PrismParser::RuleNode_definition;
}

void PrismParser::Node_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_definition(this);
}

void PrismParser::Node_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_definition(this);
}


std::any PrismParser::Node_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNode_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Node_definitionContext* PrismParser::node_definition() {
  Node_definitionContext *_localctx = _tracker.createInstance<Node_definitionContext>(_ctx, getState());
  enterRule(_localctx, 142, PrismParser::RuleNode_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(686);
    match(PrismParser::NODE);
    setState(687);
    name_id();
    setState(688);
    match(PrismParser::OBRACE);
    setState(689);
    node_block();
    setState(690);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Workgraph_pso_statContext ------------------------------------------------------------------

PrismParser::Workgraph_pso_statContext::Workgraph_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Root_sigContext* PrismParser::Workgraph_pso_statContext::root_sig() {
  return getRuleContext<PrismParser::Root_sigContext>(0);
}

PrismParser::ShaderContext* PrismParser::Workgraph_pso_statContext::shader() {
  return getRuleContext<PrismParser::ShaderContext>(0);
}

PrismParser::Define_declarationContext* PrismParser::Workgraph_pso_statContext::define_declaration() {
  return getRuleContext<PrismParser::Define_declarationContext>(0);
}

PrismParser::Node_definitionContext* PrismParser::Workgraph_pso_statContext::node_definition() {
  return getRuleContext<PrismParser::Node_definitionContext>(0);
}

tree::TerminalNode* PrismParser::Workgraph_pso_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Workgraph_pso_statContext::getRuleIndex() const {
  return PrismParser::RuleWorkgraph_pso_stat;
}

void PrismParser::Workgraph_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_stat(this);
}

void PrismParser::Workgraph_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_stat(this);
}


std::any PrismParser::Workgraph_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Workgraph_pso_statContext* PrismParser::workgraph_pso_stat() {
  Workgraph_pso_statContext *_localctx = _tracker.createInstance<Workgraph_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 144, PrismParser::RuleWorkgraph_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(697);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(692);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(693);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(694);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(695);
      node_definition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(696);
      match(PrismParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Workgraph_pso_blockContext ------------------------------------------------------------------

PrismParser::Workgraph_pso_blockContext::Workgraph_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Workgraph_pso_statContext *> PrismParser::Workgraph_pso_blockContext::workgraph_pso_stat() {
  return getRuleContexts<PrismParser::Workgraph_pso_statContext>();
}

PrismParser::Workgraph_pso_statContext* PrismParser::Workgraph_pso_blockContext::workgraph_pso_stat(size_t i) {
  return getRuleContext<PrismParser::Workgraph_pso_statContext>(i);
}


size_t PrismParser::Workgraph_pso_blockContext::getRuleIndex() const {
  return PrismParser::RuleWorkgraph_pso_block;
}

void PrismParser::Workgraph_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_block(this);
}

void PrismParser::Workgraph_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_block(this);
}


std::any PrismParser::Workgraph_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Workgraph_pso_blockContext* PrismParser::workgraph_pso_block() {
  Workgraph_pso_blockContext *_localctx = _tracker.createInstance<Workgraph_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 146, PrismParser::RuleWorkgraph_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(702);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541067265) != 0) {
      setState(699);
      workgraph_pso_stat();
      setState(704);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Workgraph_pso_definitionContext ------------------------------------------------------------------

PrismParser::Workgraph_pso_definitionContext::Workgraph_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Workgraph_pso_definitionContext::WORKGRAPH_PSO() {
  return getToken(PrismParser::WORKGRAPH_PSO, 0);
}

PrismParser::Name_idContext* PrismParser::Workgraph_pso_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Workgraph_pso_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Workgraph_pso_blockContext* PrismParser::Workgraph_pso_definitionContext::workgraph_pso_block() {
  return getRuleContext<PrismParser::Workgraph_pso_blockContext>(0);
}

tree::TerminalNode* PrismParser::Workgraph_pso_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Workgraph_pso_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Workgraph_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Workgraph_pso_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Workgraph_pso_definitionContext::getRuleIndex() const {
  return PrismParser::RuleWorkgraph_pso_definition;
}

void PrismParser::Workgraph_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_definition(this);
}

void PrismParser::Workgraph_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_definition(this);
}


std::any PrismParser::Workgraph_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Workgraph_pso_definitionContext* PrismParser::workgraph_pso_definition() {
  Workgraph_pso_definitionContext *_localctx = _tracker.createInstance<Workgraph_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 148, PrismParser::RuleWorkgraph_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(708);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(705);
        option_block(); 
      }
      setState(710);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    }
    setState(711);
    match(PrismParser::WORKGRAPH_PSO);
    setState(712);
    name_id();
    setState(714);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(713);
      inherit();
    }
    setState(716);
    match(PrismParser::OBRACE);
    setState(717);
    workgraph_pso_block();
    setState(718);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_statContext ------------------------------------------------------------------

PrismParser::Rtx_pass_statContext::Rtx_pass_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::ShaderContext* PrismParser::Rtx_pass_statContext::shader() {
  return getRuleContext<PrismParser::ShaderContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pass_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}

PrismParser::Pso_paramContext* PrismParser::Rtx_pass_statContext::pso_param() {
  return getRuleContext<PrismParser::Pso_paramContext>(0);
}


size_t PrismParser::Rtx_pass_statContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pass_stat;
}

void PrismParser::Rtx_pass_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_stat(this);
}

void PrismParser::Rtx_pass_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_stat(this);
}


std::any PrismParser::Rtx_pass_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pass_statContext* PrismParser::rtx_pass_stat() {
  Rtx_pass_statContext *_localctx = _tracker.createInstance<Rtx_pass_statContext>(_ctx, getState());
  enterRule(_localctx, 150, PrismParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(723);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(720);
      shader();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(721);
      match(PrismParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(722);
      pso_param();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_blockContext ------------------------------------------------------------------

PrismParser::Rtx_pass_blockContext::Rtx_pass_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Rtx_pass_statContext *> PrismParser::Rtx_pass_blockContext::rtx_pass_stat() {
  return getRuleContexts<PrismParser::Rtx_pass_statContext>();
}

PrismParser::Rtx_pass_statContext* PrismParser::Rtx_pass_blockContext::rtx_pass_stat(size_t i) {
  return getRuleContext<PrismParser::Rtx_pass_statContext>(i);
}


size_t PrismParser::Rtx_pass_blockContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pass_block;
}

void PrismParser::Rtx_pass_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_block(this);
}

void PrismParser::Rtx_pass_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_block(this);
}


std::any PrismParser::Rtx_pass_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pass_blockContext* PrismParser::rtx_pass_block() {
  Rtx_pass_blockContext *_localctx = _tracker.createInstance<Rtx_pass_blockContext>(_ctx, getState());
  enterRule(_localctx, 152, PrismParser::RuleRtx_pass_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(728);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080640) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(725);
      rtx_pass_stat();
      setState(730);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_definitionContext ------------------------------------------------------------------

PrismParser::Rtx_pass_definitionContext::Rtx_pass_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rtx_pass_definitionContext::RAYTRACE_PASS() {
  return getToken(PrismParser::RAYTRACE_PASS, 0);
}

PrismParser::Name_idContext* PrismParser::Rtx_pass_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pass_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Rtx_pass_blockContext* PrismParser::Rtx_pass_definitionContext::rtx_pass_block() {
  return getRuleContext<PrismParser::Rtx_pass_blockContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_pass_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Rtx_pass_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Rtx_pass_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Rtx_pass_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Rtx_pass_definitionContext::getRuleIndex() const {
  return PrismParser::RuleRtx_pass_definition;
}

void PrismParser::Rtx_pass_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_definition(this);
}

void PrismParser::Rtx_pass_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_definition(this);
}


std::any PrismParser::Rtx_pass_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_pass_definitionContext* PrismParser::rtx_pass_definition() {
  Rtx_pass_definitionContext *_localctx = _tracker.createInstance<Rtx_pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 154, PrismParser::RuleRtx_pass_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(734);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(731);
        option_block(); 
      }
      setState(736);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx);
    }
    setState(737);
    match(PrismParser::RAYTRACE_PASS);
    setState(738);
    name_id();
    setState(740);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(739);
      inherit();
    }
    setState(742);
    match(PrismParser::OBRACE);
    setState(743);
    rtx_pass_block();
    setState(744);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_statContext ------------------------------------------------------------------

PrismParser::Rtx_raygen_statContext::Rtx_raygen_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::ShaderContext* PrismParser::Rtx_raygen_statContext::shader() {
  return getRuleContext<PrismParser::ShaderContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_raygen_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Rtx_raygen_statContext::getRuleIndex() const {
  return PrismParser::RuleRtx_raygen_stat;
}

void PrismParser::Rtx_raygen_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_stat(this);
}

void PrismParser::Rtx_raygen_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_stat(this);
}


std::any PrismParser::Rtx_raygen_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_raygen_statContext* PrismParser::rtx_raygen_stat() {
  Rtx_raygen_statContext *_localctx = _tracker.createInstance<Rtx_raygen_statContext>(_ctx, getState());
  enterRule(_localctx, 156, PrismParser::RuleRtx_raygen_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(748);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::T__12:
      case PrismParser::T__13:
      case PrismParser::T__14:
      case PrismParser::T__15:
      case PrismParser::T__16:
      case PrismParser::T__17:
      case PrismParser::T__18:
      case PrismParser::T__19:
      case PrismParser::T__20:
      case PrismParser::T__21:
      case PrismParser::T__22:
      case PrismParser::T__23:
      case PrismParser::T__24:
      case PrismParser::OSBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(746);
        shader();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(747);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_blockContext ------------------------------------------------------------------

PrismParser::Rtx_raygen_blockContext::Rtx_raygen_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Rtx_raygen_statContext *> PrismParser::Rtx_raygen_blockContext::rtx_raygen_stat() {
  return getRuleContexts<PrismParser::Rtx_raygen_statContext>();
}

PrismParser::Rtx_raygen_statContext* PrismParser::Rtx_raygen_blockContext::rtx_raygen_stat(size_t i) {
  return getRuleContext<PrismParser::Rtx_raygen_statContext>(i);
}


size_t PrismParser::Rtx_raygen_blockContext::getRuleIndex() const {
  return PrismParser::RuleRtx_raygen_block;
}

void PrismParser::Rtx_raygen_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_block(this);
}

void PrismParser::Rtx_raygen_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_block(this);
}


std::any PrismParser::Rtx_raygen_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_raygen_blockContext* PrismParser::rtx_raygen_block() {
  Rtx_raygen_blockContext *_localctx = _tracker.createInstance<Rtx_raygen_blockContext>(_ctx, getState());
  enterRule(_localctx, 158, PrismParser::RuleRtx_raygen_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(753);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100672) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(750);
      rtx_raygen_stat();
      setState(755);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_definitionContext ------------------------------------------------------------------

PrismParser::Rtx_raygen_definitionContext::Rtx_raygen_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Rtx_raygen_definitionContext::RAYTRACE_RAYGEN() {
  return getToken(PrismParser::RAYTRACE_RAYGEN, 0);
}

PrismParser::Name_idContext* PrismParser::Rtx_raygen_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_raygen_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Rtx_raygen_blockContext* PrismParser::Rtx_raygen_definitionContext::rtx_raygen_block() {
  return getRuleContext<PrismParser::Rtx_raygen_blockContext>(0);
}

tree::TerminalNode* PrismParser::Rtx_raygen_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Rtx_raygen_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Rtx_raygen_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Rtx_raygen_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Rtx_raygen_definitionContext::getRuleIndex() const {
  return PrismParser::RuleRtx_raygen_definition;
}

void PrismParser::Rtx_raygen_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_definition(this);
}

void PrismParser::Rtx_raygen_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_definition(this);
}


std::any PrismParser::Rtx_raygen_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Rtx_raygen_definitionContext* PrismParser::rtx_raygen_definition() {
  Rtx_raygen_definitionContext *_localctx = _tracker.createInstance<Rtx_raygen_definitionContext>(_ctx, getState());
  enterRule(_localctx, 160, PrismParser::RuleRtx_raygen_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(759);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(756);
        option_block(); 
      }
      setState(761);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    }
    setState(762);
    match(PrismParser::RAYTRACE_RAYGEN);
    setState(763);
    name_id();
    setState(765);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(764);
      inherit();
    }
    setState(767);
    match(PrismParser::OBRACE);
    setState(768);
    rtx_raygen_block();
    setState(769);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_declarationContext ------------------------------------------------------------------

PrismParser::View_declarationContext::View_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Type_idContext* PrismParser::View_declarationContext::type_id() {
  return getRuleContext<PrismParser::Type_idContext>(0);
}

PrismParser::Name_idContext* PrismParser::View_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::View_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::View_declarationContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::View_declarationContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::View_declarationContext::getRuleIndex() const {
  return PrismParser::RuleView_declaration;
}

void PrismParser::View_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_declaration(this);
}

void PrismParser::View_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_declaration(this);
}


std::any PrismParser::View_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitView_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::View_declarationContext* PrismParser::view_declaration() {
  View_declarationContext *_localctx = _tracker.createInstance<View_declarationContext>(_ctx, getState());
  enterRule(_localctx, 162, PrismParser::RuleView_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(774);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(771);
        option_block(); 
      }
      setState(776);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    }
    setState(777);
    type_id();
    setState(778);
    name_id();
    setState(779);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_statContext ------------------------------------------------------------------

PrismParser::View_statContext::View_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::View_declarationContext* PrismParser::View_statContext::view_declaration() {
  return getRuleContext<PrismParser::View_declarationContext>(0);
}

tree::TerminalNode* PrismParser::View_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::View_statContext::getRuleIndex() const {
  return PrismParser::RuleView_stat;
}

void PrismParser::View_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_stat(this);
}

void PrismParser::View_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_stat(this);
}


std::any PrismParser::View_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitView_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::View_statContext* PrismParser::view_stat() {
  View_statContext *_localctx = _tracker.createInstance<View_statContext>(_ctx, getState());
  enterRule(_localctx, 164, PrismParser::RuleView_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(783);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(781);
        view_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(782);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_blockContext ------------------------------------------------------------------

PrismParser::View_blockContext::View_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::View_statContext *> PrismParser::View_blockContext::view_stat() {
  return getRuleContexts<PrismParser::View_statContext>();
}

PrismParser::View_statContext* PrismParser::View_blockContext::view_stat(size_t i) {
  return getRuleContext<PrismParser::View_statContext>(i);
}


size_t PrismParser::View_blockContext::getRuleIndex() const {
  return PrismParser::RuleView_block;
}

void PrismParser::View_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_block(this);
}

void PrismParser::View_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_block(this);
}


std::any PrismParser::View_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitView_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::View_blockContext* PrismParser::view_block() {
  View_blockContext *_localctx = _tracker.createInstance<View_blockContext>(_ctx, getState());
  enterRule(_localctx, 166, PrismParser::RuleView_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(788);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(785);
      view_stat();
      setState(790);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_definitionContext ------------------------------------------------------------------

PrismParser::View_definitionContext::View_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::View_definitionContext::VIEW() {
  return getToken(PrismParser::VIEW, 0);
}

PrismParser::Name_idContext* PrismParser::View_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::View_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::View_blockContext* PrismParser::View_definitionContext::view_block() {
  return getRuleContext<PrismParser::View_blockContext>(0);
}

tree::TerminalNode* PrismParser::View_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::View_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::View_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::View_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::View_definitionContext::getRuleIndex() const {
  return PrismParser::RuleView_definition;
}

void PrismParser::View_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_definition(this);
}

void PrismParser::View_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_definition(this);
}


std::any PrismParser::View_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitView_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::View_definitionContext* PrismParser::view_definition() {
  View_definitionContext *_localctx = _tracker.createInstance<View_definitionContext>(_ctx, getState());
  enterRule(_localctx, 168, PrismParser::RuleView_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(794);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 72, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(791);
        option_block(); 
      }
      setState(796);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 72, _ctx);
    }
    setState(797);
    match(PrismParser::VIEW);
    setState(798);
    name_id();
    setState(800);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(799);
      inherit();
    }
    setState(802);
    match(PrismParser::OBRACE);
    setState(803);
    view_block();
    setState(804);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pass_definitionContext ------------------------------------------------------------------

PrismParser::Pass_definitionContext::Pass_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Pass_definitionContext::PASS() {
  return getToken(PrismParser::PASS, 0);
}

PrismParser::Name_idContext* PrismParser::Pass_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Pass_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::View_blockContext* PrismParser::Pass_definitionContext::view_block() {
  return getRuleContext<PrismParser::View_blockContext>(0);
}

tree::TerminalNode* PrismParser::Pass_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Pass_definitionContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Pass_definitionContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

PrismParser::InheritContext* PrismParser::Pass_definitionContext::inherit() {
  return getRuleContext<PrismParser::InheritContext>(0);
}


size_t PrismParser::Pass_definitionContext::getRuleIndex() const {
  return PrismParser::RulePass_definition;
}

void PrismParser::Pass_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPass_definition(this);
}

void PrismParser::Pass_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPass_definition(this);
}


std::any PrismParser::Pass_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPass_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pass_definitionContext* PrismParser::pass_definition() {
  Pass_definitionContext *_localctx = _tracker.createInstance<Pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 170, PrismParser::RulePass_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(809);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(806);
        option_block(); 
      }
      setState(811);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    }
    setState(812);
    match(PrismParser::PASS);
    setState(813);
    name_id();
    setState(815);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(814);
      inherit();
    }
    setState(817);
    match(PrismParser::OBRACE);
    setState(818);
    view_block();
    setState(819);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pipeline_statContext ------------------------------------------------------------------

PrismParser::Pipeline_statContext::Pipeline_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::Pipeline_statContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Pipeline_statContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Pipeline_statContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Pipeline_statContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}

tree::TerminalNode* PrismParser::Pipeline_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Pipeline_statContext::getRuleIndex() const {
  return PrismParser::RulePipeline_stat;
}

void PrismParser::Pipeline_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_stat(this);
}

void PrismParser::Pipeline_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_stat(this);
}


std::any PrismParser::Pipeline_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPipeline_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pipeline_statContext* PrismParser::pipeline_stat() {
  Pipeline_statContext *_localctx = _tracker.createInstance<Pipeline_statContext>(_ctx, getState());
  enterRule(_localctx, 172, PrismParser::RulePipeline_stat);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(831);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(824);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == PrismParser::OSBRACE) {
          setState(821);
          option_block();
          setState(826);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(827);
        name_id();
        setState(828);
        match(PrismParser::SCOL);
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(830);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pipeline_blockContext ------------------------------------------------------------------

PrismParser::Pipeline_blockContext::Pipeline_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Pipeline_statContext *> PrismParser::Pipeline_blockContext::pipeline_stat() {
  return getRuleContexts<PrismParser::Pipeline_statContext>();
}

PrismParser::Pipeline_statContext* PrismParser::Pipeline_blockContext::pipeline_stat(size_t i) {
  return getRuleContext<PrismParser::Pipeline_statContext>(i);
}


size_t PrismParser::Pipeline_blockContext::getRuleIndex() const {
  return PrismParser::RulePipeline_block;
}

void PrismParser::Pipeline_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_block(this);
}

void PrismParser::Pipeline_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_block(this);
}


std::any PrismParser::Pipeline_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPipeline_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pipeline_blockContext* PrismParser::pipeline_block() {
  Pipeline_blockContext *_localctx = _tracker.createInstance<Pipeline_blockContext>(_ctx, getState());
  enterRule(_localctx, 174, PrismParser::RulePipeline_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(836);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(833);
      pipeline_stat();
      setState(838);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pipeline_definitionContext ------------------------------------------------------------------

PrismParser::Pipeline_definitionContext::Pipeline_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Pipeline_definitionContext::PIPELINE() {
  return getToken(PrismParser::PIPELINE, 0);
}

PrismParser::Name_idContext* PrismParser::Pipeline_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Pipeline_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Pipeline_blockContext* PrismParser::Pipeline_definitionContext::pipeline_block() {
  return getRuleContext<PrismParser::Pipeline_blockContext>(0);
}

tree::TerminalNode* PrismParser::Pipeline_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}


size_t PrismParser::Pipeline_definitionContext::getRuleIndex() const {
  return PrismParser::RulePipeline_definition;
}

void PrismParser::Pipeline_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_definition(this);
}

void PrismParser::Pipeline_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_definition(this);
}


std::any PrismParser::Pipeline_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPipeline_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pipeline_definitionContext* PrismParser::pipeline_definition() {
  Pipeline_definitionContext *_localctx = _tracker.createInstance<Pipeline_definitionContext>(_ctx, getState());
  enterRule(_localctx, 176, PrismParser::RulePipeline_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(839);
    match(PrismParser::PIPELINE);
    setState(840);
    name_id();
    setState(841);
    match(PrismParser::OBRACE);
    setState(842);
    pipeline_block();
    setState(843);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_value_declarationContext ------------------------------------------------------------------

PrismParser::Enum_value_declarationContext::Enum_value_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Name_idContext* PrismParser::Enum_value_declarationContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Enum_value_declarationContext::SCOL() {
  return getToken(PrismParser::SCOL, 0);
}

tree::TerminalNode* PrismParser::Enum_value_declarationContext::ASSIGN() {
  return getToken(PrismParser::ASSIGN, 0);
}

PrismParser::Value_idContext* PrismParser::Enum_value_declarationContext::value_id() {
  return getRuleContext<PrismParser::Value_idContext>(0);
}


size_t PrismParser::Enum_value_declarationContext::getRuleIndex() const {
  return PrismParser::RuleEnum_value_declaration;
}

void PrismParser::Enum_value_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_value_declaration(this);
}

void PrismParser::Enum_value_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_value_declaration(this);
}


std::any PrismParser::Enum_value_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitEnum_value_declaration(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Enum_value_declarationContext* PrismParser::enum_value_declaration() {
  Enum_value_declarationContext *_localctx = _tracker.createInstance<Enum_value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 178, PrismParser::RuleEnum_value_declaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(845);
    name_id();
    setState(848);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(846);
      match(PrismParser::ASSIGN);
      setState(847);
      value_id();
    }
    setState(850);
    match(PrismParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_statContext ------------------------------------------------------------------

PrismParser::Enum_statContext::Enum_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Enum_value_declarationContext* PrismParser::Enum_statContext::enum_value_declaration() {
  return getRuleContext<PrismParser::Enum_value_declarationContext>(0);
}

tree::TerminalNode* PrismParser::Enum_statContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::Enum_statContext::getRuleIndex() const {
  return PrismParser::RuleEnum_stat;
}

void PrismParser::Enum_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_stat(this);
}

void PrismParser::Enum_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_stat(this);
}


std::any PrismParser::Enum_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitEnum_stat(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Enum_statContext* PrismParser::enum_stat() {
  Enum_statContext *_localctx = _tracker.createInstance<Enum_statContext>(_ctx, getState());
  enterRule(_localctx, 180, PrismParser::RuleEnum_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(854);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(852);
        enum_value_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(853);
        match(PrismParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_blockContext ------------------------------------------------------------------

PrismParser::Enum_blockContext::Enum_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PrismParser::Enum_statContext *> PrismParser::Enum_blockContext::enum_stat() {
  return getRuleContexts<PrismParser::Enum_statContext>();
}

PrismParser::Enum_statContext* PrismParser::Enum_blockContext::enum_stat(size_t i) {
  return getRuleContext<PrismParser::Enum_statContext>(i);
}


size_t PrismParser::Enum_blockContext::getRuleIndex() const {
  return PrismParser::RuleEnum_block;
}

void PrismParser::Enum_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_block(this);
}

void PrismParser::Enum_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_block(this);
}


std::any PrismParser::Enum_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitEnum_block(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Enum_blockContext* PrismParser::enum_block() {
  Enum_blockContext *_localctx = _tracker.createInstance<Enum_blockContext>(_ctx, getState());
  enterRule(_localctx, 182, PrismParser::RuleEnum_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(859);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ID

    || _la == PrismParser::COMMENT) {
      setState(856);
      enum_stat();
      setState(861);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_definitionContext ------------------------------------------------------------------

PrismParser::Enum_definitionContext::Enum_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Enum_definitionContext::ENUM() {
  return getToken(PrismParser::ENUM, 0);
}

PrismParser::Name_idContext* PrismParser::Enum_definitionContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

tree::TerminalNode* PrismParser::Enum_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

PrismParser::Enum_blockContext* PrismParser::Enum_definitionContext::enum_block() {
  return getRuleContext<PrismParser::Enum_blockContext>(0);
}

tree::TerminalNode* PrismParser::Enum_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}


size_t PrismParser::Enum_definitionContext::getRuleIndex() const {
  return PrismParser::RuleEnum_definition;
}

void PrismParser::Enum_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_definition(this);
}

void PrismParser::Enum_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_definition(this);
}


std::any PrismParser::Enum_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitEnum_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Enum_definitionContext* PrismParser::enum_definition() {
  Enum_definitionContext *_localctx = _tracker.createInstance<Enum_definitionContext>(_ctx, getState());
  enterRule(_localctx, 184, PrismParser::RuleEnum_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(862);
    match(PrismParser::ENUM);
    setState(863);
    name_id();
    setState(864);
    match(PrismParser::OBRACE);
    setState(865);
    enum_block();
    setState(866);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Shader_typeContext ------------------------------------------------------------------

PrismParser::Shader_typeContext::Shader_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PrismParser::Shader_typeContext::getRuleIndex() const {
  return PrismParser::RuleShader_type;
}

void PrismParser::Shader_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader_type(this);
}

void PrismParser::Shader_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader_type(this);
}


std::any PrismParser::Shader_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitShader_type(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Shader_typeContext* PrismParser::shader_type() {
  Shader_typeContext *_localctx = _tracker.createInstance<Shader_typeContext>(_ctx, getState());
  enterRule(_localctx, 186, PrismParser::RuleShader_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(868);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100672) != 0)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pso_param_idContext ------------------------------------------------------------------

PrismParser::Pso_param_idContext::Pso_param_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PrismParser::Pso_param_idContext::getRuleIndex() const {
  return PrismParser::RulePso_param_id;
}

void PrismParser::Pso_param_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param_id(this);
}

void PrismParser::Pso_param_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param_id(this);
}


std::any PrismParser::Pso_param_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitPso_param_id(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Pso_param_idContext* PrismParser::pso_param_id() {
  Pso_param_idContext *_localctx = _tracker.createInstance<Pso_param_idContext>(_ctx, getState());
  enterRule(_localctx, 188, PrismParser::RulePso_param_id);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(870);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184304979968) != 0)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bool_typeContext ------------------------------------------------------------------

PrismParser::Bool_typeContext::Bool_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Bool_typeContext::TRUE() {
  return getToken(PrismParser::TRUE, 0);
}

tree::TerminalNode* PrismParser::Bool_typeContext::FALSE() {
  return getToken(PrismParser::FALSE, 0);
}


size_t PrismParser::Bool_typeContext::getRuleIndex() const {
  return PrismParser::RuleBool_type;
}

void PrismParser::Bool_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBool_type(this);
}

void PrismParser::Bool_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBool_type(this);
}


std::any PrismParser::Bool_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitBool_type(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Bool_typeContext* PrismParser::bool_type() {
  Bool_typeContext *_localctx = _tracker.createInstance<Bool_typeContext>(_ctx, getState());
  enterRule(_localctx, 190, PrismParser::RuleBool_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(872);
    _la = _input->LA(1);
    if (!(_la == PrismParser::TRUE

    || _la == PrismParser::FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void PrismParser::initialize() {
  ::antlr4::internal::call_once(prismParserOnceFlag, prismParserInitialize);
}
