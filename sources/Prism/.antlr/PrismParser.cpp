
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
      "qualified_ref", "member_ref", "cond_op", "flag_value_holder", "raw_value", 
      "options_assign", "option", "option_block", "array_count_id", "array", 
      "value_declaration", "slot_declaration", "sampler_declaration", "define_declaration", 
      "rtv_formats_declaration", "blends_declaration", "pointer", "pso_param", 
      "class_no_template", "type_with_template", "inherit_id", "name_id", 
      "option_id", "owner_id", "template_id", "function_id", "value_id", 
      "value_id_ignore", "type_id", "insert_block", "shader_path", "inherit", 
      "layout_stat", "layout_block", "layout_definition", "table_stat", 
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
  	4,1,103,853,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
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
  	91,2,92,7,92,2,93,7,93,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
  	0,1,0,1,0,1,0,5,0,204,8,0,10,0,12,0,207,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,
  	1,1,2,1,2,1,2,3,2,219,8,2,1,2,1,2,1,2,4,2,224,8,2,11,2,12,2,225,1,2,1,
  	2,3,2,230,8,2,1,3,4,3,233,8,3,11,3,12,3,234,1,4,1,4,1,4,1,4,1,4,3,4,242,
  	8,4,1,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,
  	1,10,1,11,1,11,3,11,263,8,11,1,12,1,12,1,12,1,12,5,12,269,8,12,10,12,
  	12,12,272,9,12,1,12,1,12,1,13,1,13,1,14,1,14,3,14,280,8,14,1,14,1,14,
  	1,15,5,15,285,8,15,10,15,12,15,288,9,15,1,15,1,15,1,15,3,15,293,8,15,
  	1,15,1,15,3,15,297,8,15,1,15,1,15,1,16,1,16,1,16,1,16,1,17,1,17,1,17,
  	1,17,1,17,1,17,1,18,5,18,312,8,18,10,18,12,18,315,9,18,1,18,1,18,1,18,
  	1,18,3,18,321,8,18,1,18,1,18,1,19,5,19,326,8,19,10,19,12,19,329,9,19,
  	1,19,1,19,1,19,1,19,1,19,1,20,5,20,337,8,20,10,20,12,20,340,9,20,1,20,
  	1,20,1,20,1,20,1,20,1,21,1,21,1,22,5,22,350,8,22,10,22,12,22,353,9,22,
  	1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,24,1,24,1,24,5,24,365,8,24,10,24,
  	12,24,368,9,24,1,24,3,24,371,8,24,1,24,3,24,374,8,24,1,25,1,25,1,26,1,
  	26,1,27,1,27,1,28,1,28,1,29,1,29,1,30,1,30,1,30,3,30,389,8,30,1,30,1,
  	30,5,30,393,8,30,10,30,12,30,396,9,30,1,30,1,30,1,31,1,31,1,31,1,31,1,
  	31,1,31,1,31,3,31,407,8,31,1,32,1,32,1,32,1,32,3,32,413,8,32,1,33,1,33,
  	1,34,1,34,1,35,1,35,1,36,1,36,1,36,1,36,5,36,425,8,36,10,36,12,36,428,
  	9,36,1,37,1,37,1,37,3,37,433,8,37,1,38,5,38,436,8,38,10,38,12,38,439,
  	9,38,1,39,1,39,1,39,3,39,444,8,39,1,39,1,39,1,39,1,39,1,40,1,40,1,40,
  	1,40,3,40,454,8,40,1,41,5,41,457,8,41,10,41,12,41,460,9,41,1,41,1,41,
  	1,41,1,41,1,41,1,41,3,41,468,8,41,1,41,1,41,1,42,1,42,1,42,1,42,1,42,
  	5,42,477,8,42,10,42,12,42,480,9,42,1,43,1,43,1,43,1,44,5,44,486,8,44,
  	10,44,12,44,489,9,44,1,45,5,45,492,8,45,10,45,12,45,495,9,45,1,45,1,45,
  	1,45,3,45,500,8,45,1,45,1,45,1,45,1,45,1,46,1,46,1,46,1,46,1,47,1,47,
  	1,47,1,47,1,48,1,48,1,48,3,48,517,8,48,1,49,5,49,520,8,49,10,49,12,49,
  	523,9,49,1,50,1,50,1,50,1,50,1,50,1,50,1,51,1,51,1,52,1,52,1,52,1,52,
  	5,52,537,8,52,10,52,12,52,540,9,52,1,52,1,52,1,53,1,53,1,53,1,53,1,53,
  	1,54,5,54,550,8,54,10,54,12,54,553,9,54,1,54,1,54,1,54,1,54,1,54,1,55,
  	1,55,1,55,1,55,3,55,564,8,55,1,56,5,56,567,8,56,10,56,12,56,570,9,56,
  	1,57,5,57,573,8,57,10,57,12,57,576,9,57,1,57,1,57,1,57,3,57,581,8,57,
  	1,57,1,57,1,57,1,57,1,58,1,58,1,58,1,58,1,58,1,58,1,58,3,58,594,8,58,
  	1,59,5,59,597,8,59,10,59,12,59,600,9,59,1,60,5,60,603,8,60,10,60,12,60,
  	606,9,60,1,60,1,60,1,60,3,60,611,8,60,1,60,1,60,1,60,1,60,1,61,1,61,3,
  	61,619,8,61,1,62,5,62,622,8,62,10,62,12,62,625,9,62,1,63,1,63,1,63,3,
  	63,630,8,63,1,63,1,63,1,63,1,63,1,64,1,64,1,65,1,65,1,65,1,65,1,65,1,
  	66,5,66,644,8,66,10,66,12,66,647,9,66,1,66,1,66,1,66,1,66,1,66,1,67,1,
  	67,1,67,3,67,657,8,67,1,68,5,68,660,8,68,10,68,12,68,663,9,68,1,69,1,
  	69,1,69,1,69,1,69,1,69,1,70,1,70,1,70,1,70,1,70,3,70,676,8,70,1,71,5,
  	71,679,8,71,10,71,12,71,682,9,71,1,72,5,72,685,8,72,10,72,12,72,688,9,
  	72,1,72,1,72,1,72,3,72,693,8,72,1,72,1,72,1,72,1,72,1,73,1,73,1,73,3,
  	73,702,8,73,1,74,5,74,705,8,74,10,74,12,74,708,9,74,1,75,5,75,711,8,75,
  	10,75,12,75,714,9,75,1,75,1,75,1,75,3,75,719,8,75,1,75,1,75,1,75,1,75,
  	1,76,1,76,3,76,727,8,76,1,77,5,77,730,8,77,10,77,12,77,733,9,77,1,78,
  	5,78,736,8,78,10,78,12,78,739,9,78,1,78,1,78,1,78,3,78,744,8,78,1,78,
  	1,78,1,78,1,78,1,79,5,79,751,8,79,10,79,12,79,754,9,79,1,79,1,79,1,79,
  	1,79,1,80,1,80,3,80,762,8,80,1,81,5,81,765,8,81,10,81,12,81,768,9,81,
  	1,82,5,82,771,8,82,10,82,12,82,774,9,82,1,82,1,82,1,82,3,82,779,8,82,
  	1,82,1,82,1,82,1,82,1,83,5,83,786,8,83,10,83,12,83,789,9,83,1,83,1,83,
  	1,83,3,83,794,8,83,1,83,1,83,1,83,1,83,1,84,5,84,801,8,84,10,84,12,84,
  	804,9,84,1,84,1,84,1,84,1,84,3,84,810,8,84,1,85,5,85,813,8,85,10,85,12,
  	85,816,9,85,1,86,1,86,1,86,1,86,1,86,1,86,1,87,1,87,1,87,3,87,827,8,87,
  	1,87,1,87,1,88,1,88,3,88,833,8,88,1,89,5,89,836,8,89,10,89,12,89,839,
  	9,89,1,90,1,90,1,90,1,90,1,90,1,90,1,91,1,91,1,92,1,92,1,93,1,93,1,93,
  	18,286,313,327,338,351,426,458,493,551,574,604,645,686,712,737,752,772,
  	787,0,94,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,
  	44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,
  	90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
  	128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,
  	164,166,168,170,172,174,176,178,180,182,184,186,0,7,4,0,45,46,48,53,59,
  	59,64,65,2,0,92,92,95,95,1,0,64,65,1,0,8,12,1,0,13,25,1,0,26,44,1,0,70,
  	71,878,0,205,1,0,0,0,2,210,1,0,0,0,4,229,1,0,0,0,6,232,1,0,0,0,8,241,
  	1,0,0,0,10,243,1,0,0,0,12,247,1,0,0,0,14,251,1,0,0,0,16,253,1,0,0,0,18,
  	255,1,0,0,0,20,257,1,0,0,0,22,260,1,0,0,0,24,264,1,0,0,0,26,275,1,0,0,
  	0,28,277,1,0,0,0,30,286,1,0,0,0,32,300,1,0,0,0,34,304,1,0,0,0,36,313,
  	1,0,0,0,38,327,1,0,0,0,40,338,1,0,0,0,42,346,1,0,0,0,44,351,1,0,0,0,46,
  	359,1,0,0,0,48,361,1,0,0,0,50,375,1,0,0,0,52,377,1,0,0,0,54,379,1,0,0,
  	0,56,381,1,0,0,0,58,383,1,0,0,0,60,385,1,0,0,0,62,406,1,0,0,0,64,412,
  	1,0,0,0,66,414,1,0,0,0,68,416,1,0,0,0,70,418,1,0,0,0,72,420,1,0,0,0,74,
  	432,1,0,0,0,76,437,1,0,0,0,78,440,1,0,0,0,80,453,1,0,0,0,82,458,1,0,0,
  	0,84,478,1,0,0,0,86,481,1,0,0,0,88,487,1,0,0,0,90,493,1,0,0,0,92,505,
  	1,0,0,0,94,509,1,0,0,0,96,516,1,0,0,0,98,521,1,0,0,0,100,524,1,0,0,0,
  	102,530,1,0,0,0,104,532,1,0,0,0,106,543,1,0,0,0,108,551,1,0,0,0,110,563,
  	1,0,0,0,112,568,1,0,0,0,114,574,1,0,0,0,116,593,1,0,0,0,118,598,1,0,0,
  	0,120,604,1,0,0,0,122,618,1,0,0,0,124,623,1,0,0,0,126,626,1,0,0,0,128,
  	635,1,0,0,0,130,637,1,0,0,0,132,645,1,0,0,0,134,656,1,0,0,0,136,661,1,
  	0,0,0,138,664,1,0,0,0,140,675,1,0,0,0,142,680,1,0,0,0,144,686,1,0,0,0,
  	146,701,1,0,0,0,148,706,1,0,0,0,150,712,1,0,0,0,152,726,1,0,0,0,154,731,
  	1,0,0,0,156,737,1,0,0,0,158,752,1,0,0,0,160,761,1,0,0,0,162,766,1,0,0,
  	0,164,772,1,0,0,0,166,787,1,0,0,0,168,809,1,0,0,0,170,814,1,0,0,0,172,
  	817,1,0,0,0,174,823,1,0,0,0,176,832,1,0,0,0,178,837,1,0,0,0,180,840,1,
  	0,0,0,182,846,1,0,0,0,184,848,1,0,0,0,186,850,1,0,0,0,188,204,3,78,39,
  	0,189,204,3,90,45,0,190,204,3,100,50,0,191,204,3,144,72,0,192,204,3,114,
  	57,0,193,204,3,120,60,0,194,204,3,126,63,0,195,204,3,150,75,0,196,204,
  	3,156,78,0,197,204,3,166,83,0,198,204,3,164,82,0,199,204,3,172,86,0,200,
  	204,3,180,90,0,201,204,3,2,1,0,202,204,5,97,0,0,203,188,1,0,0,0,203,189,
  	1,0,0,0,203,190,1,0,0,0,203,191,1,0,0,0,203,192,1,0,0,0,203,193,1,0,0,
  	0,203,194,1,0,0,0,203,195,1,0,0,0,203,196,1,0,0,0,203,197,1,0,0,0,203,
  	198,1,0,0,0,203,199,1,0,0,0,203,200,1,0,0,0,203,201,1,0,0,0,203,202,1,
  	0,0,0,204,207,1,0,0,0,205,203,1,0,0,0,205,206,1,0,0,0,206,208,1,0,0,0,
  	207,205,1,0,0,0,208,209,5,0,0,1,209,1,1,0,0,0,210,211,5,1,0,0,211,212,
  	3,52,26,0,212,213,3,20,10,0,213,214,5,60,0,0,214,3,1,0,0,0,215,216,3,
  	56,28,0,216,217,5,2,0,0,217,219,1,0,0,0,218,215,1,0,0,0,218,219,1,0,0,
  	0,219,220,1,0,0,0,220,223,3,16,8,0,221,222,5,47,0,0,222,224,3,16,8,0,
  	223,221,1,0,0,0,224,225,1,0,0,0,225,223,1,0,0,0,225,226,1,0,0,0,226,230,
  	1,0,0,0,227,230,3,18,9,0,228,230,3,6,3,0,229,218,1,0,0,0,229,227,1,0,
  	0,0,229,228,1,0,0,0,230,5,1,0,0,0,231,233,3,8,4,0,232,231,1,0,0,0,233,
  	234,1,0,0,0,234,232,1,0,0,0,234,235,1,0,0,0,235,7,1,0,0,0,236,242,3,10,
  	5,0,237,242,3,60,30,0,238,242,3,12,6,0,239,242,3,62,31,0,240,242,3,14,
  	7,0,241,236,1,0,0,0,241,237,1,0,0,0,241,238,1,0,0,0,241,239,1,0,0,0,241,
  	240,1,0,0,0,242,9,1,0,0,0,243,244,3,56,28,0,244,245,5,2,0,0,245,246,3,
  	62,31,0,246,11,1,0,0,0,247,248,3,52,26,0,248,249,5,62,0,0,249,250,3,52,
  	26,0,250,13,1,0,0,0,251,252,7,0,0,0,252,15,1,0,0,0,253,254,3,62,31,0,
  	254,17,1,0,0,0,255,256,5,96,0,0,256,19,1,0,0,0,257,258,5,63,0,0,258,259,
  	3,4,2,0,259,21,1,0,0,0,260,262,3,52,26,0,261,263,3,20,10,0,262,261,1,
  	0,0,0,262,263,1,0,0,0,263,23,1,0,0,0,264,265,5,68,0,0,265,270,3,22,11,
  	0,266,267,5,3,0,0,267,269,3,22,11,0,268,266,1,0,0,0,269,272,1,0,0,0,270,
  	268,1,0,0,0,270,271,1,0,0,0,271,273,1,0,0,0,272,270,1,0,0,0,273,274,5,
  	69,0,0,274,25,1,0,0,0,275,276,5,93,0,0,276,27,1,0,0,0,277,279,5,68,0,
  	0,278,280,3,26,13,0,279,278,1,0,0,0,279,280,1,0,0,0,280,281,1,0,0,0,281,
  	282,5,69,0,0,282,29,1,0,0,0,283,285,3,24,12,0,284,283,1,0,0,0,285,288,
  	1,0,0,0,286,287,1,0,0,0,286,284,1,0,0,0,287,289,1,0,0,0,288,286,1,0,0,
  	0,289,290,3,66,33,0,290,292,3,52,26,0,291,293,3,28,14,0,292,291,1,0,0,
  	0,292,293,1,0,0,0,293,296,1,0,0,0,294,295,5,63,0,0,295,297,3,62,31,0,
  	296,294,1,0,0,0,296,297,1,0,0,0,297,298,1,0,0,0,298,299,5,60,0,0,299,
  	31,1,0,0,0,300,301,5,86,0,0,301,302,3,52,26,0,302,303,5,60,0,0,303,33,
  	1,0,0,0,304,305,5,4,0,0,305,306,3,52,26,0,306,307,5,63,0,0,307,308,3,
  	62,31,0,308,309,5,60,0,0,309,35,1,0,0,0,310,312,3,24,12,0,311,310,1,0,
  	0,0,312,315,1,0,0,0,313,314,1,0,0,0,313,311,1,0,0,0,314,316,1,0,0,0,315,
  	313,1,0,0,0,316,317,5,5,0,0,317,320,3,52,26,0,318,319,5,63,0,0,319,321,
  	3,104,52,0,320,318,1,0,0,0,320,321,1,0,0,0,321,322,1,0,0,0,322,323,5,
  	60,0,0,323,37,1,0,0,0,324,326,3,24,12,0,325,324,1,0,0,0,326,329,1,0,0,
  	0,327,328,1,0,0,0,327,325,1,0,0,0,328,330,1,0,0,0,329,327,1,0,0,0,330,
  	331,5,6,0,0,331,332,5,63,0,0,332,333,3,104,52,0,333,334,5,60,0,0,334,
  	39,1,0,0,0,335,337,3,24,12,0,336,335,1,0,0,0,337,340,1,0,0,0,338,339,
  	1,0,0,0,338,336,1,0,0,0,339,341,1,0,0,0,340,338,1,0,0,0,341,342,5,7,0,
  	0,342,343,5,63,0,0,343,344,3,104,52,0,344,345,5,60,0,0,345,41,1,0,0,0,
  	346,347,5,99,0,0,347,43,1,0,0,0,348,350,3,24,12,0,349,348,1,0,0,0,350,
  	353,1,0,0,0,351,352,1,0,0,0,351,349,1,0,0,0,352,354,1,0,0,0,353,351,1,
  	0,0,0,354,355,3,184,92,0,355,356,5,63,0,0,356,357,3,62,31,0,357,358,5,
  	60,0,0,358,45,1,0,0,0,359,360,5,92,0,0,360,47,1,0,0,0,361,370,3,46,23,
  	0,362,366,5,51,0,0,363,365,3,58,29,0,364,363,1,0,0,0,365,368,1,0,0,0,
  	366,364,1,0,0,0,366,367,1,0,0,0,367,369,1,0,0,0,368,366,1,0,0,0,369,371,
  	5,50,0,0,370,362,1,0,0,0,370,371,1,0,0,0,371,373,1,0,0,0,372,374,3,42,
  	21,0,373,372,1,0,0,0,373,374,1,0,0,0,374,49,1,0,0,0,375,376,5,92,0,0,
  	376,51,1,0,0,0,377,378,5,92,0,0,378,53,1,0,0,0,379,380,5,92,0,0,380,55,
  	1,0,0,0,381,382,5,92,0,0,382,57,1,0,0,0,383,384,5,92,0,0,384,59,1,0,0,
  	0,385,386,5,92,0,0,386,388,5,64,0,0,387,389,3,64,32,0,388,387,1,0,0,0,
  	388,389,1,0,0,0,389,394,1,0,0,0,390,391,5,3,0,0,391,393,3,64,32,0,392,
  	390,1,0,0,0,393,396,1,0,0,0,394,392,1,0,0,0,394,395,1,0,0,0,395,397,1,
  	0,0,0,396,394,1,0,0,0,397,398,5,65,0,0,398,61,1,0,0,0,399,407,3,182,91,
  	0,400,407,5,92,0,0,401,407,5,93,0,0,402,407,5,94,0,0,403,407,3,186,93,
  	0,404,407,3,60,30,0,405,407,3,104,52,0,406,399,1,0,0,0,406,400,1,0,0,
  	0,406,401,1,0,0,0,406,402,1,0,0,0,406,403,1,0,0,0,406,404,1,0,0,0,406,
  	405,1,0,0,0,407,63,1,0,0,0,408,413,5,92,0,0,409,413,5,93,0,0,410,413,
  	5,94,0,0,411,413,3,186,93,0,412,408,1,0,0,0,412,409,1,0,0,0,412,410,1,
  	0,0,0,412,411,1,0,0,0,413,65,1,0,0,0,414,415,3,48,24,0,415,67,1,0,0,0,
  	416,417,5,103,0,0,417,69,1,0,0,0,418,419,7,1,0,0,419,71,1,0,0,0,420,421,
  	5,61,0,0,421,426,3,50,25,0,422,423,5,3,0,0,423,425,3,50,25,0,424,422,
  	1,0,0,0,425,428,1,0,0,0,426,427,1,0,0,0,426,424,1,0,0,0,427,73,1,0,0,
  	0,428,426,1,0,0,0,429,433,3,32,16,0,430,433,3,34,17,0,431,433,5,97,0,
  	0,432,429,1,0,0,0,432,430,1,0,0,0,432,431,1,0,0,0,433,75,1,0,0,0,434,
  	436,3,74,37,0,435,434,1,0,0,0,436,439,1,0,0,0,437,435,1,0,0,0,437,438,
  	1,0,0,0,438,77,1,0,0,0,439,437,1,0,0,0,440,441,5,73,0,0,441,443,3,52,
  	26,0,442,444,3,72,36,0,443,442,1,0,0,0,443,444,1,0,0,0,444,445,1,0,0,
  	0,445,446,5,66,0,0,446,447,3,76,38,0,447,448,5,67,0,0,448,79,1,0,0,0,
  	449,454,3,30,15,0,450,454,3,82,41,0,451,454,3,68,34,0,452,454,5,97,0,
  	0,453,449,1,0,0,0,453,450,1,0,0,0,453,451,1,0,0,0,453,452,1,0,0,0,454,
  	81,1,0,0,0,455,457,3,24,12,0,456,455,1,0,0,0,457,460,1,0,0,0,458,459,
  	1,0,0,0,458,456,1,0,0,0,459,461,1,0,0,0,460,458,1,0,0,0,461,462,3,66,
  	33,0,462,463,3,52,26,0,463,464,5,64,0,0,464,465,3,84,42,0,465,467,5,65,
  	0,0,466,468,3,86,43,0,467,466,1,0,0,0,467,468,1,0,0,0,468,469,1,0,0,0,
  	469,470,5,100,0,0,470,83,1,0,0,0,471,472,5,64,0,0,472,473,3,84,42,0,473,
  	474,5,65,0,0,474,477,1,0,0,0,475,477,8,2,0,0,476,471,1,0,0,0,476,475,
  	1,0,0,0,477,480,1,0,0,0,478,476,1,0,0,0,478,479,1,0,0,0,479,85,1,0,0,
  	0,480,478,1,0,0,0,481,482,5,61,0,0,482,483,5,92,0,0,483,87,1,0,0,0,484,
  	486,3,80,40,0,485,484,1,0,0,0,486,489,1,0,0,0,487,485,1,0,0,0,487,488,
  	1,0,0,0,488,89,1,0,0,0,489,487,1,0,0,0,490,492,3,24,12,0,491,490,1,0,
  	0,0,492,495,1,0,0,0,493,494,1,0,0,0,493,491,1,0,0,0,494,496,1,0,0,0,495,
  	493,1,0,0,0,496,497,5,74,0,0,497,499,3,52,26,0,498,500,3,72,36,0,499,
  	498,1,0,0,0,499,500,1,0,0,0,500,501,1,0,0,0,501,502,5,66,0,0,502,503,
  	3,88,44,0,503,504,5,67,0,0,504,91,1,0,0,0,505,506,3,66,33,0,506,507,3,
  	52,26,0,507,508,5,60,0,0,508,93,1,0,0,0,509,510,5,89,0,0,510,511,3,52,
  	26,0,511,512,5,60,0,0,512,95,1,0,0,0,513,517,3,92,46,0,514,517,3,94,47,
  	0,515,517,5,97,0,0,516,513,1,0,0,0,516,514,1,0,0,0,516,515,1,0,0,0,517,
  	97,1,0,0,0,518,520,3,96,48,0,519,518,1,0,0,0,520,523,1,0,0,0,521,519,
  	1,0,0,0,521,522,1,0,0,0,522,99,1,0,0,0,523,521,1,0,0,0,524,525,5,87,0,
  	0,525,526,3,52,26,0,526,527,5,66,0,0,527,528,3,98,49,0,528,529,5,67,0,
  	0,529,101,1,0,0,0,530,531,3,62,31,0,531,103,1,0,0,0,532,533,5,66,0,0,
  	533,538,3,102,51,0,534,535,5,3,0,0,535,537,3,102,51,0,536,534,1,0,0,0,
  	537,540,1,0,0,0,538,536,1,0,0,0,538,539,1,0,0,0,539,541,1,0,0,0,540,538,
  	1,0,0,0,541,542,5,67,0,0,542,105,1,0,0,0,543,544,5,90,0,0,544,545,5,63,
  	0,0,545,546,3,52,26,0,546,547,5,60,0,0,547,107,1,0,0,0,548,550,3,24,12,
  	0,549,548,1,0,0,0,550,553,1,0,0,0,551,552,1,0,0,0,551,549,1,0,0,0,552,
  	554,1,0,0,0,553,551,1,0,0,0,554,555,3,182,91,0,555,556,5,63,0,0,556,557,
  	3,70,35,0,557,558,5,60,0,0,558,109,1,0,0,0,559,564,3,106,53,0,560,564,
  	3,108,54,0,561,564,3,36,18,0,562,564,5,97,0,0,563,559,1,0,0,0,563,560,
  	1,0,0,0,563,561,1,0,0,0,563,562,1,0,0,0,564,111,1,0,0,0,565,567,3,110,
  	55,0,566,565,1,0,0,0,567,570,1,0,0,0,568,566,1,0,0,0,568,569,1,0,0,0,
  	569,113,1,0,0,0,570,568,1,0,0,0,571,573,3,24,12,0,572,571,1,0,0,0,573,
  	576,1,0,0,0,574,575,1,0,0,0,574,572,1,0,0,0,575,577,1,0,0,0,576,574,1,
  	0,0,0,577,578,5,75,0,0,578,580,3,52,26,0,579,581,3,72,36,0,580,579,1,
  	0,0,0,580,581,1,0,0,0,581,582,1,0,0,0,582,583,5,66,0,0,583,584,3,112,
  	56,0,584,585,5,67,0,0,585,115,1,0,0,0,586,594,3,106,53,0,587,594,3,108,
  	54,0,588,594,3,36,18,0,589,594,3,38,19,0,590,594,3,40,20,0,591,594,3,
  	44,22,0,592,594,5,97,0,0,593,586,1,0,0,0,593,587,1,0,0,0,593,588,1,0,
  	0,0,593,589,1,0,0,0,593,590,1,0,0,0,593,591,1,0,0,0,593,592,1,0,0,0,594,
  	117,1,0,0,0,595,597,3,116,58,0,596,595,1,0,0,0,597,600,1,0,0,0,598,596,
  	1,0,0,0,598,599,1,0,0,0,599,119,1,0,0,0,600,598,1,0,0,0,601,603,3,24,
  	12,0,602,601,1,0,0,0,603,606,1,0,0,0,604,605,1,0,0,0,604,602,1,0,0,0,
  	605,607,1,0,0,0,606,604,1,0,0,0,607,608,5,76,0,0,608,610,3,52,26,0,609,
  	611,3,72,36,0,610,609,1,0,0,0,610,611,1,0,0,0,611,612,1,0,0,0,612,613,
  	5,66,0,0,613,614,3,118,59,0,614,615,5,67,0,0,615,121,1,0,0,0,616,619,
  	3,106,53,0,617,619,5,97,0,0,618,616,1,0,0,0,618,617,1,0,0,0,619,123,1,
  	0,0,0,620,622,3,122,61,0,621,620,1,0,0,0,622,625,1,0,0,0,623,621,1,0,
  	0,0,623,624,1,0,0,0,624,125,1,0,0,0,625,623,1,0,0,0,626,627,5,77,0,0,
  	627,629,3,52,26,0,628,630,3,72,36,0,629,628,1,0,0,0,629,630,1,0,0,0,630,
  	631,1,0,0,0,631,632,5,66,0,0,632,633,3,124,62,0,633,634,5,67,0,0,634,
  	127,1,0,0,0,635,636,7,3,0,0,636,129,1,0,0,0,637,638,3,128,64,0,638,639,
  	5,63,0,0,639,640,3,62,31,0,640,641,5,60,0,0,641,131,1,0,0,0,642,644,3,
  	24,12,0,643,642,1,0,0,0,644,647,1,0,0,0,645,646,1,0,0,0,645,643,1,0,0,
  	0,646,648,1,0,0,0,647,645,1,0,0,0,648,649,5,80,0,0,649,650,3,66,33,0,
  	650,651,3,52,26,0,651,652,5,60,0,0,652,133,1,0,0,0,653,657,3,130,65,0,
  	654,657,3,132,66,0,655,657,5,97,0,0,656,653,1,0,0,0,656,654,1,0,0,0,656,
  	655,1,0,0,0,657,135,1,0,0,0,658,660,3,134,67,0,659,658,1,0,0,0,660,663,
  	1,0,0,0,661,659,1,0,0,0,661,662,1,0,0,0,662,137,1,0,0,0,663,661,1,0,0,
  	0,664,665,5,79,0,0,665,666,3,52,26,0,666,667,5,66,0,0,667,668,3,136,68,
  	0,668,669,5,67,0,0,669,139,1,0,0,0,670,676,3,106,53,0,671,676,3,108,54,
  	0,672,676,3,36,18,0,673,676,3,138,69,0,674,676,5,97,0,0,675,670,1,0,0,
  	0,675,671,1,0,0,0,675,672,1,0,0,0,675,673,1,0,0,0,675,674,1,0,0,0,676,
  	141,1,0,0,0,677,679,3,140,70,0,678,677,1,0,0,0,679,682,1,0,0,0,680,678,
  	1,0,0,0,680,681,1,0,0,0,681,143,1,0,0,0,682,680,1,0,0,0,683,685,3,24,
  	12,0,684,683,1,0,0,0,685,688,1,0,0,0,686,687,1,0,0,0,686,684,1,0,0,0,
  	687,689,1,0,0,0,688,686,1,0,0,0,689,690,5,78,0,0,690,692,3,52,26,0,691,
  	693,3,72,36,0,692,691,1,0,0,0,692,693,1,0,0,0,693,694,1,0,0,0,694,695,
  	5,66,0,0,695,696,3,142,71,0,696,697,5,67,0,0,697,145,1,0,0,0,698,702,
  	3,108,54,0,699,702,5,97,0,0,700,702,3,44,22,0,701,698,1,0,0,0,701,699,
  	1,0,0,0,701,700,1,0,0,0,702,147,1,0,0,0,703,705,3,146,73,0,704,703,1,
  	0,0,0,705,708,1,0,0,0,706,704,1,0,0,0,706,707,1,0,0,0,707,149,1,0,0,0,
  	708,706,1,0,0,0,709,711,3,24,12,0,710,709,1,0,0,0,711,714,1,0,0,0,712,
  	713,1,0,0,0,712,710,1,0,0,0,713,715,1,0,0,0,714,712,1,0,0,0,715,716,5,
  	82,0,0,716,718,3,52,26,0,717,719,3,72,36,0,718,717,1,0,0,0,718,719,1,
  	0,0,0,719,720,1,0,0,0,720,721,5,66,0,0,721,722,3,148,74,0,722,723,5,67,
  	0,0,723,151,1,0,0,0,724,727,3,108,54,0,725,727,5,97,0,0,726,724,1,0,0,
  	0,726,725,1,0,0,0,727,153,1,0,0,0,728,730,3,152,76,0,729,728,1,0,0,0,
  	730,733,1,0,0,0,731,729,1,0,0,0,731,732,1,0,0,0,732,155,1,0,0,0,733,731,
  	1,0,0,0,734,736,3,24,12,0,735,734,1,0,0,0,736,739,1,0,0,0,737,738,1,0,
  	0,0,737,735,1,0,0,0,738,740,1,0,0,0,739,737,1,0,0,0,740,741,5,81,0,0,
  	741,743,3,52,26,0,742,744,3,72,36,0,743,742,1,0,0,0,743,744,1,0,0,0,744,
  	745,1,0,0,0,745,746,5,66,0,0,746,747,3,154,77,0,747,748,5,67,0,0,748,
  	157,1,0,0,0,749,751,3,24,12,0,750,749,1,0,0,0,751,754,1,0,0,0,752,753,
  	1,0,0,0,752,750,1,0,0,0,753,755,1,0,0,0,754,752,1,0,0,0,755,756,3,66,
  	33,0,756,757,3,52,26,0,757,758,5,60,0,0,758,159,1,0,0,0,759,762,3,158,
  	79,0,760,762,5,97,0,0,761,759,1,0,0,0,761,760,1,0,0,0,762,161,1,0,0,0,
  	763,765,3,160,80,0,764,763,1,0,0,0,765,768,1,0,0,0,766,764,1,0,0,0,766,
  	767,1,0,0,0,767,163,1,0,0,0,768,766,1,0,0,0,769,771,3,24,12,0,770,769,
  	1,0,0,0,771,774,1,0,0,0,772,773,1,0,0,0,772,770,1,0,0,0,773,775,1,0,0,
  	0,774,772,1,0,0,0,775,776,5,84,0,0,776,778,3,52,26,0,777,779,3,72,36,
  	0,778,777,1,0,0,0,778,779,1,0,0,0,779,780,1,0,0,0,780,781,5,66,0,0,781,
  	782,3,162,81,0,782,783,5,67,0,0,783,165,1,0,0,0,784,786,3,24,12,0,785,
  	784,1,0,0,0,786,789,1,0,0,0,787,788,1,0,0,0,787,785,1,0,0,0,788,790,1,
  	0,0,0,789,787,1,0,0,0,790,791,5,83,0,0,791,793,3,52,26,0,792,794,3,72,
  	36,0,793,792,1,0,0,0,793,794,1,0,0,0,794,795,1,0,0,0,795,796,5,66,0,0,
  	796,797,3,162,81,0,797,798,5,67,0,0,798,167,1,0,0,0,799,801,3,24,12,0,
  	800,799,1,0,0,0,801,804,1,0,0,0,802,800,1,0,0,0,802,803,1,0,0,0,803,805,
  	1,0,0,0,804,802,1,0,0,0,805,806,3,52,26,0,806,807,5,60,0,0,807,810,1,
  	0,0,0,808,810,5,97,0,0,809,802,1,0,0,0,809,808,1,0,0,0,810,169,1,0,0,
  	0,811,813,3,168,84,0,812,811,1,0,0,0,813,816,1,0,0,0,814,812,1,0,0,0,
  	814,815,1,0,0,0,815,171,1,0,0,0,816,814,1,0,0,0,817,818,5,85,0,0,818,
  	819,3,52,26,0,819,820,5,66,0,0,820,821,3,170,85,0,821,822,5,67,0,0,822,
  	173,1,0,0,0,823,826,3,52,26,0,824,825,5,63,0,0,825,827,3,62,31,0,826,
  	824,1,0,0,0,826,827,1,0,0,0,827,828,1,0,0,0,828,829,5,60,0,0,829,175,
  	1,0,0,0,830,833,3,174,87,0,831,833,5,97,0,0,832,830,1,0,0,0,832,831,1,
  	0,0,0,833,177,1,0,0,0,834,836,3,176,88,0,835,834,1,0,0,0,836,839,1,0,
  	0,0,837,835,1,0,0,0,837,838,1,0,0,0,838,179,1,0,0,0,839,837,1,0,0,0,840,
  	841,5,91,0,0,841,842,3,52,26,0,842,843,5,66,0,0,843,844,3,178,89,0,844,
  	845,5,67,0,0,845,181,1,0,0,0,846,847,7,4,0,0,847,183,1,0,0,0,848,849,
  	7,5,0,0,849,185,1,0,0,0,850,851,7,6,0,0,851,187,1,0,0,0,80,203,205,218,
  	225,229,234,241,262,270,279,286,292,296,313,320,327,338,351,366,370,373,
  	388,394,406,412,426,432,437,443,453,458,467,476,478,487,493,499,516,521,
  	538,551,563,568,574,580,593,598,604,610,618,623,629,645,656,661,675,680,
  	686,692,701,706,712,718,726,731,737,743,752,761,766,772,778,787,793,802,
  	809,814,826,832,837
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
    setState(205);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 546039777) != 0) {
      setState(203);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(188);
        layout_definition();
        break;
      }

      case 2: {
        setState(189);
        table_definition();
        break;
      }

      case 3: {
        setState(190);
        rt_definition();
        break;
      }

      case 4: {
        setState(191);
        workgraph_pso_definition();
        break;
      }

      case 5: {
        setState(192);
        compute_pso_definition();
        break;
      }

      case 6: {
        setState(193);
        graphics_pso_definition();
        break;
      }

      case 7: {
        setState(194);
        rtx_pso_definition();
        break;
      }

      case 8: {
        setState(195);
        rtx_pass_definition();
        break;
      }

      case 9: {
        setState(196);
        rtx_raygen_definition();
        break;
      }

      case 10: {
        setState(197);
        pass_definition();
        break;
      }

      case 11: {
        setState(198);
        view_definition();
        break;
      }

      case 12: {
        setState(199);
        pipeline_definition();
        break;
      }

      case 13: {
        setState(200);
        enum_definition();
        break;
      }

      case 14: {
        setState(201);
        const_definition();
        break;
      }

      case 15: {
        setState(202);
        match(PrismParser::COMMENT);
        break;
      }

      default:
        break;
      }
      setState(207);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(208);
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
    setState(210);
    match(PrismParser::T__0);
    setState(211);
    name_id();
    setState(212);
    options_assign();
    setState(213);
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
    setState(229);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(218);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
      case 1: {
        setState(215);
        owner_id();
        setState(216);
        match(PrismParser::T__1);
        break;
      }

      default:
        break;
      }
      setState(220);
      flag_value_holder();
      setState(223); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(221);
        match(PrismParser::PIPE);
        setState(222);
        flag_value_holder();
        setState(225); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == PrismParser::PIPE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(227);
      raw_value();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(228);
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
    setState(232); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(231);
      cond_term();
      setState(234); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 594299229019561984) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 1879048391) != 0);
   
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
    setState(241);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(236);
      qualified_ref();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(237);
      function_id();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(238);
      member_ref();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(239);
      value_id();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(240);
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
  enterRule(_localctx, 10, PrismParser::RuleQualified_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(243);
    owner_id();
    setState(244);
    match(PrismParser::T__1);
    setState(245);
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
  enterRule(_localctx, 12, PrismParser::RuleMember_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(247);
    name_id();
    setState(248);
    match(PrismParser::DOT);
    setState(249);
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
  enterRule(_localctx, 14, PrismParser::RuleCond_op);
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
    setState(251);
    _la = _input->LA(1);
    if (!((((_la - 45) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 45)) & 1589755) != 0)) {
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
  enterRule(_localctx, 16, PrismParser::RuleFlag_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(253);
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
  enterRule(_localctx, 18, PrismParser::RuleRaw_value);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(255);
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
  enterRule(_localctx, 20, PrismParser::RuleOptions_assign);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(257);
    match(PrismParser::ASSIGN);
    setState(258);
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
  enterRule(_localctx, 22, PrismParser::RuleOption);
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
    setState(260);
    name_id();
    setState(262);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(261);
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
  enterRule(_localctx, 24, PrismParser::RuleOption_block);
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
    setState(264);
    match(PrismParser::OSBRACE);
    setState(265);
    option();
    setState(270);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(266);
      match(PrismParser::T__2);
      setState(267);
      option();
      setState(272);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(273);
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
  enterRule(_localctx, 26, PrismParser::RuleArray_count_id);

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
  enterRule(_localctx, 28, PrismParser::RuleArray);
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
    setState(277);
    match(PrismParser::OSBRACE);
    setState(279);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::INT_SCALAR) {
      setState(278);
      array_count_id();
    }
    setState(281);
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
  enterRule(_localctx, 30, PrismParser::RuleValue_declaration);
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
    setState(286);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(283);
        option_block(); 
      }
      setState(288);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    }
    setState(289);
    type_id();
    setState(290);
    name_id();
    setState(292);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::OSBRACE) {
      setState(291);
      array();
    }
    setState(296);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(294);
      match(PrismParser::ASSIGN);
      setState(295);
      value_id();
    }
    setState(298);
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
  enterRule(_localctx, 32, PrismParser::RuleSlot_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(300);
    match(PrismParser::SLOT);
    setState(301);
    name_id();
    setState(302);
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
  enterRule(_localctx, 34, PrismParser::RuleSampler_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(304);
    match(PrismParser::T__3);
    setState(305);
    name_id();
    setState(306);
    match(PrismParser::ASSIGN);
    setState(307);
    value_id();
    setState(308);
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
  enterRule(_localctx, 36, PrismParser::RuleDefine_declaration);
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
    setState(313);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(310);
        option_block(); 
      }
      setState(315);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    }
    setState(316);
    match(PrismParser::T__4);
    setState(317);
    name_id();
    setState(320);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(318);
      match(PrismParser::ASSIGN);
      setState(319);
      array_value_ids();
    }
    setState(322);
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
  enterRule(_localctx, 38, PrismParser::RuleRtv_formats_declaration);

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
    setState(327);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(324);
        option_block(); 
      }
      setState(329);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    }
    setState(330);
    match(PrismParser::T__5);
    setState(331);
    match(PrismParser::ASSIGN);
    setState(332);
    array_value_ids();
    setState(333);
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
  enterRule(_localctx, 40, PrismParser::RuleBlends_declaration);

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
    setState(338);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(335);
        option_block(); 
      }
      setState(340);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx);
    }
    setState(341);
    match(PrismParser::T__6);
    setState(342);
    match(PrismParser::ASSIGN);
    setState(343);
    array_value_ids();
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
  enterRule(_localctx, 42, PrismParser::RulePointer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(346);
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
  enterRule(_localctx, 44, PrismParser::RulePso_param);

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
    setState(351);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(348);
        option_block(); 
      }
      setState(353);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    }
    setState(354);
    pso_param_id();
    setState(355);
    match(PrismParser::ASSIGN);
    setState(356);
    value_id();
    setState(357);
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
  enterRule(_localctx, 46, PrismParser::RuleClass_no_template);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(359);
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
  enterRule(_localctx, 48, PrismParser::RuleType_with_template);
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
    setState(361);
    class_no_template();
    setState(370);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::LT) {
      setState(362);
      match(PrismParser::LT);
      setState(366);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == PrismParser::ID) {
        setState(363);
        template_id();
        setState(368);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(369);
      match(PrismParser::GT);
    }
    setState(373);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::POINTER) {
      setState(372);
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
  enterRule(_localctx, 50, PrismParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(375);
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
  enterRule(_localctx, 52, PrismParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(377);
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
  enterRule(_localctx, 54, PrismParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(379);
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
  enterRule(_localctx, 56, PrismParser::RuleOwner_id);

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
  enterRule(_localctx, 58, PrismParser::RuleTemplate_id);

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
  enterRule(_localctx, 60, PrismParser::RuleFunction_id);
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
    setState(385);
    match(PrismParser::ID);
    setState(386);
    match(PrismParser::OPAR);
    setState(388);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la - 70) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 70)) & 29360131) != 0) {
      setState(387);
      value_id_ignore();
    }
    setState(394);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(390);
      match(PrismParser::T__2);
      setState(391);
      value_id_ignore();
      setState(396);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(397);
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
  enterRule(_localctx, 62, PrismParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(406);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(399);
      shader_type();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(400);
      match(PrismParser::ID);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(401);
      match(PrismParser::INT_SCALAR);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(402);
      match(PrismParser::FLOAT_SCALAR);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(403);
      bool_type();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(404);
      function_id();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(405);
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
  enterRule(_localctx, 64, PrismParser::RuleValue_id_ignore);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(412);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(408);
        match(PrismParser::ID);
        break;
      }

      case PrismParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(409);
        match(PrismParser::INT_SCALAR);
        break;
      }

      case PrismParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(410);
        match(PrismParser::FLOAT_SCALAR);
        break;
      }

      case PrismParser::TRUE:
      case PrismParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(411);
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
  enterRule(_localctx, 66, PrismParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(414);
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
  enterRule(_localctx, 68, PrismParser::RuleInsert_block);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(416);
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
  enterRule(_localctx, 70, PrismParser::RuleShader_path);
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
    setState(418);
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
  enterRule(_localctx, 72, PrismParser::RuleInherit);

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
    setState(420);
    match(PrismParser::COLON);
    setState(421);
    inherit_id();
    setState(426);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(422);
        match(PrismParser::T__2);
        setState(423);
        inherit_id(); 
      }
      setState(428);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
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
  enterRule(_localctx, 74, PrismParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(432);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(429);
        slot_declaration();
        break;
      }

      case PrismParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(430);
        sampler_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(431);
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
  enterRule(_localctx, 76, PrismParser::RuleLayout_block);
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
    setState(437);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__3 || _la == PrismParser::SLOT

    || _la == PrismParser::COMMENT) {
      setState(434);
      layout_stat();
      setState(439);
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
  enterRule(_localctx, 78, PrismParser::RuleLayout_definition);
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
    match(PrismParser::LAYOUT);
    setState(441);
    name_id();
    setState(443);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(442);
      inherit();
    }
    setState(445);
    match(PrismParser::OBRACE);
    setState(446);
    layout_block();
    setState(447);
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
  enterRule(_localctx, 80, PrismParser::RuleTable_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(453);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(449);
      value_declaration();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(450);
      function_definition();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(451);
      insert_block();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(452);
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
  enterRule(_localctx, 82, PrismParser::RuleFunction_definition);
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
    setState(458);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(455);
        option_block(); 
      }
      setState(460);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    }
    setState(461);
    type_id();
    setState(462);
    name_id();
    setState(463);
    match(PrismParser::OPAR);
    setState(464);
    function_params();
    setState(465);
    match(PrismParser::CPAR);
    setState(467);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(466);
      function_semantic();
    }
    setState(469);
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
  enterRule(_localctx, 84, PrismParser::RuleFunction_params);
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
    setState(478);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & -2) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 1099511627773) != 0) {
      setState(476);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case PrismParser::OPAR: {
          setState(471);
          match(PrismParser::OPAR);
          setState(472);
          function_params();
          setState(473);
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
          setState(475);
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
      setState(480);
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
  enterRule(_localctx, 86, PrismParser::RuleFunction_semantic);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(481);
    match(PrismParser::COLON);
    setState(482);
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
  enterRule(_localctx, 88, PrismParser::RuleTable_block);
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
    setState(487);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 34913386497) != 0) {
      setState(484);
      table_stat();
      setState(489);
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
  enterRule(_localctx, 90, PrismParser::RuleTable_definition);
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
    setState(493);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(490);
        option_block(); 
      }
      setState(495);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    }
    setState(496);
    match(PrismParser::STRUCT);
    setState(497);
    name_id();
    setState(499);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(498);
      inherit();
    }
    setState(501);
    match(PrismParser::OBRACE);
    setState(502);
    table_block();
    setState(503);
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
  enterRule(_localctx, 92, PrismParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(505);
    type_id();
    setState(506);
    name_id();
    setState(507);
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
  enterRule(_localctx, 94, PrismParser::RuleRt_ds_declaration);

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
    match(PrismParser::DSV);
    setState(510);
    name_id();
    setState(511);
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
  enterRule(_localctx, 96, PrismParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(516);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(513);
        rt_color_declaration();
        break;
      }

      case PrismParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(514);
        rt_ds_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(515);
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
  enterRule(_localctx, 98, PrismParser::RuleRt_block);
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
    setState(521);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & 265) != 0) {
      setState(518);
      rt_stat();
      setState(523);
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
  enterRule(_localctx, 100, PrismParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(524);
    match(PrismParser::RT);
    setState(525);
    name_id();
    setState(526);
    match(PrismParser::OBRACE);
    setState(527);
    rt_block();
    setState(528);
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
  enterRule(_localctx, 102, PrismParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(530);
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
  enterRule(_localctx, 104, PrismParser::RuleArray_value_ids);
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
    setState(532);
    match(PrismParser::OBRACE);
    setState(533);
    array_value_holder();
    setState(538);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(534);
      match(PrismParser::T__2);
      setState(535);
      array_value_holder();
      setState(540);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(541);
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
  enterRule(_localctx, 106, PrismParser::RuleRoot_sig);

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
    match(PrismParser::ROOTSIG);
    setState(544);
    match(PrismParser::ASSIGN);
    setState(545);
    name_id();
    setState(546);
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
  enterRule(_localctx, 108, PrismParser::RuleShader);

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
    setState(551);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(548);
        option_block(); 
      }
      setState(553);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
    setState(554);
    shader_type();
    setState(555);
    match(PrismParser::ASSIGN);
    setState(556);
    shader_path();
    setState(557);
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
  enterRule(_localctx, 110, PrismParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(563);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(559);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(560);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(561);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(562);
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
  enterRule(_localctx, 112, PrismParser::RuleCompute_pso_block);
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
    setState(568);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(565);
      compute_pso_stat();
      setState(570);
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
  enterRule(_localctx, 114, PrismParser::RuleCompute_pso_definition);
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
    setState(574);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(571);
        option_block(); 
      }
      setState(576);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
    }
    setState(577);
    match(PrismParser::COMPUTE_PSO);
    setState(578);
    name_id();
    setState(580);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(579);
      inherit();
    }
    setState(582);
    match(PrismParser::OBRACE);
    setState(583);
    compute_pso_block();
    setState(584);
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
  enterRule(_localctx, 116, PrismParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(593);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(586);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(587);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(588);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(589);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(590);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(591);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(592);
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
  enterRule(_localctx, 118, PrismParser::RuleGraphics_pso_block);
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
    setState(598);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080864) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(595);
      graphics_pso_stat();
      setState(600);
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
  enterRule(_localctx, 120, PrismParser::RuleGraphics_pso_definition);
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
    setState(604);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(601);
        option_block(); 
      }
      setState(606);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    }
    setState(607);
    match(PrismParser::GRAPHICS_PSO);
    setState(608);
    name_id();
    setState(610);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(609);
      inherit();
    }
    setState(612);
    match(PrismParser::OBRACE);
    setState(613);
    graphics_pso_block();
    setState(614);
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
  enterRule(_localctx, 122, PrismParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(618);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(616);
        root_sig();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(617);
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
  enterRule(_localctx, 124, PrismParser::RuleRtx_pso_block);
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
    setState(623);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ROOTSIG

    || _la == PrismParser::COMMENT) {
      setState(620);
      rtx_pso_stat();
      setState(625);
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
  enterRule(_localctx, 126, PrismParser::RuleRtx_pso_definition);
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
    setState(626);
    match(PrismParser::RAYTRACE_PSO);
    setState(627);
    name_id();
    setState(629);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(628);
      inherit();
    }
    setState(631);
    match(PrismParser::OBRACE);
    setState(632);
    rtx_pso_block();
    setState(633);
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
  enterRule(_localctx, 128, PrismParser::RuleNode_param_id);
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
    setState(635);
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
  enterRule(_localctx, 130, PrismParser::RuleNode_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(637);
    node_param_id();
    setState(638);
    match(PrismParser::ASSIGN);
    setState(639);
    value_id();
    setState(640);
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
  enterRule(_localctx, 132, PrismParser::RuleNode_output_decl);

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
    setState(645);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(642);
        option_block(); 
      }
      setState(647);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx);
    }
    setState(648);
    match(PrismParser::NODE_OUTPUT);
    setState(649);
    type_id();
    setState(650);
    name_id();
    setState(651);
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
  enterRule(_localctx, 134, PrismParser::RuleNode_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(656);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::T__7:
      case PrismParser::T__8:
      case PrismParser::T__9:
      case PrismParser::T__10:
      case PrismParser::T__11: {
        enterOuterAlt(_localctx, 1);
        setState(653);
        node_param();
        break;
      }

      case PrismParser::OSBRACE:
      case PrismParser::NODE_OUTPUT: {
        enterOuterAlt(_localctx, 2);
        setState(654);
        node_output_decl();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(655);
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
  enterRule(_localctx, 136, PrismParser::RuleNode_block);
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
    setState(661);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7936) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 536875009) != 0) {
      setState(658);
      node_stat();
      setState(663);
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
  enterRule(_localctx, 138, PrismParser::RuleNode_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(664);
    match(PrismParser::NODE);
    setState(665);
    name_id();
    setState(666);
    match(PrismParser::OBRACE);
    setState(667);
    node_block();
    setState(668);
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
  enterRule(_localctx, 140, PrismParser::RuleWorkgraph_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(675);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(670);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(671);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(672);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(673);
      node_definition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(674);
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
  enterRule(_localctx, 142, PrismParser::RuleWorkgraph_pso_block);
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
    setState(680);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541067265) != 0) {
      setState(677);
      workgraph_pso_stat();
      setState(682);
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
  enterRule(_localctx, 144, PrismParser::RuleWorkgraph_pso_definition);
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
    setState(686);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(683);
        option_block(); 
      }
      setState(688);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx);
    }
    setState(689);
    match(PrismParser::WORKGRAPH_PSO);
    setState(690);
    name_id();
    setState(692);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(691);
      inherit();
    }
    setState(694);
    match(PrismParser::OBRACE);
    setState(695);
    workgraph_pso_block();
    setState(696);
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
  enterRule(_localctx, 146, PrismParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(701);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(698);
      shader();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(699);
      match(PrismParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(700);
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
  enterRule(_localctx, 148, PrismParser::RuleRtx_pass_block);
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
    setState(706);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080640) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(703);
      rtx_pass_stat();
      setState(708);
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
  enterRule(_localctx, 150, PrismParser::RuleRtx_pass_definition);
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
    setState(712);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(709);
        option_block(); 
      }
      setState(714);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
    }
    setState(715);
    match(PrismParser::RAYTRACE_PASS);
    setState(716);
    name_id();
    setState(718);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(717);
      inherit();
    }
    setState(720);
    match(PrismParser::OBRACE);
    setState(721);
    rtx_pass_block();
    setState(722);
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
  enterRule(_localctx, 152, PrismParser::RuleRtx_raygen_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(726);
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
        setState(724);
        shader();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(725);
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
  enterRule(_localctx, 154, PrismParser::RuleRtx_raygen_block);
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
    setState(731);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100672) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(728);
      rtx_raygen_stat();
      setState(733);
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
  enterRule(_localctx, 156, PrismParser::RuleRtx_raygen_definition);
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
    setState(737);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(734);
        option_block(); 
      }
      setState(739);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
    }
    setState(740);
    match(PrismParser::RAYTRACE_RAYGEN);
    setState(741);
    name_id();
    setState(743);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(742);
      inherit();
    }
    setState(745);
    match(PrismParser::OBRACE);
    setState(746);
    rtx_raygen_block();
    setState(747);
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
  enterRule(_localctx, 158, PrismParser::RuleView_declaration);

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
    setState(752);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(749);
        option_block(); 
      }
      setState(754);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    }
    setState(755);
    type_id();
    setState(756);
    name_id();
    setState(757);
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
  enterRule(_localctx, 160, PrismParser::RuleView_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(761);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(759);
        view_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(760);
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
  enterRule(_localctx, 162, PrismParser::RuleView_block);
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
    setState(766);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(763);
      view_stat();
      setState(768);
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
  enterRule(_localctx, 164, PrismParser::RuleView_definition);
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
    setState(772);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(769);
        option_block(); 
      }
      setState(774);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx);
    }
    setState(775);
    match(PrismParser::VIEW);
    setState(776);
    name_id();
    setState(778);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(777);
      inherit();
    }
    setState(780);
    match(PrismParser::OBRACE);
    setState(781);
    view_block();
    setState(782);
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
  enterRule(_localctx, 166, PrismParser::RulePass_definition);
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
    setState(787);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 72, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(784);
        option_block(); 
      }
      setState(789);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 72, _ctx);
    }
    setState(790);
    match(PrismParser::PASS);
    setState(791);
    name_id();
    setState(793);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(792);
      inherit();
    }
    setState(795);
    match(PrismParser::OBRACE);
    setState(796);
    view_block();
    setState(797);
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
  enterRule(_localctx, 168, PrismParser::RulePipeline_stat);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(809);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(802);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == PrismParser::OSBRACE) {
          setState(799);
          option_block();
          setState(804);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(805);
        name_id();
        setState(806);
        match(PrismParser::SCOL);
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(808);
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
  enterRule(_localctx, 170, PrismParser::RulePipeline_block);
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
    setState(814);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(811);
      pipeline_stat();
      setState(816);
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
  enterRule(_localctx, 172, PrismParser::RulePipeline_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(817);
    match(PrismParser::PIPELINE);
    setState(818);
    name_id();
    setState(819);
    match(PrismParser::OBRACE);
    setState(820);
    pipeline_block();
    setState(821);
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
  enterRule(_localctx, 174, PrismParser::RuleEnum_value_declaration);
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
    setState(823);
    name_id();
    setState(826);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(824);
      match(PrismParser::ASSIGN);
      setState(825);
      value_id();
    }
    setState(828);
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
  enterRule(_localctx, 176, PrismParser::RuleEnum_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(832);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(830);
        enum_value_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(831);
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
  enterRule(_localctx, 178, PrismParser::RuleEnum_block);
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
    setState(837);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ID

    || _la == PrismParser::COMMENT) {
      setState(834);
      enum_stat();
      setState(839);
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
  enterRule(_localctx, 180, PrismParser::RuleEnum_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(840);
    match(PrismParser::ENUM);
    setState(841);
    name_id();
    setState(842);
    match(PrismParser::OBRACE);
    setState(843);
    enum_block();
    setState(844);
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
  enterRule(_localctx, 182, PrismParser::RuleShader_type);
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
    setState(846);
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
  enterRule(_localctx, 184, PrismParser::RulePso_param_id);
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
    setState(848);
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
  enterRule(_localctx, 186, PrismParser::RuleBool_type);
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
    setState(850);
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
