
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
      "parse", "definition", "namespace_definition", "namespace_header", 
      "const_definition", "bind_option", "cond_expr", "cond_term", "call", 
      "call_arg", "qualified_ref", "member_ref", "cond_op", "flag_value_holder", 
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
      "'struct'", "'namespace'", "'ComputePSO'", "'GraphicsPSO'", "'RaytracePSO'", 
      "'WorkgraphPSO'", "'Node'", "'NodeOutput'", "'RaytraceRaygen'", "'RaytracePass'", 
      "'PassNode'", "'PassView'", "'Pipeline'", "'slot'", "'rt'", "'RTV'", 
      "'DSV'", "'root'", "'enum'", "", "", "", "", "", "", "", "'*'", "", 
      "'%{'", "'}%'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "OR", "AND", "PIPE", "EQ", 
      "NEQ", "GT", "LT", "GTEQ", "LTEQ", "PLUS", "MINUS", "DIV", "MOD", 
      "POW", "NOT", "SCOL", "COLON", "DOT", "ASSIGN", "OPAR", "CPAR", "OBRACE", 
      "CBRACE", "OSBRACE", "CSBRACE", "TRUE", "FALSE", "LOG", "LAYOUT", 
      "STRUCT", "NAMESPACE", "COMPUTE_PSO", "GRAPHICS_PSO", "RAYTRACE_PSO", 
      "WORKGRAPH_PSO", "NODE", "NODE_OUTPUT", "RAYTRACE_RAYGEN", "RAYTRACE_PASS", 
      "PASS", "VIEW", "PIPELINE", "SLOT", "RT", "RTV", "DSV", "ROOTSIG", 
      "ENUM", "ID", "INT_SCALAR", "FLOAT_SCALAR", "STRING", "RAWEXPR", "COMMENT", 
      "SPACE", "POINTER", "FUNC_BODY", "INSERT_START", "INSERT_END", "INSERT_BLOCK"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,104,904,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
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
  	91,2,92,7,92,2,93,7,93,2,94,7,94,2,95,7,95,2,96,7,96,2,97,7,97,2,98,7,
  	98,1,0,5,0,200,8,0,10,0,12,0,203,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,223,8,1,1,2,1,2,1,2,5,2,228,
  	8,2,10,2,12,2,231,9,2,1,2,1,2,1,3,5,3,236,8,3,10,3,12,3,239,9,3,1,3,1,
  	3,1,3,1,4,1,4,1,4,1,4,1,4,1,5,1,5,1,5,3,5,252,8,5,1,5,1,5,1,5,4,5,257,
  	8,5,11,5,12,5,258,1,5,1,5,3,5,263,8,5,1,6,4,6,266,8,6,11,6,12,6,267,1,
  	7,1,7,1,7,1,7,1,7,1,7,3,7,276,8,7,1,8,1,8,1,8,1,8,1,8,5,8,283,8,8,10,
  	8,12,8,286,9,8,1,8,1,8,1,9,4,9,291,8,9,11,9,12,9,292,1,10,1,10,1,10,1,
  	10,1,11,1,11,1,11,1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,15,1,15,1,15,1,
  	16,1,16,3,16,314,8,16,1,17,1,17,1,17,1,17,5,17,320,8,17,10,17,12,17,323,
  	9,17,1,17,1,17,1,18,1,18,1,19,1,19,3,19,331,8,19,1,19,1,19,1,20,5,20,
  	336,8,20,10,20,12,20,339,9,20,1,20,1,20,1,20,3,20,344,8,20,1,20,1,20,
  	3,20,348,8,20,1,20,1,20,1,21,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,
  	1,22,1,23,5,23,363,8,23,10,23,12,23,366,9,23,1,23,1,23,1,23,1,23,3,23,
  	372,8,23,1,23,1,23,1,24,5,24,377,8,24,10,24,12,24,380,9,24,1,24,1,24,
  	1,24,1,24,1,24,1,25,5,25,388,8,25,10,25,12,25,391,9,25,1,25,1,25,1,25,
  	1,25,1,25,1,26,1,26,1,27,5,27,401,8,27,10,27,12,27,404,9,27,1,27,1,27,
  	1,27,1,27,1,27,1,28,1,28,1,29,1,29,1,29,5,29,416,8,29,10,29,12,29,419,
  	9,29,1,29,3,29,422,8,29,1,29,3,29,425,8,29,1,30,1,30,1,31,1,31,1,32,1,
  	32,1,33,1,33,1,34,1,34,1,35,1,35,1,35,3,35,440,8,35,1,35,1,35,5,35,444,
  	8,35,10,35,12,35,447,9,35,1,35,1,35,1,36,1,36,1,36,1,36,1,36,1,36,1,36,
  	3,36,458,8,36,1,37,1,37,1,37,1,37,3,37,464,8,37,1,38,1,38,1,39,1,39,1,
  	40,1,40,1,41,1,41,1,41,1,41,5,41,476,8,41,10,41,12,41,479,9,41,1,42,1,
  	42,1,42,3,42,484,8,42,1,43,5,43,487,8,43,10,43,12,43,490,9,43,1,44,1,
  	44,1,44,3,44,495,8,44,1,44,1,44,1,44,1,44,1,45,1,45,1,45,1,45,3,45,505,
  	8,45,1,46,5,46,508,8,46,10,46,12,46,511,9,46,1,46,1,46,1,46,1,46,1,46,
  	1,46,3,46,519,8,46,1,46,1,46,1,47,1,47,1,47,1,47,1,47,5,47,528,8,47,10,
  	47,12,47,531,9,47,1,48,1,48,1,48,1,49,5,49,537,8,49,10,49,12,49,540,9,
  	49,1,50,5,50,543,8,50,10,50,12,50,546,9,50,1,50,1,50,1,50,3,50,551,8,
  	50,1,50,1,50,1,50,1,50,1,51,1,51,1,51,1,51,1,52,1,52,1,52,1,52,1,53,1,
  	53,1,53,3,53,568,8,53,1,54,5,54,571,8,54,10,54,12,54,574,9,54,1,55,1,
  	55,1,55,1,55,1,55,1,55,1,56,1,56,1,57,1,57,1,57,1,57,5,57,588,8,57,10,
  	57,12,57,591,9,57,1,57,1,57,1,58,1,58,1,58,1,58,1,58,1,59,5,59,601,8,
  	59,10,59,12,59,604,9,59,1,59,1,59,1,59,1,59,1,59,1,60,1,60,1,60,1,60,
  	3,60,615,8,60,1,61,5,61,618,8,61,10,61,12,61,621,9,61,1,62,5,62,624,8,
  	62,10,62,12,62,627,9,62,1,62,1,62,1,62,3,62,632,8,62,1,62,1,62,1,62,1,
  	62,1,63,1,63,1,63,1,63,1,63,1,63,1,63,3,63,645,8,63,1,64,5,64,648,8,64,
  	10,64,12,64,651,9,64,1,65,5,65,654,8,65,10,65,12,65,657,9,65,1,65,1,65,
  	1,65,3,65,662,8,65,1,65,1,65,1,65,1,65,1,66,1,66,3,66,670,8,66,1,67,5,
  	67,673,8,67,10,67,12,67,676,9,67,1,68,1,68,1,68,3,68,681,8,68,1,68,1,
  	68,1,68,1,68,1,69,1,69,1,70,1,70,1,70,1,70,1,70,1,71,5,71,695,8,71,10,
  	71,12,71,698,9,71,1,71,1,71,1,71,1,71,1,71,1,72,1,72,1,72,3,72,708,8,
  	72,1,73,5,73,711,8,73,10,73,12,73,714,9,73,1,74,1,74,1,74,1,74,1,74,1,
  	74,1,75,1,75,1,75,1,75,1,75,3,75,727,8,75,1,76,5,76,730,8,76,10,76,12,
  	76,733,9,76,1,77,5,77,736,8,77,10,77,12,77,739,9,77,1,77,1,77,1,77,3,
  	77,744,8,77,1,77,1,77,1,77,1,77,1,78,1,78,1,78,3,78,753,8,78,1,79,5,79,
  	756,8,79,10,79,12,79,759,9,79,1,80,5,80,762,8,80,10,80,12,80,765,9,80,
  	1,80,1,80,1,80,3,80,770,8,80,1,80,1,80,1,80,1,80,1,81,1,81,3,81,778,8,
  	81,1,82,5,82,781,8,82,10,82,12,82,784,9,82,1,83,5,83,787,8,83,10,83,12,
  	83,790,9,83,1,83,1,83,1,83,3,83,795,8,83,1,83,1,83,1,83,1,83,1,84,5,84,
  	802,8,84,10,84,12,84,805,9,84,1,84,1,84,1,84,1,84,1,85,1,85,3,85,813,
  	8,85,1,86,5,86,816,8,86,10,86,12,86,819,9,86,1,87,5,87,822,8,87,10,87,
  	12,87,825,9,87,1,87,1,87,1,87,3,87,830,8,87,1,87,1,87,1,87,1,87,1,88,
  	5,88,837,8,88,10,88,12,88,840,9,88,1,88,1,88,1,88,3,88,845,8,88,1,88,
  	1,88,1,88,1,88,1,89,5,89,852,8,89,10,89,12,89,855,9,89,1,89,1,89,1,89,
  	1,89,3,89,861,8,89,1,90,5,90,864,8,90,10,90,12,90,867,9,90,1,91,1,91,
  	1,91,1,91,1,91,1,91,1,92,1,92,1,92,3,92,878,8,92,1,92,1,92,1,93,1,93,
  	3,93,884,8,93,1,94,5,94,887,8,94,10,94,12,94,890,9,94,1,95,1,95,1,95,
  	1,95,1,95,1,95,1,96,1,96,1,97,1,97,1,98,1,98,1,98,19,237,337,364,378,
  	389,402,477,509,544,602,625,655,696,737,763,788,803,823,838,0,99,0,2,
  	4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,
  	52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,
  	98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,
  	134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,
  	170,172,174,176,178,180,182,184,186,188,190,192,194,196,0,7,5,0,45,46,
  	48,57,59,59,64,65,100,100,2,0,93,93,96,96,1,0,64,65,1,0,8,12,1,0,13,25,
  	1,0,26,44,1,0,70,71,930,0,201,1,0,0,0,2,222,1,0,0,0,4,224,1,0,0,0,6,237,
  	1,0,0,0,8,243,1,0,0,0,10,262,1,0,0,0,12,265,1,0,0,0,14,275,1,0,0,0,16,
  	277,1,0,0,0,18,290,1,0,0,0,20,294,1,0,0,0,22,298,1,0,0,0,24,302,1,0,0,
  	0,26,304,1,0,0,0,28,306,1,0,0,0,30,308,1,0,0,0,32,311,1,0,0,0,34,315,
  	1,0,0,0,36,326,1,0,0,0,38,328,1,0,0,0,40,337,1,0,0,0,42,351,1,0,0,0,44,
  	355,1,0,0,0,46,364,1,0,0,0,48,378,1,0,0,0,50,389,1,0,0,0,52,397,1,0,0,
  	0,54,402,1,0,0,0,56,410,1,0,0,0,58,412,1,0,0,0,60,426,1,0,0,0,62,428,
  	1,0,0,0,64,430,1,0,0,0,66,432,1,0,0,0,68,434,1,0,0,0,70,436,1,0,0,0,72,
  	457,1,0,0,0,74,463,1,0,0,0,76,465,1,0,0,0,78,467,1,0,0,0,80,469,1,0,0,
  	0,82,471,1,0,0,0,84,483,1,0,0,0,86,488,1,0,0,0,88,491,1,0,0,0,90,504,
  	1,0,0,0,92,509,1,0,0,0,94,529,1,0,0,0,96,532,1,0,0,0,98,538,1,0,0,0,100,
  	544,1,0,0,0,102,556,1,0,0,0,104,560,1,0,0,0,106,567,1,0,0,0,108,572,1,
  	0,0,0,110,575,1,0,0,0,112,581,1,0,0,0,114,583,1,0,0,0,116,594,1,0,0,0,
  	118,602,1,0,0,0,120,614,1,0,0,0,122,619,1,0,0,0,124,625,1,0,0,0,126,644,
  	1,0,0,0,128,649,1,0,0,0,130,655,1,0,0,0,132,669,1,0,0,0,134,674,1,0,0,
  	0,136,677,1,0,0,0,138,686,1,0,0,0,140,688,1,0,0,0,142,696,1,0,0,0,144,
  	707,1,0,0,0,146,712,1,0,0,0,148,715,1,0,0,0,150,726,1,0,0,0,152,731,1,
  	0,0,0,154,737,1,0,0,0,156,752,1,0,0,0,158,757,1,0,0,0,160,763,1,0,0,0,
  	162,777,1,0,0,0,164,782,1,0,0,0,166,788,1,0,0,0,168,803,1,0,0,0,170,812,
  	1,0,0,0,172,817,1,0,0,0,174,823,1,0,0,0,176,838,1,0,0,0,178,860,1,0,0,
  	0,180,865,1,0,0,0,182,868,1,0,0,0,184,874,1,0,0,0,186,883,1,0,0,0,188,
  	888,1,0,0,0,190,891,1,0,0,0,192,897,1,0,0,0,194,899,1,0,0,0,196,901,1,
  	0,0,0,198,200,3,2,1,0,199,198,1,0,0,0,200,203,1,0,0,0,201,199,1,0,0,0,
  	201,202,1,0,0,0,202,204,1,0,0,0,203,201,1,0,0,0,204,205,5,0,0,1,205,1,
  	1,0,0,0,206,223,3,88,44,0,207,223,3,100,50,0,208,223,3,110,55,0,209,223,
  	3,154,77,0,210,223,3,124,62,0,211,223,3,130,65,0,212,223,3,136,68,0,213,
  	223,3,160,80,0,214,223,3,166,83,0,215,223,3,176,88,0,216,223,3,174,87,
  	0,217,223,3,182,91,0,218,223,3,190,95,0,219,223,3,8,4,0,220,223,3,4,2,
  	0,221,223,5,98,0,0,222,206,1,0,0,0,222,207,1,0,0,0,222,208,1,0,0,0,222,
  	209,1,0,0,0,222,210,1,0,0,0,222,211,1,0,0,0,222,212,1,0,0,0,222,213,1,
  	0,0,0,222,214,1,0,0,0,222,215,1,0,0,0,222,216,1,0,0,0,222,217,1,0,0,0,
  	222,218,1,0,0,0,222,219,1,0,0,0,222,220,1,0,0,0,222,221,1,0,0,0,223,3,
  	1,0,0,0,224,225,3,6,3,0,225,229,5,66,0,0,226,228,3,2,1,0,227,226,1,0,
  	0,0,228,231,1,0,0,0,229,227,1,0,0,0,229,230,1,0,0,0,230,232,1,0,0,0,231,
  	229,1,0,0,0,232,233,5,67,0,0,233,5,1,0,0,0,234,236,3,34,17,0,235,234,
  	1,0,0,0,236,239,1,0,0,0,237,238,1,0,0,0,237,235,1,0,0,0,238,240,1,0,0,
  	0,239,237,1,0,0,0,240,241,5,75,0,0,241,242,3,62,31,0,242,7,1,0,0,0,243,
  	244,5,1,0,0,244,245,3,62,31,0,245,246,3,30,15,0,246,247,5,60,0,0,247,
  	9,1,0,0,0,248,249,3,66,33,0,249,250,5,2,0,0,250,252,1,0,0,0,251,248,1,
  	0,0,0,251,252,1,0,0,0,252,253,1,0,0,0,253,256,3,26,13,0,254,255,5,47,
  	0,0,255,257,3,26,13,0,256,254,1,0,0,0,257,258,1,0,0,0,258,256,1,0,0,0,
  	258,259,1,0,0,0,259,263,1,0,0,0,260,263,3,28,14,0,261,263,3,12,6,0,262,
  	251,1,0,0,0,262,260,1,0,0,0,262,261,1,0,0,0,263,11,1,0,0,0,264,266,3,
  	14,7,0,265,264,1,0,0,0,266,267,1,0,0,0,267,265,1,0,0,0,267,268,1,0,0,
  	0,268,13,1,0,0,0,269,276,3,20,10,0,270,276,3,70,35,0,271,276,3,16,8,0,
  	272,276,3,22,11,0,273,276,3,72,36,0,274,276,3,24,12,0,275,269,1,0,0,0,
  	275,270,1,0,0,0,275,271,1,0,0,0,275,272,1,0,0,0,275,273,1,0,0,0,275,274,
  	1,0,0,0,276,15,1,0,0,0,277,278,5,93,0,0,278,279,5,64,0,0,279,284,3,18,
  	9,0,280,281,5,3,0,0,281,283,3,18,9,0,282,280,1,0,0,0,283,286,1,0,0,0,
  	284,282,1,0,0,0,284,285,1,0,0,0,285,287,1,0,0,0,286,284,1,0,0,0,287,288,
  	5,65,0,0,288,17,1,0,0,0,289,291,3,14,7,0,290,289,1,0,0,0,291,292,1,0,
  	0,0,292,290,1,0,0,0,292,293,1,0,0,0,293,19,1,0,0,0,294,295,3,66,33,0,
  	295,296,5,2,0,0,296,297,3,72,36,0,297,21,1,0,0,0,298,299,3,62,31,0,299,
  	300,5,62,0,0,300,301,3,62,31,0,301,23,1,0,0,0,302,303,7,0,0,0,303,25,
  	1,0,0,0,304,305,3,72,36,0,305,27,1,0,0,0,306,307,5,97,0,0,307,29,1,0,
  	0,0,308,309,5,63,0,0,309,310,3,10,5,0,310,31,1,0,0,0,311,313,3,62,31,
  	0,312,314,3,30,15,0,313,312,1,0,0,0,313,314,1,0,0,0,314,33,1,0,0,0,315,
  	316,5,68,0,0,316,321,3,32,16,0,317,318,5,3,0,0,318,320,3,32,16,0,319,
  	317,1,0,0,0,320,323,1,0,0,0,321,319,1,0,0,0,321,322,1,0,0,0,322,324,1,
  	0,0,0,323,321,1,0,0,0,324,325,5,69,0,0,325,35,1,0,0,0,326,327,5,94,0,
  	0,327,37,1,0,0,0,328,330,5,68,0,0,329,331,3,36,18,0,330,329,1,0,0,0,330,
  	331,1,0,0,0,331,332,1,0,0,0,332,333,5,69,0,0,333,39,1,0,0,0,334,336,3,
  	34,17,0,335,334,1,0,0,0,336,339,1,0,0,0,337,338,1,0,0,0,337,335,1,0,0,
  	0,338,340,1,0,0,0,339,337,1,0,0,0,340,341,3,76,38,0,341,343,3,62,31,0,
  	342,344,3,38,19,0,343,342,1,0,0,0,343,344,1,0,0,0,344,347,1,0,0,0,345,
  	346,5,63,0,0,346,348,3,72,36,0,347,345,1,0,0,0,347,348,1,0,0,0,348,349,
  	1,0,0,0,349,350,5,60,0,0,350,41,1,0,0,0,351,352,5,87,0,0,352,353,3,62,
  	31,0,353,354,5,60,0,0,354,43,1,0,0,0,355,356,5,4,0,0,356,357,3,62,31,
  	0,357,358,5,63,0,0,358,359,3,72,36,0,359,360,5,60,0,0,360,45,1,0,0,0,
  	361,363,3,34,17,0,362,361,1,0,0,0,363,366,1,0,0,0,364,365,1,0,0,0,364,
  	362,1,0,0,0,365,367,1,0,0,0,366,364,1,0,0,0,367,368,5,5,0,0,368,371,3,
  	62,31,0,369,370,5,63,0,0,370,372,3,114,57,0,371,369,1,0,0,0,371,372,1,
  	0,0,0,372,373,1,0,0,0,373,374,5,60,0,0,374,47,1,0,0,0,375,377,3,34,17,
  	0,376,375,1,0,0,0,377,380,1,0,0,0,378,379,1,0,0,0,378,376,1,0,0,0,379,
  	381,1,0,0,0,380,378,1,0,0,0,381,382,5,6,0,0,382,383,5,63,0,0,383,384,
  	3,114,57,0,384,385,5,60,0,0,385,49,1,0,0,0,386,388,3,34,17,0,387,386,
  	1,0,0,0,388,391,1,0,0,0,389,390,1,0,0,0,389,387,1,0,0,0,390,392,1,0,0,
  	0,391,389,1,0,0,0,392,393,5,7,0,0,393,394,5,63,0,0,394,395,3,114,57,0,
  	395,396,5,60,0,0,396,51,1,0,0,0,397,398,5,100,0,0,398,53,1,0,0,0,399,
  	401,3,34,17,0,400,399,1,0,0,0,401,404,1,0,0,0,402,403,1,0,0,0,402,400,
  	1,0,0,0,403,405,1,0,0,0,404,402,1,0,0,0,405,406,3,194,97,0,406,407,5,
  	63,0,0,407,408,3,72,36,0,408,409,5,60,0,0,409,55,1,0,0,0,410,411,5,93,
  	0,0,411,57,1,0,0,0,412,421,3,56,28,0,413,417,5,51,0,0,414,416,3,68,34,
  	0,415,414,1,0,0,0,416,419,1,0,0,0,417,415,1,0,0,0,417,418,1,0,0,0,418,
  	420,1,0,0,0,419,417,1,0,0,0,420,422,5,50,0,0,421,413,1,0,0,0,421,422,
  	1,0,0,0,422,424,1,0,0,0,423,425,3,52,26,0,424,423,1,0,0,0,424,425,1,0,
  	0,0,425,59,1,0,0,0,426,427,5,93,0,0,427,61,1,0,0,0,428,429,5,93,0,0,429,
  	63,1,0,0,0,430,431,5,93,0,0,431,65,1,0,0,0,432,433,5,93,0,0,433,67,1,
  	0,0,0,434,435,5,93,0,0,435,69,1,0,0,0,436,437,5,93,0,0,437,439,5,64,0,
  	0,438,440,3,74,37,0,439,438,1,0,0,0,439,440,1,0,0,0,440,445,1,0,0,0,441,
  	442,5,3,0,0,442,444,3,74,37,0,443,441,1,0,0,0,444,447,1,0,0,0,445,443,
  	1,0,0,0,445,446,1,0,0,0,446,448,1,0,0,0,447,445,1,0,0,0,448,449,5,65,
  	0,0,449,71,1,0,0,0,450,458,3,192,96,0,451,458,5,93,0,0,452,458,5,94,0,
  	0,453,458,5,95,0,0,454,458,3,196,98,0,455,458,3,70,35,0,456,458,3,114,
  	57,0,457,450,1,0,0,0,457,451,1,0,0,0,457,452,1,0,0,0,457,453,1,0,0,0,
  	457,454,1,0,0,0,457,455,1,0,0,0,457,456,1,0,0,0,458,73,1,0,0,0,459,464,
  	5,93,0,0,460,464,5,94,0,0,461,464,5,95,0,0,462,464,3,196,98,0,463,459,
  	1,0,0,0,463,460,1,0,0,0,463,461,1,0,0,0,463,462,1,0,0,0,464,75,1,0,0,
  	0,465,466,3,58,29,0,466,77,1,0,0,0,467,468,5,104,0,0,468,79,1,0,0,0,469,
  	470,7,1,0,0,470,81,1,0,0,0,471,472,5,61,0,0,472,477,3,60,30,0,473,474,
  	5,3,0,0,474,476,3,60,30,0,475,473,1,0,0,0,476,479,1,0,0,0,477,478,1,0,
  	0,0,477,475,1,0,0,0,478,83,1,0,0,0,479,477,1,0,0,0,480,484,3,42,21,0,
  	481,484,3,44,22,0,482,484,5,98,0,0,483,480,1,0,0,0,483,481,1,0,0,0,483,
  	482,1,0,0,0,484,85,1,0,0,0,485,487,3,84,42,0,486,485,1,0,0,0,487,490,
  	1,0,0,0,488,486,1,0,0,0,488,489,1,0,0,0,489,87,1,0,0,0,490,488,1,0,0,
  	0,491,492,5,73,0,0,492,494,3,62,31,0,493,495,3,82,41,0,494,493,1,0,0,
  	0,494,495,1,0,0,0,495,496,1,0,0,0,496,497,5,66,0,0,497,498,3,86,43,0,
  	498,499,5,67,0,0,499,89,1,0,0,0,500,505,3,40,20,0,501,505,3,92,46,0,502,
  	505,3,78,39,0,503,505,5,98,0,0,504,500,1,0,0,0,504,501,1,0,0,0,504,502,
  	1,0,0,0,504,503,1,0,0,0,505,91,1,0,0,0,506,508,3,34,17,0,507,506,1,0,
  	0,0,508,511,1,0,0,0,509,510,1,0,0,0,509,507,1,0,0,0,510,512,1,0,0,0,511,
  	509,1,0,0,0,512,513,3,76,38,0,513,514,3,62,31,0,514,515,5,64,0,0,515,
  	516,3,94,47,0,516,518,5,65,0,0,517,519,3,96,48,0,518,517,1,0,0,0,518,
  	519,1,0,0,0,519,520,1,0,0,0,520,521,5,101,0,0,521,93,1,0,0,0,522,523,
  	5,64,0,0,523,524,3,94,47,0,524,525,5,65,0,0,525,528,1,0,0,0,526,528,8,
  	2,0,0,527,522,1,0,0,0,527,526,1,0,0,0,528,531,1,0,0,0,529,527,1,0,0,0,
  	529,530,1,0,0,0,530,95,1,0,0,0,531,529,1,0,0,0,532,533,5,61,0,0,533,534,
  	5,93,0,0,534,97,1,0,0,0,535,537,3,90,45,0,536,535,1,0,0,0,537,540,1,0,
  	0,0,538,536,1,0,0,0,538,539,1,0,0,0,539,99,1,0,0,0,540,538,1,0,0,0,541,
  	543,3,34,17,0,542,541,1,0,0,0,543,546,1,0,0,0,544,545,1,0,0,0,544,542,
  	1,0,0,0,545,547,1,0,0,0,546,544,1,0,0,0,547,548,5,74,0,0,548,550,3,62,
  	31,0,549,551,3,82,41,0,550,549,1,0,0,0,550,551,1,0,0,0,551,552,1,0,0,
  	0,552,553,5,66,0,0,553,554,3,98,49,0,554,555,5,67,0,0,555,101,1,0,0,0,
  	556,557,3,76,38,0,557,558,3,62,31,0,558,559,5,60,0,0,559,103,1,0,0,0,
  	560,561,5,90,0,0,561,562,3,62,31,0,562,563,5,60,0,0,563,105,1,0,0,0,564,
  	568,3,102,51,0,565,568,3,104,52,0,566,568,5,98,0,0,567,564,1,0,0,0,567,
  	565,1,0,0,0,567,566,1,0,0,0,568,107,1,0,0,0,569,571,3,106,53,0,570,569,
  	1,0,0,0,571,574,1,0,0,0,572,570,1,0,0,0,572,573,1,0,0,0,573,109,1,0,0,
  	0,574,572,1,0,0,0,575,576,5,88,0,0,576,577,3,62,31,0,577,578,5,66,0,0,
  	578,579,3,108,54,0,579,580,5,67,0,0,580,111,1,0,0,0,581,582,3,72,36,0,
  	582,113,1,0,0,0,583,584,5,66,0,0,584,589,3,112,56,0,585,586,5,3,0,0,586,
  	588,3,112,56,0,587,585,1,0,0,0,588,591,1,0,0,0,589,587,1,0,0,0,589,590,
  	1,0,0,0,590,592,1,0,0,0,591,589,1,0,0,0,592,593,5,67,0,0,593,115,1,0,
  	0,0,594,595,5,91,0,0,595,596,5,63,0,0,596,597,3,62,31,0,597,598,5,60,
  	0,0,598,117,1,0,0,0,599,601,3,34,17,0,600,599,1,0,0,0,601,604,1,0,0,0,
  	602,603,1,0,0,0,602,600,1,0,0,0,603,605,1,0,0,0,604,602,1,0,0,0,605,606,
  	3,192,96,0,606,607,5,63,0,0,607,608,3,80,40,0,608,609,5,60,0,0,609,119,
  	1,0,0,0,610,615,3,116,58,0,611,615,3,118,59,0,612,615,3,46,23,0,613,615,
  	5,98,0,0,614,610,1,0,0,0,614,611,1,0,0,0,614,612,1,0,0,0,614,613,1,0,
  	0,0,615,121,1,0,0,0,616,618,3,120,60,0,617,616,1,0,0,0,618,621,1,0,0,
  	0,619,617,1,0,0,0,619,620,1,0,0,0,620,123,1,0,0,0,621,619,1,0,0,0,622,
  	624,3,34,17,0,623,622,1,0,0,0,624,627,1,0,0,0,625,626,1,0,0,0,625,623,
  	1,0,0,0,626,628,1,0,0,0,627,625,1,0,0,0,628,629,5,76,0,0,629,631,3,62,
  	31,0,630,632,3,82,41,0,631,630,1,0,0,0,631,632,1,0,0,0,632,633,1,0,0,
  	0,633,634,5,66,0,0,634,635,3,122,61,0,635,636,5,67,0,0,636,125,1,0,0,
  	0,637,645,3,116,58,0,638,645,3,118,59,0,639,645,3,46,23,0,640,645,3,48,
  	24,0,641,645,3,50,25,0,642,645,3,54,27,0,643,645,5,98,0,0,644,637,1,0,
  	0,0,644,638,1,0,0,0,644,639,1,0,0,0,644,640,1,0,0,0,644,641,1,0,0,0,644,
  	642,1,0,0,0,644,643,1,0,0,0,645,127,1,0,0,0,646,648,3,126,63,0,647,646,
  	1,0,0,0,648,651,1,0,0,0,649,647,1,0,0,0,649,650,1,0,0,0,650,129,1,0,0,
  	0,651,649,1,0,0,0,652,654,3,34,17,0,653,652,1,0,0,0,654,657,1,0,0,0,655,
  	656,1,0,0,0,655,653,1,0,0,0,656,658,1,0,0,0,657,655,1,0,0,0,658,659,5,
  	77,0,0,659,661,3,62,31,0,660,662,3,82,41,0,661,660,1,0,0,0,661,662,1,
  	0,0,0,662,663,1,0,0,0,663,664,5,66,0,0,664,665,3,128,64,0,665,666,5,67,
  	0,0,666,131,1,0,0,0,667,670,3,116,58,0,668,670,5,98,0,0,669,667,1,0,0,
  	0,669,668,1,0,0,0,670,133,1,0,0,0,671,673,3,132,66,0,672,671,1,0,0,0,
  	673,676,1,0,0,0,674,672,1,0,0,0,674,675,1,0,0,0,675,135,1,0,0,0,676,674,
  	1,0,0,0,677,678,5,78,0,0,678,680,3,62,31,0,679,681,3,82,41,0,680,679,
  	1,0,0,0,680,681,1,0,0,0,681,682,1,0,0,0,682,683,5,66,0,0,683,684,3,134,
  	67,0,684,685,5,67,0,0,685,137,1,0,0,0,686,687,7,3,0,0,687,139,1,0,0,0,
  	688,689,3,138,69,0,689,690,5,63,0,0,690,691,3,72,36,0,691,692,5,60,0,
  	0,692,141,1,0,0,0,693,695,3,34,17,0,694,693,1,0,0,0,695,698,1,0,0,0,696,
  	697,1,0,0,0,696,694,1,0,0,0,697,699,1,0,0,0,698,696,1,0,0,0,699,700,5,
  	81,0,0,700,701,3,76,38,0,701,702,3,62,31,0,702,703,5,60,0,0,703,143,1,
  	0,0,0,704,708,3,140,70,0,705,708,3,142,71,0,706,708,5,98,0,0,707,704,
  	1,0,0,0,707,705,1,0,0,0,707,706,1,0,0,0,708,145,1,0,0,0,709,711,3,144,
  	72,0,710,709,1,0,0,0,711,714,1,0,0,0,712,710,1,0,0,0,712,713,1,0,0,0,
  	713,147,1,0,0,0,714,712,1,0,0,0,715,716,5,80,0,0,716,717,3,62,31,0,717,
  	718,5,66,0,0,718,719,3,146,73,0,719,720,5,67,0,0,720,149,1,0,0,0,721,
  	727,3,116,58,0,722,727,3,118,59,0,723,727,3,46,23,0,724,727,3,148,74,
  	0,725,727,5,98,0,0,726,721,1,0,0,0,726,722,1,0,0,0,726,723,1,0,0,0,726,
  	724,1,0,0,0,726,725,1,0,0,0,727,151,1,0,0,0,728,730,3,150,75,0,729,728,
  	1,0,0,0,730,733,1,0,0,0,731,729,1,0,0,0,731,732,1,0,0,0,732,153,1,0,0,
  	0,733,731,1,0,0,0,734,736,3,34,17,0,735,734,1,0,0,0,736,739,1,0,0,0,737,
  	738,1,0,0,0,737,735,1,0,0,0,738,740,1,0,0,0,739,737,1,0,0,0,740,741,5,
  	79,0,0,741,743,3,62,31,0,742,744,3,82,41,0,743,742,1,0,0,0,743,744,1,
  	0,0,0,744,745,1,0,0,0,745,746,5,66,0,0,746,747,3,152,76,0,747,748,5,67,
  	0,0,748,155,1,0,0,0,749,753,3,118,59,0,750,753,5,98,0,0,751,753,3,54,
  	27,0,752,749,1,0,0,0,752,750,1,0,0,0,752,751,1,0,0,0,753,157,1,0,0,0,
  	754,756,3,156,78,0,755,754,1,0,0,0,756,759,1,0,0,0,757,755,1,0,0,0,757,
  	758,1,0,0,0,758,159,1,0,0,0,759,757,1,0,0,0,760,762,3,34,17,0,761,760,
  	1,0,0,0,762,765,1,0,0,0,763,764,1,0,0,0,763,761,1,0,0,0,764,766,1,0,0,
  	0,765,763,1,0,0,0,766,767,5,83,0,0,767,769,3,62,31,0,768,770,3,82,41,
  	0,769,768,1,0,0,0,769,770,1,0,0,0,770,771,1,0,0,0,771,772,5,66,0,0,772,
  	773,3,158,79,0,773,774,5,67,0,0,774,161,1,0,0,0,775,778,3,118,59,0,776,
  	778,5,98,0,0,777,775,1,0,0,0,777,776,1,0,0,0,778,163,1,0,0,0,779,781,
  	3,162,81,0,780,779,1,0,0,0,781,784,1,0,0,0,782,780,1,0,0,0,782,783,1,
  	0,0,0,783,165,1,0,0,0,784,782,1,0,0,0,785,787,3,34,17,0,786,785,1,0,0,
  	0,787,790,1,0,0,0,788,789,1,0,0,0,788,786,1,0,0,0,789,791,1,0,0,0,790,
  	788,1,0,0,0,791,792,5,82,0,0,792,794,3,62,31,0,793,795,3,82,41,0,794,
  	793,1,0,0,0,794,795,1,0,0,0,795,796,1,0,0,0,796,797,5,66,0,0,797,798,
  	3,164,82,0,798,799,5,67,0,0,799,167,1,0,0,0,800,802,3,34,17,0,801,800,
  	1,0,0,0,802,805,1,0,0,0,803,804,1,0,0,0,803,801,1,0,0,0,804,806,1,0,0,
  	0,805,803,1,0,0,0,806,807,3,76,38,0,807,808,3,62,31,0,808,809,5,60,0,
  	0,809,169,1,0,0,0,810,813,3,168,84,0,811,813,5,98,0,0,812,810,1,0,0,0,
  	812,811,1,0,0,0,813,171,1,0,0,0,814,816,3,170,85,0,815,814,1,0,0,0,816,
  	819,1,0,0,0,817,815,1,0,0,0,817,818,1,0,0,0,818,173,1,0,0,0,819,817,1,
  	0,0,0,820,822,3,34,17,0,821,820,1,0,0,0,822,825,1,0,0,0,823,824,1,0,0,
  	0,823,821,1,0,0,0,824,826,1,0,0,0,825,823,1,0,0,0,826,827,5,85,0,0,827,
  	829,3,62,31,0,828,830,3,82,41,0,829,828,1,0,0,0,829,830,1,0,0,0,830,831,
  	1,0,0,0,831,832,5,66,0,0,832,833,3,172,86,0,833,834,5,67,0,0,834,175,
  	1,0,0,0,835,837,3,34,17,0,836,835,1,0,0,0,837,840,1,0,0,0,838,839,1,0,
  	0,0,838,836,1,0,0,0,839,841,1,0,0,0,840,838,1,0,0,0,841,842,5,84,0,0,
  	842,844,3,62,31,0,843,845,3,82,41,0,844,843,1,0,0,0,844,845,1,0,0,0,845,
  	846,1,0,0,0,846,847,5,66,0,0,847,848,3,172,86,0,848,849,5,67,0,0,849,
  	177,1,0,0,0,850,852,3,34,17,0,851,850,1,0,0,0,852,855,1,0,0,0,853,851,
  	1,0,0,0,853,854,1,0,0,0,854,856,1,0,0,0,855,853,1,0,0,0,856,857,3,62,
  	31,0,857,858,5,60,0,0,858,861,1,0,0,0,859,861,5,98,0,0,860,853,1,0,0,
  	0,860,859,1,0,0,0,861,179,1,0,0,0,862,864,3,178,89,0,863,862,1,0,0,0,
  	864,867,1,0,0,0,865,863,1,0,0,0,865,866,1,0,0,0,866,181,1,0,0,0,867,865,
  	1,0,0,0,868,869,5,86,0,0,869,870,3,62,31,0,870,871,5,66,0,0,871,872,3,
  	180,90,0,872,873,5,67,0,0,873,183,1,0,0,0,874,877,3,62,31,0,875,876,5,
  	63,0,0,876,878,3,72,36,0,877,875,1,0,0,0,877,878,1,0,0,0,878,879,1,0,
  	0,0,879,880,5,60,0,0,880,185,1,0,0,0,881,884,3,184,92,0,882,884,5,98,
  	0,0,883,881,1,0,0,0,883,882,1,0,0,0,884,187,1,0,0,0,885,887,3,186,93,
  	0,886,885,1,0,0,0,887,890,1,0,0,0,888,886,1,0,0,0,888,889,1,0,0,0,889,
  	189,1,0,0,0,890,888,1,0,0,0,891,892,5,92,0,0,892,893,3,62,31,0,893,894,
  	5,66,0,0,894,895,3,188,94,0,895,896,5,67,0,0,896,191,1,0,0,0,897,898,
  	7,4,0,0,898,193,1,0,0,0,899,900,7,5,0,0,900,195,1,0,0,0,901,902,7,6,0,
  	0,902,197,1,0,0,0,84,201,222,229,237,251,258,262,267,275,284,292,313,
  	321,330,337,343,347,364,371,378,389,402,417,421,424,439,445,457,463,477,
  	483,488,494,504,509,518,527,529,538,544,550,567,572,589,602,614,619,625,
  	631,644,649,655,661,669,674,680,696,707,712,726,731,737,743,752,757,763,
  	769,777,782,788,794,803,812,817,823,829,838,844,853,860,865,877,883,888
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

std::vector<PrismParser::DefinitionContext *> PrismParser::ParseContext::definition() {
  return getRuleContexts<PrismParser::DefinitionContext>();
}

PrismParser::DefinitionContext* PrismParser::ParseContext::definition(size_t i) {
  return getRuleContext<PrismParser::DefinitionContext>(i);
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
    setState(201);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1092079585) != 0) {
      setState(198);
      definition();
      setState(203);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(204);
    match(PrismParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DefinitionContext ------------------------------------------------------------------

PrismParser::DefinitionContext::DefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Layout_definitionContext* PrismParser::DefinitionContext::layout_definition() {
  return getRuleContext<PrismParser::Layout_definitionContext>(0);
}

PrismParser::Table_definitionContext* PrismParser::DefinitionContext::table_definition() {
  return getRuleContext<PrismParser::Table_definitionContext>(0);
}

PrismParser::Rt_definitionContext* PrismParser::DefinitionContext::rt_definition() {
  return getRuleContext<PrismParser::Rt_definitionContext>(0);
}

PrismParser::Workgraph_pso_definitionContext* PrismParser::DefinitionContext::workgraph_pso_definition() {
  return getRuleContext<PrismParser::Workgraph_pso_definitionContext>(0);
}

PrismParser::Compute_pso_definitionContext* PrismParser::DefinitionContext::compute_pso_definition() {
  return getRuleContext<PrismParser::Compute_pso_definitionContext>(0);
}

PrismParser::Graphics_pso_definitionContext* PrismParser::DefinitionContext::graphics_pso_definition() {
  return getRuleContext<PrismParser::Graphics_pso_definitionContext>(0);
}

PrismParser::Rtx_pso_definitionContext* PrismParser::DefinitionContext::rtx_pso_definition() {
  return getRuleContext<PrismParser::Rtx_pso_definitionContext>(0);
}

PrismParser::Rtx_pass_definitionContext* PrismParser::DefinitionContext::rtx_pass_definition() {
  return getRuleContext<PrismParser::Rtx_pass_definitionContext>(0);
}

PrismParser::Rtx_raygen_definitionContext* PrismParser::DefinitionContext::rtx_raygen_definition() {
  return getRuleContext<PrismParser::Rtx_raygen_definitionContext>(0);
}

PrismParser::Pass_definitionContext* PrismParser::DefinitionContext::pass_definition() {
  return getRuleContext<PrismParser::Pass_definitionContext>(0);
}

PrismParser::View_definitionContext* PrismParser::DefinitionContext::view_definition() {
  return getRuleContext<PrismParser::View_definitionContext>(0);
}

PrismParser::Pipeline_definitionContext* PrismParser::DefinitionContext::pipeline_definition() {
  return getRuleContext<PrismParser::Pipeline_definitionContext>(0);
}

PrismParser::Enum_definitionContext* PrismParser::DefinitionContext::enum_definition() {
  return getRuleContext<PrismParser::Enum_definitionContext>(0);
}

PrismParser::Const_definitionContext* PrismParser::DefinitionContext::const_definition() {
  return getRuleContext<PrismParser::Const_definitionContext>(0);
}

PrismParser::Namespace_definitionContext* PrismParser::DefinitionContext::namespace_definition() {
  return getRuleContext<PrismParser::Namespace_definitionContext>(0);
}

tree::TerminalNode* PrismParser::DefinitionContext::COMMENT() {
  return getToken(PrismParser::COMMENT, 0);
}


size_t PrismParser::DefinitionContext::getRuleIndex() const {
  return PrismParser::RuleDefinition;
}

void PrismParser::DefinitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefinition(this);
}

void PrismParser::DefinitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefinition(this);
}


std::any PrismParser::DefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitDefinition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::DefinitionContext* PrismParser::definition() {
  DefinitionContext *_localctx = _tracker.createInstance<DefinitionContext>(_ctx, getState());
  enterRule(_localctx, 2, PrismParser::RuleDefinition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(222);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(206);
      layout_definition();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(207);
      table_definition();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(208);
      rt_definition();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(209);
      workgraph_pso_definition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(210);
      compute_pso_definition();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(211);
      graphics_pso_definition();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(212);
      rtx_pso_definition();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(213);
      rtx_pass_definition();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(214);
      rtx_raygen_definition();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(215);
      pass_definition();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(216);
      view_definition();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(217);
      pipeline_definition();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(218);
      enum_definition();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(219);
      const_definition();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(220);
      namespace_definition();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(221);
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

//----------------- Namespace_definitionContext ------------------------------------------------------------------

PrismParser::Namespace_definitionContext::Namespace_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PrismParser::Namespace_headerContext* PrismParser::Namespace_definitionContext::namespace_header() {
  return getRuleContext<PrismParser::Namespace_headerContext>(0);
}

tree::TerminalNode* PrismParser::Namespace_definitionContext::OBRACE() {
  return getToken(PrismParser::OBRACE, 0);
}

tree::TerminalNode* PrismParser::Namespace_definitionContext::CBRACE() {
  return getToken(PrismParser::CBRACE, 0);
}

std::vector<PrismParser::DefinitionContext *> PrismParser::Namespace_definitionContext::definition() {
  return getRuleContexts<PrismParser::DefinitionContext>();
}

PrismParser::DefinitionContext* PrismParser::Namespace_definitionContext::definition(size_t i) {
  return getRuleContext<PrismParser::DefinitionContext>(i);
}


size_t PrismParser::Namespace_definitionContext::getRuleIndex() const {
  return PrismParser::RuleNamespace_definition;
}

void PrismParser::Namespace_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNamespace_definition(this);
}

void PrismParser::Namespace_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNamespace_definition(this);
}


std::any PrismParser::Namespace_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNamespace_definition(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Namespace_definitionContext* PrismParser::namespace_definition() {
  Namespace_definitionContext *_localctx = _tracker.createInstance<Namespace_definitionContext>(_ctx, getState());
  enterRule(_localctx, 4, PrismParser::RuleNamespace_definition);
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
    setState(224);
    namespace_header();
    setState(225);
    match(PrismParser::OBRACE);
    setState(229);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1092079585) != 0) {
      setState(226);
      definition();
      setState(231);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(232);
    match(PrismParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Namespace_headerContext ------------------------------------------------------------------

PrismParser::Namespace_headerContext::Namespace_headerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PrismParser::Namespace_headerContext::NAMESPACE() {
  return getToken(PrismParser::NAMESPACE, 0);
}

PrismParser::Name_idContext* PrismParser::Namespace_headerContext::name_id() {
  return getRuleContext<PrismParser::Name_idContext>(0);
}

std::vector<PrismParser::Option_blockContext *> PrismParser::Namespace_headerContext::option_block() {
  return getRuleContexts<PrismParser::Option_blockContext>();
}

PrismParser::Option_blockContext* PrismParser::Namespace_headerContext::option_block(size_t i) {
  return getRuleContext<PrismParser::Option_blockContext>(i);
}


size_t PrismParser::Namespace_headerContext::getRuleIndex() const {
  return PrismParser::RuleNamespace_header;
}

void PrismParser::Namespace_headerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNamespace_header(this);
}

void PrismParser::Namespace_headerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<PrismListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNamespace_header(this);
}


std::any PrismParser::Namespace_headerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PrismVisitor*>(visitor))
    return parserVisitor->visitNamespace_header(this);
  else
    return visitor->visitChildren(this);
}

PrismParser::Namespace_headerContext* PrismParser::namespace_header() {
  Namespace_headerContext *_localctx = _tracker.createInstance<Namespace_headerContext>(_ctx, getState());
  enterRule(_localctx, 6, PrismParser::RuleNamespace_header);

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
    setState(237);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(234);
        option_block(); 
      }
      setState(239);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    }
    setState(240);
    match(PrismParser::NAMESPACE);
    setState(241);
    name_id();
   
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
  enterRule(_localctx, 8, PrismParser::RuleConst_definition);

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
    match(PrismParser::T__0);
    setState(244);
    name_id();
    setState(245);
    options_assign();
    setState(246);
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
  enterRule(_localctx, 10, PrismParser::RuleBind_option);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(262);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(251);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
      case 1: {
        setState(248);
        owner_id();
        setState(249);
        match(PrismParser::T__1);
        break;
      }

      default:
        break;
      }
      setState(253);
      flag_value_holder();
      setState(256); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(254);
        match(PrismParser::PIPE);
        setState(255);
        flag_value_holder();
        setState(258); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == PrismParser::PIPE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(260);
      raw_value();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(261);
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
  enterRule(_localctx, 12, PrismParser::RuleCond_expr);
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
    setState(265); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(264);
      cond_term();
      setState(267); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 864515206661791744) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 72477573319) != 0);
   
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
  enterRule(_localctx, 14, PrismParser::RuleCond_term);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(275);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(269);
      qualified_ref();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(270);
      function_id();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(271);
      call();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(272);
      member_ref();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(273);
      value_id();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(274);
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
  enterRule(_localctx, 16, PrismParser::RuleCall);
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
    match(PrismParser::ID);
    setState(278);
    match(PrismParser::OPAR);
    setState(279);
    call_arg();
    setState(284);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(280);
      match(PrismParser::T__2);
      setState(281);
      call_arg();
      setState(286);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(287);
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
  enterRule(_localctx, 18, PrismParser::RuleCall_arg);

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
    setState(290); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(289);
              cond_term();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(292); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
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
  enterRule(_localctx, 20, PrismParser::RuleQualified_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(294);
    owner_id();
    setState(295);
    match(PrismParser::T__1);
    setState(296);
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
  enterRule(_localctx, 22, PrismParser::RuleMember_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(298);
    name_id();
    setState(299);
    match(PrismParser::DOT);
    setState(300);
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
  enterRule(_localctx, 24, PrismParser::RuleCond_op);
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
    setState(302);
    _la = _input->LA(1);
    if (!((((_la - 45) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 45)) & 36028797020561403) != 0)) {
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
  enterRule(_localctx, 26, PrismParser::RuleFlag_value_holder);

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
  enterRule(_localctx, 28, PrismParser::RuleRaw_value);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(306);
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
  enterRule(_localctx, 30, PrismParser::RuleOptions_assign);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(308);
    match(PrismParser::ASSIGN);
    setState(309);
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
  enterRule(_localctx, 32, PrismParser::RuleOption);
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
    setState(311);
    name_id();
    setState(313);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(312);
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
  enterRule(_localctx, 34, PrismParser::RuleOption_block);
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
    setState(315);
    match(PrismParser::OSBRACE);
    setState(316);
    option();
    setState(321);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(317);
      match(PrismParser::T__2);
      setState(318);
      option();
      setState(323);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(324);
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
  enterRule(_localctx, 36, PrismParser::RuleArray_count_id);

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
  enterRule(_localctx, 38, PrismParser::RuleArray);
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
    setState(328);
    match(PrismParser::OSBRACE);
    setState(330);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::INT_SCALAR) {
      setState(329);
      array_count_id();
    }
    setState(332);
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
  enterRule(_localctx, 40, PrismParser::RuleValue_declaration);
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
    setState(337);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(334);
        option_block(); 
      }
      setState(339);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx);
    }
    setState(340);
    type_id();
    setState(341);
    name_id();
    setState(343);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::OSBRACE) {
      setState(342);
      array();
    }
    setState(347);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(345);
      match(PrismParser::ASSIGN);
      setState(346);
      value_id();
    }
    setState(349);
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
  enterRule(_localctx, 42, PrismParser::RuleSlot_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(351);
    match(PrismParser::SLOT);
    setState(352);
    name_id();
    setState(353);
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
  enterRule(_localctx, 44, PrismParser::RuleSampler_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(355);
    match(PrismParser::T__3);
    setState(356);
    name_id();
    setState(357);
    match(PrismParser::ASSIGN);
    setState(358);
    value_id();
    setState(359);
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
  enterRule(_localctx, 46, PrismParser::RuleDefine_declaration);
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
    setState(364);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(361);
        option_block(); 
      }
      setState(366);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    }
    setState(367);
    match(PrismParser::T__4);
    setState(368);
    name_id();
    setState(371);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(369);
      match(PrismParser::ASSIGN);
      setState(370);
      array_value_ids();
    }
    setState(373);
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
  enterRule(_localctx, 48, PrismParser::RuleRtv_formats_declaration);

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
    setState(378);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(375);
        option_block(); 
      }
      setState(380);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    }
    setState(381);
    match(PrismParser::T__5);
    setState(382);
    match(PrismParser::ASSIGN);
    setState(383);
    array_value_ids();
    setState(384);
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
  enterRule(_localctx, 50, PrismParser::RuleBlends_declaration);

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
    setState(389);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(386);
        option_block(); 
      }
      setState(391);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx);
    }
    setState(392);
    match(PrismParser::T__6);
    setState(393);
    match(PrismParser::ASSIGN);
    setState(394);
    array_value_ids();
    setState(395);
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
  enterRule(_localctx, 52, PrismParser::RulePointer);

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
  enterRule(_localctx, 54, PrismParser::RulePso_param);

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
    setState(402);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(399);
        option_block(); 
      }
      setState(404);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
    setState(405);
    pso_param_id();
    setState(406);
    match(PrismParser::ASSIGN);
    setState(407);
    value_id();
    setState(408);
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
  enterRule(_localctx, 56, PrismParser::RuleClass_no_template);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(410);
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
  enterRule(_localctx, 58, PrismParser::RuleType_with_template);
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
    setState(412);
    class_no_template();
    setState(421);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::LT) {
      setState(413);
      match(PrismParser::LT);
      setState(417);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == PrismParser::ID) {
        setState(414);
        template_id();
        setState(419);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(420);
      match(PrismParser::GT);
    }
    setState(424);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::POINTER) {
      setState(423);
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
  enterRule(_localctx, 60, PrismParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(426);
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
  enterRule(_localctx, 62, PrismParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(428);
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
  enterRule(_localctx, 64, PrismParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(430);
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
  enterRule(_localctx, 66, PrismParser::RuleOwner_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(432);
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
  enterRule(_localctx, 68, PrismParser::RuleTemplate_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(434);
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
  enterRule(_localctx, 70, PrismParser::RuleFunction_id);
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
    setState(436);
    match(PrismParser::ID);
    setState(437);
    match(PrismParser::OPAR);
    setState(439);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la - 70) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 70)) & 58720259) != 0) {
      setState(438);
      value_id_ignore();
    }
    setState(445);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(441);
      match(PrismParser::T__2);
      setState(442);
      value_id_ignore();
      setState(447);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(448);
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
  enterRule(_localctx, 72, PrismParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(457);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(450);
      shader_type();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(451);
      match(PrismParser::ID);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(452);
      match(PrismParser::INT_SCALAR);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(453);
      match(PrismParser::FLOAT_SCALAR);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(454);
      bool_type();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(455);
      function_id();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(456);
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
  enterRule(_localctx, 74, PrismParser::RuleValue_id_ignore);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(463);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(459);
        match(PrismParser::ID);
        break;
      }

      case PrismParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(460);
        match(PrismParser::INT_SCALAR);
        break;
      }

      case PrismParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(461);
        match(PrismParser::FLOAT_SCALAR);
        break;
      }

      case PrismParser::TRUE:
      case PrismParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(462);
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
  enterRule(_localctx, 76, PrismParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(465);
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
  enterRule(_localctx, 78, PrismParser::RuleInsert_block);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(467);
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
  enterRule(_localctx, 80, PrismParser::RuleShader_path);
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
    setState(469);
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
  enterRule(_localctx, 82, PrismParser::RuleInherit);

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
    setState(471);
    match(PrismParser::COLON);
    setState(472);
    inherit_id();
    setState(477);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(473);
        match(PrismParser::T__2);
        setState(474);
        inherit_id(); 
      }
      setState(479);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx);
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
  enterRule(_localctx, 84, PrismParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(483);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(480);
        slot_declaration();
        break;
      }

      case PrismParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(481);
        sampler_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(482);
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
  enterRule(_localctx, 86, PrismParser::RuleLayout_block);
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
    setState(488);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__3 || _la == PrismParser::SLOT

    || _la == PrismParser::COMMENT) {
      setState(485);
      layout_stat();
      setState(490);
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
  enterRule(_localctx, 88, PrismParser::RuleLayout_definition);
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
    setState(491);
    match(PrismParser::LAYOUT);
    setState(492);
    name_id();
    setState(494);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(493);
      inherit();
    }
    setState(496);
    match(PrismParser::OBRACE);
    setState(497);
    layout_block();
    setState(498);
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
  enterRule(_localctx, 90, PrismParser::RuleTable_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(504);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(500);
      value_declaration();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(501);
      function_definition();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(502);
      insert_block();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(503);
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
  enterRule(_localctx, 92, PrismParser::RuleFunction_definition);
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
    setState(509);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(506);
        option_block(); 
      }
      setState(511);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    }
    setState(512);
    type_id();
    setState(513);
    name_id();
    setState(514);
    match(PrismParser::OPAR);
    setState(515);
    function_params();
    setState(516);
    match(PrismParser::CPAR);
    setState(518);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(517);
      function_semantic();
    }
    setState(520);
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
  enterRule(_localctx, 94, PrismParser::RuleFunction_params);
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
    setState(529);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & -2) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 2199023255549) != 0) {
      setState(527);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case PrismParser::OPAR: {
          setState(522);
          match(PrismParser::OPAR);
          setState(523);
          function_params();
          setState(524);
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
        case PrismParser::NAMESPACE:
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
          setState(526);
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
      setState(531);
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
  enterRule(_localctx, 96, PrismParser::RuleFunction_semantic);

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
    match(PrismParser::COLON);
    setState(533);
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
  enterRule(_localctx, 98, PrismParser::RuleTable_block);
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
    setState(538);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 69826772993) != 0) {
      setState(535);
      table_stat();
      setState(540);
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
  enterRule(_localctx, 100, PrismParser::RuleTable_definition);
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
    setState(544);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(541);
        option_block(); 
      }
      setState(546);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
    setState(547);
    match(PrismParser::STRUCT);
    setState(548);
    name_id();
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(549);
      inherit();
    }
    setState(552);
    match(PrismParser::OBRACE);
    setState(553);
    table_block();
    setState(554);
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
  enterRule(_localctx, 102, PrismParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(556);
    type_id();
    setState(557);
    name_id();
    setState(558);
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
  enterRule(_localctx, 104, PrismParser::RuleRt_ds_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(560);
    match(PrismParser::DSV);
    setState(561);
    name_id();
    setState(562);
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
  enterRule(_localctx, 106, PrismParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(567);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(564);
        rt_color_declaration();
        break;
      }

      case PrismParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(565);
        rt_ds_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(566);
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
  enterRule(_localctx, 108, PrismParser::RuleRt_block);
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
    setState(572);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 90) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 90)) & 265) != 0) {
      setState(569);
      rt_stat();
      setState(574);
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
  enterRule(_localctx, 110, PrismParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(575);
    match(PrismParser::RT);
    setState(576);
    name_id();
    setState(577);
    match(PrismParser::OBRACE);
    setState(578);
    rt_block();
    setState(579);
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
  enterRule(_localctx, 112, PrismParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(581);
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
  enterRule(_localctx, 114, PrismParser::RuleArray_value_ids);
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
    setState(583);
    match(PrismParser::OBRACE);
    setState(584);
    array_value_holder();
    setState(589);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::T__2) {
      setState(585);
      match(PrismParser::T__2);
      setState(586);
      array_value_holder();
      setState(591);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(592);
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
  enterRule(_localctx, 116, PrismParser::RuleRoot_sig);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(594);
    match(PrismParser::ROOTSIG);
    setState(595);
    match(PrismParser::ASSIGN);
    setState(596);
    name_id();
    setState(597);
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
  enterRule(_localctx, 118, PrismParser::RuleShader);

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
    setState(602);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(599);
        option_block(); 
      }
      setState(604);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    }
    setState(605);
    shader_type();
    setState(606);
    match(PrismParser::ASSIGN);
    setState(607);
    shader_path();
    setState(608);
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
  enterRule(_localctx, 120, PrismParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(614);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(610);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(611);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(612);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(613);
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
  enterRule(_localctx, 122, PrismParser::RuleCompute_pso_block);
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
    setState(619);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1082130433) != 0) {
      setState(616);
      compute_pso_stat();
      setState(621);
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
  enterRule(_localctx, 124, PrismParser::RuleCompute_pso_definition);
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
    setState(625);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(622);
        option_block(); 
      }
      setState(627);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    }
    setState(628);
    match(PrismParser::COMPUTE_PSO);
    setState(629);
    name_id();
    setState(631);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(630);
      inherit();
    }
    setState(633);
    match(PrismParser::OBRACE);
    setState(634);
    compute_pso_block();
    setState(635);
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
  enterRule(_localctx, 126, PrismParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(644);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(637);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(638);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(639);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(640);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(641);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(642);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(643);
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
  enterRule(_localctx, 128, PrismParser::RuleGraphics_pso_block);
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
    setState(649);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080864) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1082130433) != 0) {
      setState(646);
      graphics_pso_stat();
      setState(651);
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
  enterRule(_localctx, 130, PrismParser::RuleGraphics_pso_definition);
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
    setState(655);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(652);
        option_block(); 
      }
      setState(657);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx);
    }
    setState(658);
    match(PrismParser::GRAPHICS_PSO);
    setState(659);
    name_id();
    setState(661);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(660);
      inherit();
    }
    setState(663);
    match(PrismParser::OBRACE);
    setState(664);
    graphics_pso_block();
    setState(665);
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
  enterRule(_localctx, 132, PrismParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(669);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(667);
        root_sig();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(668);
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
  enterRule(_localctx, 134, PrismParser::RuleRtx_pso_block);
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
    setState(674);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ROOTSIG

    || _la == PrismParser::COMMENT) {
      setState(671);
      rtx_pso_stat();
      setState(676);
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
  enterRule(_localctx, 136, PrismParser::RuleRtx_pso_definition);
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
    setState(677);
    match(PrismParser::RAYTRACE_PSO);
    setState(678);
    name_id();
    setState(680);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(679);
      inherit();
    }
    setState(682);
    match(PrismParser::OBRACE);
    setState(683);
    rtx_pso_block();
    setState(684);
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
  enterRule(_localctx, 138, PrismParser::RuleNode_param_id);
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
    setState(686);
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
  enterRule(_localctx, 140, PrismParser::RuleNode_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(688);
    node_param_id();
    setState(689);
    match(PrismParser::ASSIGN);
    setState(690);
    value_id();
    setState(691);
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
  enterRule(_localctx, 142, PrismParser::RuleNode_output_decl);

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
    setState(696);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(693);
        option_block(); 
      }
      setState(698);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx);
    }
    setState(699);
    match(PrismParser::NODE_OUTPUT);
    setState(700);
    type_id();
    setState(701);
    name_id();
    setState(702);
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
  enterRule(_localctx, 144, PrismParser::RuleNode_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(707);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::T__7:
      case PrismParser::T__8:
      case PrismParser::T__9:
      case PrismParser::T__10:
      case PrismParser::T__11: {
        enterOuterAlt(_localctx, 1);
        setState(704);
        node_param();
        break;
      }

      case PrismParser::OSBRACE:
      case PrismParser::NODE_OUTPUT: {
        enterOuterAlt(_localctx, 2);
        setState(705);
        node_output_decl();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(706);
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
  enterRule(_localctx, 146, PrismParser::RuleNode_block);
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
    setState(712);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7936) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1073750017) != 0) {
      setState(709);
      node_stat();
      setState(714);
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
  enterRule(_localctx, 148, PrismParser::RuleNode_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(715);
    match(PrismParser::NODE);
    setState(716);
    name_id();
    setState(717);
    match(PrismParser::OBRACE);
    setState(718);
    node_block();
    setState(719);
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
  enterRule(_localctx, 150, PrismParser::RuleWorkgraph_pso_stat);

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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(721);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(722);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(723);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(724);
      node_definition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(725);
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
  enterRule(_localctx, 152, PrismParser::RuleWorkgraph_pso_block);
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
      ((1ULL << _la) & 67100704) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1082134529) != 0) {
      setState(728);
      workgraph_pso_stat();
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
  enterRule(_localctx, 154, PrismParser::RuleWorkgraph_pso_definition);
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
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(734);
        option_block(); 
      }
      setState(739);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
    }
    setState(740);
    match(PrismParser::WORKGRAPH_PSO);
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
    workgraph_pso_block();
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
  enterRule(_localctx, 156, PrismParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(752);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(749);
      shader();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(750);
      match(PrismParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(751);
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
  enterRule(_localctx, 158, PrismParser::RuleRtx_pass_block);
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
    setState(757);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 35184372080640) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(754);
      rtx_pass_stat();
      setState(759);
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
  enterRule(_localctx, 160, PrismParser::RuleRtx_pass_definition);
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
    setState(763);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(760);
        option_block(); 
      }
      setState(765);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
    }
    setState(766);
    match(PrismParser::RAYTRACE_PASS);
    setState(767);
    name_id();
    setState(769);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(768);
      inherit();
    }
    setState(771);
    match(PrismParser::OBRACE);
    setState(772);
    rtx_pass_block();
    setState(773);
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
  enterRule(_localctx, 162, PrismParser::RuleRtx_raygen_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(777);
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
        setState(775);
        shader();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(776);
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
  enterRule(_localctx, 164, PrismParser::RuleRtx_raygen_block);
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
    setState(782);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67100672) != 0 || _la == PrismParser::OSBRACE

    || _la == PrismParser::COMMENT) {
      setState(779);
      rtx_raygen_stat();
      setState(784);
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
  enterRule(_localctx, 166, PrismParser::RuleRtx_raygen_definition);
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
    setState(788);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(785);
        option_block(); 
      }
      setState(790);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    }
    setState(791);
    match(PrismParser::RAYTRACE_RAYGEN);
    setState(792);
    name_id();
    setState(794);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(793);
      inherit();
    }
    setState(796);
    match(PrismParser::OBRACE);
    setState(797);
    rtx_raygen_block();
    setState(798);
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
  enterRule(_localctx, 168, PrismParser::RuleView_declaration);

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
    setState(803);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(800);
        option_block(); 
      }
      setState(805);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx);
    }
    setState(806);
    type_id();
    setState(807);
    name_id();
    setState(808);
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
  enterRule(_localctx, 170, PrismParser::RuleView_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(812);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(810);
        view_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(811);
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
  enterRule(_localctx, 172, PrismParser::RuleView_block);
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
    setState(817);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1107296257) != 0) {
      setState(814);
      view_stat();
      setState(819);
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
  enterRule(_localctx, 174, PrismParser::RuleView_definition);
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
    setState(823);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(820);
        option_block(); 
      }
      setState(825);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    }
    setState(826);
    match(PrismParser::VIEW);
    setState(827);
    name_id();
    setState(829);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(828);
      inherit();
    }
    setState(831);
    match(PrismParser::OBRACE);
    setState(832);
    view_block();
    setState(833);
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
  enterRule(_localctx, 176, PrismParser::RulePass_definition);
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
    setState(838);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 76, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(835);
        option_block(); 
      }
      setState(840);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 76, _ctx);
    }
    setState(841);
    match(PrismParser::PASS);
    setState(842);
    name_id();
    setState(844);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::COLON) {
      setState(843);
      inherit();
    }
    setState(846);
    match(PrismParser::OBRACE);
    setState(847);
    view_block();
    setState(848);
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
  enterRule(_localctx, 178, PrismParser::RulePipeline_stat);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(860);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::OSBRACE:
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(853);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == PrismParser::OSBRACE) {
          setState(850);
          option_block();
          setState(855);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(856);
        name_id();
        setState(857);
        match(PrismParser::SCOL);
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(859);
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
  enterRule(_localctx, 180, PrismParser::RulePipeline_block);
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
    setState(865);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 1107296257) != 0) {
      setState(862);
      pipeline_stat();
      setState(867);
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
  enterRule(_localctx, 182, PrismParser::RulePipeline_definition);

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
    match(PrismParser::PIPELINE);
    setState(869);
    name_id();
    setState(870);
    match(PrismParser::OBRACE);
    setState(871);
    pipeline_block();
    setState(872);
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
  enterRule(_localctx, 184, PrismParser::RuleEnum_value_declaration);
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
    setState(874);
    name_id();
    setState(877);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == PrismParser::ASSIGN) {
      setState(875);
      match(PrismParser::ASSIGN);
      setState(876);
      value_id();
    }
    setState(879);
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
  enterRule(_localctx, 186, PrismParser::RuleEnum_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(883);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PrismParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(881);
        enum_value_declaration();
        break;
      }

      case PrismParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(882);
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
  enterRule(_localctx, 188, PrismParser::RuleEnum_block);
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
    setState(888);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PrismParser::ID

    || _la == PrismParser::COMMENT) {
      setState(885);
      enum_stat();
      setState(890);
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
  enterRule(_localctx, 190, PrismParser::RuleEnum_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(891);
    match(PrismParser::ENUM);
    setState(892);
    name_id();
    setState(893);
    match(PrismParser::OBRACE);
    setState(894);
    enum_block();
    setState(895);
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
  enterRule(_localctx, 192, PrismParser::RuleShader_type);
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
    setState(897);
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
  enterRule(_localctx, 194, PrismParser::RulePso_param_id);
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
    setState(899);
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
  enterRule(_localctx, 196, PrismParser::RuleBool_type);
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
    setState(901);
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
