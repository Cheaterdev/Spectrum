
// Generated from sources/SIGParser/SIG.g4 by ANTLR 4.11.1


#include "SIGListener.h"
#include "SIGVisitor.h"

#include "SIGParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SIGParserStaticData final {
  SIGParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SIGParserStaticData(const SIGParserStaticData&) = delete;
  SIGParserStaticData(SIGParserStaticData&&) = delete;
  SIGParserStaticData& operator=(const SIGParserStaticData&) = delete;
  SIGParserStaticData& operator=(SIGParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sigParserOnceFlag;
SIGParserStaticData *sigParserStaticData = nullptr;

void sigParserInitialize() {
  assert(sigParserStaticData == nullptr);
  auto staticData = std::make_unique<SIGParserStaticData>(
    std::vector<std::string>{
      "parse", "const_definition", "bind_option", "cond_expr", "cond_term", 
      "qualified_ref", "member_ref", "cond_op", "flag_value_holder", "raw_value", 
      "options_assign", "option", "option_block", "array_count_id", "array", 
      "value_declaration", "slot_declaration", "sampler_declaration", "define_declaration", 
      "rtv_formats_declaration", "blends_declaration", "pointer", "pso_param", 
      "class_no_template", "type_with_template", "inherit_id", "name_id", 
      "option_id", "owner_id", "template_id", "function_id", "value_id", 
      "value_id_ignore", "type_id", "insert_block", "path_id", "inherit", 
      "layout_stat", "layout_block", "layout_definition", "table_stat", 
      "table_block", "table_definition", "rt_color_declaration", "rt_ds_declaration", 
      "rt_stat", "rt_block", "rt_definition", "array_value_holder", "array_value_ids", 
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
      "':'", "'launch'", "'entry'", "'num_threads'", "'max_dispatch_grid'", 
      "'input'", "'compute'", "'vertex'", "'pixel'", "'domain'", "'hull'", 
      "'geometry'", "'miss'", "'closest_hit'", "'any_hit'", "'raygen'", 
      "'amplification'", "'mesh'", "'shader'", "'ds'", "'cull'", "'depth_func'", 
      "'depth_write'", "'conservative'", "'depth_bias'", "'depth_bias_clamp'", 
      "'slope_scaled_depth_bias'", "'enable_depth'", "'topology'", "'enable_stencil'", 
      "'stencil_func'", "'stencil_pass_op'", "'stencil_read_mask'", "'stencil_write_mask'", 
      "'recursion_depth'", "'payload'", "'per_material'", "'local'", "'||'", 
      "'&&'", "'|'", "'=='", "'!='", "'>'", "'<'", "'>='", "'<='", "'+'", 
      "'-'", "'/'", "'%'", "'^'", "'!'", "';'", "'.'", "'='", "'('", "')'", 
      "'{'", "'}'", "'['", "']'", "'true'", "'false'", "'log'", "'layout'", 
      "'struct'", "'ComputePSO'", "'GraphicsPSO'", "'RaytracePSO'", "'WorkgraphPSO'", 
      "'Node'", "'NodeOutput'", "'RaytraceRaygen'", "'RaytracePass'", "'PassNode'", 
      "'PassView'", "'Pipeline'", "'slot'", "'rt'", "'RTV'", "'DSV'", "'root'", 
      "'enum'", "", "", "", "", "", "", "", "'*'", "'%{'", "'}%'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "OR", "AND", "PIPE", 
      "EQ", "NEQ", "GT", "LT", "GTEQ", "LTEQ", "PLUS", "MINUS", "DIV", "MOD", 
      "POW", "NOT", "SCOL", "DOT", "ASSIGN", "OPAR", "CPAR", "OBRACE", "CBRACE", 
      "OSBRACE", "CSBRACE", "TRUE", "FALSE", "LOG", "LAYOUT", "STRUCT", 
      "COMPUTE_PSO", "GRAPHICS_PSO", "RAYTRACE_PSO", "WORKGRAPH_PSO", "NODE", 
      "NODE_OUTPUT", "RAYTRACE_RAYGEN", "RAYTRACE_PASS", "PASS", "VIEW", 
      "PIPELINE", "SLOT", "RT", "RTV", "DSV", "ROOTSIG", "ENUM", "ID", "INT_SCALAR", 
      "FLOAT_SCALAR", "STRING", "RAWEXPR", "COMMENT", "SPACE", "POINTER", 
      "INSERT_START", "INSERT_END", "INSERT_BLOCK"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,102,824,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
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
  	84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,2,89,7,89,2,90,7,90,1,0,1,
  	0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,5,0,198,8,0,10,
  	0,12,0,201,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,2,3,2,213,8,2,1,
  	2,1,2,1,2,4,2,218,8,2,11,2,12,2,219,1,2,1,2,3,2,224,8,2,1,3,4,3,227,8,
  	3,11,3,12,3,228,1,4,1,4,1,4,1,4,1,4,3,4,236,8,4,1,5,1,5,1,5,1,5,1,6,1,
  	6,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,10,1,11,1,11,3,11,257,8,
  	11,1,12,1,12,1,12,1,12,5,12,263,8,12,10,12,12,12,266,9,12,1,12,1,12,1,
  	13,1,13,1,14,1,14,3,14,274,8,14,1,14,1,14,1,15,5,15,279,8,15,10,15,12,
  	15,282,9,15,1,15,1,15,1,15,3,15,287,8,15,1,15,1,15,3,15,291,8,15,1,15,
  	1,15,1,16,1,16,1,16,1,16,1,17,1,17,1,17,1,17,1,17,1,17,1,18,5,18,306,
  	8,18,10,18,12,18,309,9,18,1,18,1,18,1,18,1,18,3,18,315,8,18,1,18,1,18,
  	1,19,5,19,320,8,19,10,19,12,19,323,9,19,1,19,1,19,1,19,1,19,1,19,1,20,
  	5,20,331,8,20,10,20,12,20,334,9,20,1,20,1,20,1,20,1,20,1,20,1,21,1,21,
  	1,22,5,22,344,8,22,10,22,12,22,347,9,22,1,22,1,22,1,22,1,22,1,22,1,23,
  	1,23,1,24,1,24,1,24,5,24,359,8,24,10,24,12,24,362,9,24,1,24,3,24,365,
  	8,24,1,24,3,24,368,8,24,1,25,1,25,1,26,1,26,1,27,1,27,1,28,1,28,1,29,
  	1,29,1,30,1,30,1,30,3,30,383,8,30,1,30,1,30,5,30,387,8,30,10,30,12,30,
  	390,9,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,1,31,1,31,3,31,401,8,31,1,
  	32,1,32,1,32,1,32,3,32,407,8,32,1,33,1,33,1,34,1,34,1,35,1,35,5,35,415,
  	8,35,10,35,12,35,418,9,35,1,35,1,35,1,36,1,36,1,36,1,36,5,36,426,8,36,
  	10,36,12,36,429,9,36,1,37,1,37,1,37,3,37,434,8,37,1,38,5,38,437,8,38,
  	10,38,12,38,440,9,38,1,39,1,39,1,39,3,39,445,8,39,1,39,1,39,1,39,1,39,
  	1,40,1,40,1,40,3,40,454,8,40,1,41,5,41,457,8,41,10,41,12,41,460,9,41,
  	1,42,5,42,463,8,42,10,42,12,42,466,9,42,1,42,1,42,1,42,3,42,471,8,42,
  	1,42,1,42,1,42,1,42,1,43,1,43,1,43,1,43,1,44,1,44,1,44,1,44,1,45,1,45,
  	1,45,3,45,488,8,45,1,46,5,46,491,8,46,10,46,12,46,494,9,46,1,47,1,47,
  	1,47,1,47,1,47,1,47,1,48,1,48,1,49,1,49,1,49,1,49,5,49,508,8,49,10,49,
  	12,49,511,9,49,1,49,1,49,1,50,1,50,1,50,1,50,1,50,1,51,5,51,521,8,51,
  	10,51,12,51,524,9,51,1,51,1,51,1,51,1,51,1,51,1,52,1,52,1,52,1,52,3,52,
  	535,8,52,1,53,5,53,538,8,53,10,53,12,53,541,9,53,1,54,5,54,544,8,54,10,
  	54,12,54,547,9,54,1,54,1,54,1,54,3,54,552,8,54,1,54,1,54,1,54,1,54,1,
  	55,1,55,1,55,1,55,1,55,1,55,1,55,3,55,565,8,55,1,56,5,56,568,8,56,10,
  	56,12,56,571,9,56,1,57,5,57,574,8,57,10,57,12,57,577,9,57,1,57,1,57,1,
  	57,3,57,582,8,57,1,57,1,57,1,57,1,57,1,58,1,58,3,58,590,8,58,1,59,5,59,
  	593,8,59,10,59,12,59,596,9,59,1,60,1,60,1,60,3,60,601,8,60,1,60,1,60,
  	1,60,1,60,1,61,1,61,1,62,1,62,1,62,1,62,1,62,1,63,5,63,615,8,63,10,63,
  	12,63,618,9,63,1,63,1,63,1,63,1,63,1,63,1,64,1,64,1,64,3,64,628,8,64,
  	1,65,5,65,631,8,65,10,65,12,65,634,9,65,1,66,1,66,1,66,1,66,1,66,1,66,
  	1,67,1,67,1,67,1,67,1,67,3,67,647,8,67,1,68,5,68,650,8,68,10,68,12,68,
  	653,9,68,1,69,5,69,656,8,69,10,69,12,69,659,9,69,1,69,1,69,1,69,3,69,
  	664,8,69,1,69,1,69,1,69,1,69,1,70,1,70,1,70,3,70,673,8,70,1,71,5,71,676,
  	8,71,10,71,12,71,679,9,71,1,72,5,72,682,8,72,10,72,12,72,685,9,72,1,72,
  	1,72,1,72,3,72,690,8,72,1,72,1,72,1,72,1,72,1,73,1,73,3,73,698,8,73,1,
  	74,5,74,701,8,74,10,74,12,74,704,9,74,1,75,5,75,707,8,75,10,75,12,75,
  	710,9,75,1,75,1,75,1,75,3,75,715,8,75,1,75,1,75,1,75,1,75,1,76,5,76,722,
  	8,76,10,76,12,76,725,9,76,1,76,1,76,1,76,1,76,1,77,1,77,3,77,733,8,77,
  	1,78,5,78,736,8,78,10,78,12,78,739,9,78,1,79,5,79,742,8,79,10,79,12,79,
  	745,9,79,1,79,1,79,1,79,3,79,750,8,79,1,79,1,79,1,79,1,79,1,80,5,80,757,
  	8,80,10,80,12,80,760,9,80,1,80,1,80,1,80,3,80,765,8,80,1,80,1,80,1,80,
  	1,80,1,81,5,81,772,8,81,10,81,12,81,775,9,81,1,81,1,81,1,81,1,81,3,81,
  	781,8,81,1,82,5,82,784,8,82,10,82,12,82,787,9,82,1,83,1,83,1,83,1,83,
  	1,83,1,83,1,84,1,84,1,84,3,84,798,8,84,1,84,1,84,1,85,1,85,3,85,804,8,
  	85,1,86,5,86,807,8,86,10,86,12,86,810,9,86,1,87,1,87,1,87,1,87,1,87,1,
  	87,1,88,1,88,1,89,1,89,1,90,1,90,1,90,18,280,307,321,332,345,416,427,
  	464,522,545,575,616,657,683,708,723,743,758,0,91,0,2,4,6,8,10,12,14,16,
  	18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
  	64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,
  	108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,
  	144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,
  	180,0,5,4,0,46,47,49,54,60,60,64,65,1,0,9,13,1,0,14,26,1,0,27,45,1,0,
  	70,71,848,0,199,1,0,0,0,2,204,1,0,0,0,4,223,1,0,0,0,6,226,1,0,0,0,8,235,
  	1,0,0,0,10,237,1,0,0,0,12,241,1,0,0,0,14,245,1,0,0,0,16,247,1,0,0,0,18,
  	249,1,0,0,0,20,251,1,0,0,0,22,254,1,0,0,0,24,258,1,0,0,0,26,269,1,0,0,
  	0,28,271,1,0,0,0,30,280,1,0,0,0,32,294,1,0,0,0,34,298,1,0,0,0,36,307,
  	1,0,0,0,38,321,1,0,0,0,40,332,1,0,0,0,42,340,1,0,0,0,44,345,1,0,0,0,46,
  	353,1,0,0,0,48,355,1,0,0,0,50,369,1,0,0,0,52,371,1,0,0,0,54,373,1,0,0,
  	0,56,375,1,0,0,0,58,377,1,0,0,0,60,379,1,0,0,0,62,400,1,0,0,0,64,406,
  	1,0,0,0,66,408,1,0,0,0,68,410,1,0,0,0,70,416,1,0,0,0,72,421,1,0,0,0,74,
  	433,1,0,0,0,76,438,1,0,0,0,78,441,1,0,0,0,80,453,1,0,0,0,82,458,1,0,0,
  	0,84,464,1,0,0,0,86,476,1,0,0,0,88,480,1,0,0,0,90,487,1,0,0,0,92,492,
  	1,0,0,0,94,495,1,0,0,0,96,501,1,0,0,0,98,503,1,0,0,0,100,514,1,0,0,0,
  	102,522,1,0,0,0,104,534,1,0,0,0,106,539,1,0,0,0,108,545,1,0,0,0,110,564,
  	1,0,0,0,112,569,1,0,0,0,114,575,1,0,0,0,116,589,1,0,0,0,118,594,1,0,0,
  	0,120,597,1,0,0,0,122,606,1,0,0,0,124,608,1,0,0,0,126,616,1,0,0,0,128,
  	627,1,0,0,0,130,632,1,0,0,0,132,635,1,0,0,0,134,646,1,0,0,0,136,651,1,
  	0,0,0,138,657,1,0,0,0,140,672,1,0,0,0,142,677,1,0,0,0,144,683,1,0,0,0,
  	146,697,1,0,0,0,148,702,1,0,0,0,150,708,1,0,0,0,152,723,1,0,0,0,154,732,
  	1,0,0,0,156,737,1,0,0,0,158,743,1,0,0,0,160,758,1,0,0,0,162,780,1,0,0,
  	0,164,785,1,0,0,0,166,788,1,0,0,0,168,794,1,0,0,0,170,803,1,0,0,0,172,
  	808,1,0,0,0,174,811,1,0,0,0,176,817,1,0,0,0,178,819,1,0,0,0,180,821,1,
  	0,0,0,182,198,3,78,39,0,183,198,3,84,42,0,184,198,3,94,47,0,185,198,3,
  	138,69,0,186,198,3,108,54,0,187,198,3,114,57,0,188,198,3,120,60,0,189,
  	198,3,144,72,0,190,198,3,150,75,0,191,198,3,160,80,0,192,198,3,158,79,
  	0,193,198,3,166,83,0,194,198,3,174,87,0,195,198,3,2,1,0,196,198,5,97,
  	0,0,197,182,1,0,0,0,197,183,1,0,0,0,197,184,1,0,0,0,197,185,1,0,0,0,197,
  	186,1,0,0,0,197,187,1,0,0,0,197,188,1,0,0,0,197,189,1,0,0,0,197,190,1,
  	0,0,0,197,191,1,0,0,0,197,192,1,0,0,0,197,193,1,0,0,0,197,194,1,0,0,0,
  	197,195,1,0,0,0,197,196,1,0,0,0,198,201,1,0,0,0,199,197,1,0,0,0,199,200,
  	1,0,0,0,200,202,1,0,0,0,201,199,1,0,0,0,202,203,5,0,0,1,203,1,1,0,0,0,
  	204,205,5,1,0,0,205,206,3,52,26,0,206,207,3,20,10,0,207,208,5,61,0,0,
  	208,3,1,0,0,0,209,210,3,56,28,0,210,211,5,2,0,0,211,213,1,0,0,0,212,209,
  	1,0,0,0,212,213,1,0,0,0,213,214,1,0,0,0,214,217,3,16,8,0,215,216,5,48,
  	0,0,216,218,3,16,8,0,217,215,1,0,0,0,218,219,1,0,0,0,219,217,1,0,0,0,
  	219,220,1,0,0,0,220,224,1,0,0,0,221,224,3,18,9,0,222,224,3,6,3,0,223,
  	212,1,0,0,0,223,221,1,0,0,0,223,222,1,0,0,0,224,5,1,0,0,0,225,227,3,8,
  	4,0,226,225,1,0,0,0,227,228,1,0,0,0,228,226,1,0,0,0,228,229,1,0,0,0,229,
  	7,1,0,0,0,230,236,3,10,5,0,231,236,3,60,30,0,232,236,3,12,6,0,233,236,
  	3,62,31,0,234,236,3,14,7,0,235,230,1,0,0,0,235,231,1,0,0,0,235,232,1,
  	0,0,0,235,233,1,0,0,0,235,234,1,0,0,0,236,9,1,0,0,0,237,238,3,56,28,0,
  	238,239,5,2,0,0,239,240,3,62,31,0,240,11,1,0,0,0,241,242,3,52,26,0,242,
  	243,5,62,0,0,243,244,3,52,26,0,244,13,1,0,0,0,245,246,7,0,0,0,246,15,
  	1,0,0,0,247,248,3,62,31,0,248,17,1,0,0,0,249,250,5,96,0,0,250,19,1,0,
  	0,0,251,252,5,63,0,0,252,253,3,4,2,0,253,21,1,0,0,0,254,256,3,52,26,0,
  	255,257,3,20,10,0,256,255,1,0,0,0,256,257,1,0,0,0,257,23,1,0,0,0,258,
  	259,5,68,0,0,259,264,3,22,11,0,260,261,5,3,0,0,261,263,3,22,11,0,262,
  	260,1,0,0,0,263,266,1,0,0,0,264,262,1,0,0,0,264,265,1,0,0,0,265,267,1,
  	0,0,0,266,264,1,0,0,0,267,268,5,69,0,0,268,25,1,0,0,0,269,270,5,93,0,
  	0,270,27,1,0,0,0,271,273,5,68,0,0,272,274,3,26,13,0,273,272,1,0,0,0,273,
  	274,1,0,0,0,274,275,1,0,0,0,275,276,5,69,0,0,276,29,1,0,0,0,277,279,3,
  	24,12,0,278,277,1,0,0,0,279,282,1,0,0,0,280,281,1,0,0,0,280,278,1,0,0,
  	0,281,283,1,0,0,0,282,280,1,0,0,0,283,284,3,66,33,0,284,286,3,52,26,0,
  	285,287,3,28,14,0,286,285,1,0,0,0,286,287,1,0,0,0,287,290,1,0,0,0,288,
  	289,5,63,0,0,289,291,3,62,31,0,290,288,1,0,0,0,290,291,1,0,0,0,291,292,
  	1,0,0,0,292,293,5,61,0,0,293,31,1,0,0,0,294,295,5,86,0,0,295,296,3,52,
  	26,0,296,297,5,61,0,0,297,33,1,0,0,0,298,299,5,4,0,0,299,300,3,52,26,
  	0,300,301,5,63,0,0,301,302,3,62,31,0,302,303,5,61,0,0,303,35,1,0,0,0,
  	304,306,3,24,12,0,305,304,1,0,0,0,306,309,1,0,0,0,307,308,1,0,0,0,307,
  	305,1,0,0,0,308,310,1,0,0,0,309,307,1,0,0,0,310,311,5,5,0,0,311,314,3,
  	52,26,0,312,313,5,63,0,0,313,315,3,98,49,0,314,312,1,0,0,0,314,315,1,
  	0,0,0,315,316,1,0,0,0,316,317,5,61,0,0,317,37,1,0,0,0,318,320,3,24,12,
  	0,319,318,1,0,0,0,320,323,1,0,0,0,321,322,1,0,0,0,321,319,1,0,0,0,322,
  	324,1,0,0,0,323,321,1,0,0,0,324,325,5,6,0,0,325,326,5,63,0,0,326,327,
  	3,98,49,0,327,328,5,61,0,0,328,39,1,0,0,0,329,331,3,24,12,0,330,329,1,
  	0,0,0,331,334,1,0,0,0,332,333,1,0,0,0,332,330,1,0,0,0,333,335,1,0,0,0,
  	334,332,1,0,0,0,335,336,5,7,0,0,336,337,5,63,0,0,337,338,3,98,49,0,338,
  	339,5,61,0,0,339,41,1,0,0,0,340,341,5,99,0,0,341,43,1,0,0,0,342,344,3,
  	24,12,0,343,342,1,0,0,0,344,347,1,0,0,0,345,346,1,0,0,0,345,343,1,0,0,
  	0,346,348,1,0,0,0,347,345,1,0,0,0,348,349,3,178,89,0,349,350,5,63,0,0,
  	350,351,3,62,31,0,351,352,5,61,0,0,352,45,1,0,0,0,353,354,5,92,0,0,354,
  	47,1,0,0,0,355,364,3,46,23,0,356,360,5,52,0,0,357,359,3,58,29,0,358,357,
  	1,0,0,0,359,362,1,0,0,0,360,358,1,0,0,0,360,361,1,0,0,0,361,363,1,0,0,
  	0,362,360,1,0,0,0,363,365,5,51,0,0,364,356,1,0,0,0,364,365,1,0,0,0,365,
  	367,1,0,0,0,366,368,3,42,21,0,367,366,1,0,0,0,367,368,1,0,0,0,368,49,
  	1,0,0,0,369,370,5,92,0,0,370,51,1,0,0,0,371,372,5,92,0,0,372,53,1,0,0,
  	0,373,374,5,92,0,0,374,55,1,0,0,0,375,376,5,92,0,0,376,57,1,0,0,0,377,
  	378,5,92,0,0,378,59,1,0,0,0,379,380,5,92,0,0,380,382,5,64,0,0,381,383,
  	3,64,32,0,382,381,1,0,0,0,382,383,1,0,0,0,383,388,1,0,0,0,384,385,5,3,
  	0,0,385,387,3,64,32,0,386,384,1,0,0,0,387,390,1,0,0,0,388,386,1,0,0,0,
  	388,389,1,0,0,0,389,391,1,0,0,0,390,388,1,0,0,0,391,392,5,65,0,0,392,
  	61,1,0,0,0,393,401,3,176,88,0,394,401,5,92,0,0,395,401,5,93,0,0,396,401,
  	5,94,0,0,397,401,3,180,90,0,398,401,3,60,30,0,399,401,3,98,49,0,400,393,
  	1,0,0,0,400,394,1,0,0,0,400,395,1,0,0,0,400,396,1,0,0,0,400,397,1,0,0,
  	0,400,398,1,0,0,0,400,399,1,0,0,0,401,63,1,0,0,0,402,407,5,92,0,0,403,
  	407,5,93,0,0,404,407,5,94,0,0,405,407,3,180,90,0,406,402,1,0,0,0,406,
  	403,1,0,0,0,406,404,1,0,0,0,406,405,1,0,0,0,407,65,1,0,0,0,408,409,3,
  	48,24,0,409,67,1,0,0,0,410,411,5,102,0,0,411,69,1,0,0,0,412,413,5,92,
  	0,0,413,415,5,57,0,0,414,412,1,0,0,0,415,418,1,0,0,0,416,417,1,0,0,0,
  	416,414,1,0,0,0,417,419,1,0,0,0,418,416,1,0,0,0,419,420,5,92,0,0,420,
  	71,1,0,0,0,421,422,5,8,0,0,422,427,3,50,25,0,423,424,5,3,0,0,424,426,
  	3,50,25,0,425,423,1,0,0,0,426,429,1,0,0,0,427,428,1,0,0,0,427,425,1,0,
  	0,0,428,73,1,0,0,0,429,427,1,0,0,0,430,434,3,32,16,0,431,434,3,34,17,
  	0,432,434,5,97,0,0,433,430,1,0,0,0,433,431,1,0,0,0,433,432,1,0,0,0,434,
  	75,1,0,0,0,435,437,3,74,37,0,436,435,1,0,0,0,437,440,1,0,0,0,438,436,
  	1,0,0,0,438,439,1,0,0,0,439,77,1,0,0,0,440,438,1,0,0,0,441,442,5,73,0,
  	0,442,444,3,52,26,0,443,445,3,72,36,0,444,443,1,0,0,0,444,445,1,0,0,0,
  	445,446,1,0,0,0,446,447,5,66,0,0,447,448,3,76,38,0,448,449,5,67,0,0,449,
  	79,1,0,0,0,450,454,3,30,15,0,451,454,3,68,34,0,452,454,5,97,0,0,453,450,
  	1,0,0,0,453,451,1,0,0,0,453,452,1,0,0,0,454,81,1,0,0,0,455,457,3,80,40,
  	0,456,455,1,0,0,0,457,460,1,0,0,0,458,456,1,0,0,0,458,459,1,0,0,0,459,
  	83,1,0,0,0,460,458,1,0,0,0,461,463,3,24,12,0,462,461,1,0,0,0,463,466,
  	1,0,0,0,464,465,1,0,0,0,464,462,1,0,0,0,465,467,1,0,0,0,466,464,1,0,0,
  	0,467,468,5,74,0,0,468,470,3,52,26,0,469,471,3,72,36,0,470,469,1,0,0,
  	0,470,471,1,0,0,0,471,472,1,0,0,0,472,473,5,66,0,0,473,474,3,82,41,0,
  	474,475,5,67,0,0,475,85,1,0,0,0,476,477,3,66,33,0,477,478,3,52,26,0,478,
  	479,5,61,0,0,479,87,1,0,0,0,480,481,5,89,0,0,481,482,3,52,26,0,482,483,
  	5,61,0,0,483,89,1,0,0,0,484,488,3,86,43,0,485,488,3,88,44,0,486,488,5,
  	97,0,0,487,484,1,0,0,0,487,485,1,0,0,0,487,486,1,0,0,0,488,91,1,0,0,0,
  	489,491,3,90,45,0,490,489,1,0,0,0,491,494,1,0,0,0,492,490,1,0,0,0,492,
  	493,1,0,0,0,493,93,1,0,0,0,494,492,1,0,0,0,495,496,5,87,0,0,496,497,3,
  	52,26,0,497,498,5,66,0,0,498,499,3,92,46,0,499,500,5,67,0,0,500,95,1,
  	0,0,0,501,502,3,62,31,0,502,97,1,0,0,0,503,504,5,66,0,0,504,509,3,96,
  	48,0,505,506,5,3,0,0,506,508,3,96,48,0,507,505,1,0,0,0,508,511,1,0,0,
  	0,509,507,1,0,0,0,509,510,1,0,0,0,510,512,1,0,0,0,511,509,1,0,0,0,512,
  	513,5,67,0,0,513,99,1,0,0,0,514,515,5,90,0,0,515,516,5,63,0,0,516,517,
  	3,52,26,0,517,518,5,61,0,0,518,101,1,0,0,0,519,521,3,24,12,0,520,519,
  	1,0,0,0,521,524,1,0,0,0,522,523,1,0,0,0,522,520,1,0,0,0,523,525,1,0,0,
  	0,524,522,1,0,0,0,525,526,3,176,88,0,526,527,5,63,0,0,527,528,3,70,35,
  	0,528,529,5,61,0,0,529,103,1,0,0,0,530,535,3,100,50,0,531,535,3,102,51,
  	0,532,535,3,36,18,0,533,535,5,97,0,0,534,530,1,0,0,0,534,531,1,0,0,0,
  	534,532,1,0,0,0,534,533,1,0,0,0,535,105,1,0,0,0,536,538,3,104,52,0,537,
  	536,1,0,0,0,538,541,1,0,0,0,539,537,1,0,0,0,539,540,1,0,0,0,540,107,1,
  	0,0,0,541,539,1,0,0,0,542,544,3,24,12,0,543,542,1,0,0,0,544,547,1,0,0,
  	0,545,546,1,0,0,0,545,543,1,0,0,0,546,548,1,0,0,0,547,545,1,0,0,0,548,
  	549,5,75,0,0,549,551,3,52,26,0,550,552,3,72,36,0,551,550,1,0,0,0,551,
  	552,1,0,0,0,552,553,1,0,0,0,553,554,5,66,0,0,554,555,3,106,53,0,555,556,
  	5,67,0,0,556,109,1,0,0,0,557,565,3,100,50,0,558,565,3,102,51,0,559,565,
  	3,36,18,0,560,565,3,38,19,0,561,565,3,40,20,0,562,565,3,44,22,0,563,565,
  	5,97,0,0,564,557,1,0,0,0,564,558,1,0,0,0,564,559,1,0,0,0,564,560,1,0,
  	0,0,564,561,1,0,0,0,564,562,1,0,0,0,564,563,1,0,0,0,565,111,1,0,0,0,566,
  	568,3,110,55,0,567,566,1,0,0,0,568,571,1,0,0,0,569,567,1,0,0,0,569,570,
  	1,0,0,0,570,113,1,0,0,0,571,569,1,0,0,0,572,574,3,24,12,0,573,572,1,0,
  	0,0,574,577,1,0,0,0,575,576,1,0,0,0,575,573,1,0,0,0,576,578,1,0,0,0,577,
  	575,1,0,0,0,578,579,5,76,0,0,579,581,3,52,26,0,580,582,3,72,36,0,581,
  	580,1,0,0,0,581,582,1,0,0,0,582,583,1,0,0,0,583,584,5,66,0,0,584,585,
  	3,112,56,0,585,586,5,67,0,0,586,115,1,0,0,0,587,590,3,100,50,0,588,590,
  	5,97,0,0,589,587,1,0,0,0,589,588,1,0,0,0,590,117,1,0,0,0,591,593,3,116,
  	58,0,592,591,1,0,0,0,593,596,1,0,0,0,594,592,1,0,0,0,594,595,1,0,0,0,
  	595,119,1,0,0,0,596,594,1,0,0,0,597,598,5,77,0,0,598,600,3,52,26,0,599,
  	601,3,72,36,0,600,599,1,0,0,0,600,601,1,0,0,0,601,602,1,0,0,0,602,603,
  	5,66,0,0,603,604,3,118,59,0,604,605,5,67,0,0,605,121,1,0,0,0,606,607,
  	7,1,0,0,607,123,1,0,0,0,608,609,3,122,61,0,609,610,5,63,0,0,610,611,3,
  	62,31,0,611,612,5,61,0,0,612,125,1,0,0,0,613,615,3,24,12,0,614,613,1,
  	0,0,0,615,618,1,0,0,0,616,617,1,0,0,0,616,614,1,0,0,0,617,619,1,0,0,0,
  	618,616,1,0,0,0,619,620,5,80,0,0,620,621,3,66,33,0,621,622,3,52,26,0,
  	622,623,5,61,0,0,623,127,1,0,0,0,624,628,3,124,62,0,625,628,3,126,63,
  	0,626,628,5,97,0,0,627,624,1,0,0,0,627,625,1,0,0,0,627,626,1,0,0,0,628,
  	129,1,0,0,0,629,631,3,128,64,0,630,629,1,0,0,0,631,634,1,0,0,0,632,630,
  	1,0,0,0,632,633,1,0,0,0,633,131,1,0,0,0,634,632,1,0,0,0,635,636,5,79,
  	0,0,636,637,3,52,26,0,637,638,5,66,0,0,638,639,3,130,65,0,639,640,5,67,
  	0,0,640,133,1,0,0,0,641,647,3,100,50,0,642,647,3,102,51,0,643,647,3,36,
  	18,0,644,647,3,132,66,0,645,647,5,97,0,0,646,641,1,0,0,0,646,642,1,0,
  	0,0,646,643,1,0,0,0,646,644,1,0,0,0,646,645,1,0,0,0,647,135,1,0,0,0,648,
  	650,3,134,67,0,649,648,1,0,0,0,650,653,1,0,0,0,651,649,1,0,0,0,651,652,
  	1,0,0,0,652,137,1,0,0,0,653,651,1,0,0,0,654,656,3,24,12,0,655,654,1,0,
  	0,0,656,659,1,0,0,0,657,658,1,0,0,0,657,655,1,0,0,0,658,660,1,0,0,0,659,
  	657,1,0,0,0,660,661,5,78,0,0,661,663,3,52,26,0,662,664,3,72,36,0,663,
  	662,1,0,0,0,663,664,1,0,0,0,664,665,1,0,0,0,665,666,5,66,0,0,666,667,
  	3,136,68,0,667,668,5,67,0,0,668,139,1,0,0,0,669,673,3,102,51,0,670,673,
  	5,97,0,0,671,673,3,44,22,0,672,669,1,0,0,0,672,670,1,0,0,0,672,671,1,
  	0,0,0,673,141,1,0,0,0,674,676,3,140,70,0,675,674,1,0,0,0,676,679,1,0,
  	0,0,677,675,1,0,0,0,677,678,1,0,0,0,678,143,1,0,0,0,679,677,1,0,0,0,680,
  	682,3,24,12,0,681,680,1,0,0,0,682,685,1,0,0,0,683,684,1,0,0,0,683,681,
  	1,0,0,0,684,686,1,0,0,0,685,683,1,0,0,0,686,687,5,82,0,0,687,689,3,52,
  	26,0,688,690,3,72,36,0,689,688,1,0,0,0,689,690,1,0,0,0,690,691,1,0,0,
  	0,691,692,5,66,0,0,692,693,3,142,71,0,693,694,5,67,0,0,694,145,1,0,0,
  	0,695,698,3,102,51,0,696,698,5,97,0,0,697,695,1,0,0,0,697,696,1,0,0,0,
  	698,147,1,0,0,0,699,701,3,146,73,0,700,699,1,0,0,0,701,704,1,0,0,0,702,
  	700,1,0,0,0,702,703,1,0,0,0,703,149,1,0,0,0,704,702,1,0,0,0,705,707,3,
  	24,12,0,706,705,1,0,0,0,707,710,1,0,0,0,708,709,1,0,0,0,708,706,1,0,0,
  	0,709,711,1,0,0,0,710,708,1,0,0,0,711,712,5,81,0,0,712,714,3,52,26,0,
  	713,715,3,72,36,0,714,713,1,0,0,0,714,715,1,0,0,0,715,716,1,0,0,0,716,
  	717,5,66,0,0,717,718,3,148,74,0,718,719,5,67,0,0,719,151,1,0,0,0,720,
  	722,3,24,12,0,721,720,1,0,0,0,722,725,1,0,0,0,723,724,1,0,0,0,723,721,
  	1,0,0,0,724,726,1,0,0,0,725,723,1,0,0,0,726,727,3,66,33,0,727,728,3,52,
  	26,0,728,729,5,61,0,0,729,153,1,0,0,0,730,733,3,152,76,0,731,733,5,97,
  	0,0,732,730,1,0,0,0,732,731,1,0,0,0,733,155,1,0,0,0,734,736,3,154,77,
  	0,735,734,1,0,0,0,736,739,1,0,0,0,737,735,1,0,0,0,737,738,1,0,0,0,738,
  	157,1,0,0,0,739,737,1,0,0,0,740,742,3,24,12,0,741,740,1,0,0,0,742,745,
  	1,0,0,0,743,744,1,0,0,0,743,741,1,0,0,0,744,746,1,0,0,0,745,743,1,0,0,
  	0,746,747,5,84,0,0,747,749,3,52,26,0,748,750,3,72,36,0,749,748,1,0,0,
  	0,749,750,1,0,0,0,750,751,1,0,0,0,751,752,5,66,0,0,752,753,3,156,78,0,
  	753,754,5,67,0,0,754,159,1,0,0,0,755,757,3,24,12,0,756,755,1,0,0,0,757,
  	760,1,0,0,0,758,759,1,0,0,0,758,756,1,0,0,0,759,761,1,0,0,0,760,758,1,
  	0,0,0,761,762,5,83,0,0,762,764,3,52,26,0,763,765,3,72,36,0,764,763,1,
  	0,0,0,764,765,1,0,0,0,765,766,1,0,0,0,766,767,5,66,0,0,767,768,3,156,
  	78,0,768,769,5,67,0,0,769,161,1,0,0,0,770,772,3,24,12,0,771,770,1,0,0,
  	0,772,775,1,0,0,0,773,771,1,0,0,0,773,774,1,0,0,0,774,776,1,0,0,0,775,
  	773,1,0,0,0,776,777,3,52,26,0,777,778,5,61,0,0,778,781,1,0,0,0,779,781,
  	5,97,0,0,780,773,1,0,0,0,780,779,1,0,0,0,781,163,1,0,0,0,782,784,3,162,
  	81,0,783,782,1,0,0,0,784,787,1,0,0,0,785,783,1,0,0,0,785,786,1,0,0,0,
  	786,165,1,0,0,0,787,785,1,0,0,0,788,789,5,85,0,0,789,790,3,52,26,0,790,
  	791,5,66,0,0,791,792,3,164,82,0,792,793,5,67,0,0,793,167,1,0,0,0,794,
  	797,3,52,26,0,795,796,5,63,0,0,796,798,3,62,31,0,797,795,1,0,0,0,797,
  	798,1,0,0,0,798,799,1,0,0,0,799,800,5,61,0,0,800,169,1,0,0,0,801,804,
  	3,168,84,0,802,804,5,97,0,0,803,801,1,0,0,0,803,802,1,0,0,0,804,171,1,
  	0,0,0,805,807,3,170,85,0,806,805,1,0,0,0,807,810,1,0,0,0,808,806,1,0,
  	0,0,808,809,1,0,0,0,809,173,1,0,0,0,810,808,1,0,0,0,811,812,5,91,0,0,
  	812,813,3,52,26,0,813,814,5,66,0,0,814,815,3,172,86,0,815,816,5,67,0,
  	0,816,175,1,0,0,0,817,818,7,2,0,0,818,177,1,0,0,0,819,820,7,3,0,0,820,
  	179,1,0,0,0,821,822,7,4,0,0,822,181,1,0,0,0,77,197,199,212,219,223,228,
  	235,256,264,273,280,286,290,307,314,321,332,345,360,364,367,382,388,400,
  	406,416,427,433,438,444,453,458,464,470,487,492,509,522,534,539,545,551,
  	564,569,575,581,589,594,600,616,627,632,646,651,657,663,672,677,683,689,
  	697,702,708,714,723,732,737,743,749,758,764,773,780,785,797,803,808
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sigParserStaticData = staticData.release();
}

}

SIGParser::SIGParser(TokenStream *input) : SIGParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SIGParser::SIGParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SIGParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sigParserStaticData->atn, sigParserStaticData->decisionToDFA, sigParserStaticData->sharedContextCache, options);
}

SIGParser::~SIGParser() {
  delete _interpreter;
}

const atn::ATN& SIGParser::getATN() const {
  return *sigParserStaticData->atn;
}

std::string SIGParser::getGrammarFileName() const {
  return "SIG.g4";
}

const std::vector<std::string>& SIGParser::getRuleNames() const {
  return sigParserStaticData->ruleNames;
}

const dfa::Vocabulary& SIGParser::getVocabulary() const {
  return sigParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SIGParser::getSerializedATN() const {
  return sigParserStaticData->serializedATN;
}


//----------------- ParseContext ------------------------------------------------------------------

SIGParser::ParseContext::ParseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::ParseContext::EOF() {
  return getToken(SIGParser::EOF, 0);
}

std::vector<SIGParser::Layout_definitionContext *> SIGParser::ParseContext::layout_definition() {
  return getRuleContexts<SIGParser::Layout_definitionContext>();
}

SIGParser::Layout_definitionContext* SIGParser::ParseContext::layout_definition(size_t i) {
  return getRuleContext<SIGParser::Layout_definitionContext>(i);
}

std::vector<SIGParser::Table_definitionContext *> SIGParser::ParseContext::table_definition() {
  return getRuleContexts<SIGParser::Table_definitionContext>();
}

SIGParser::Table_definitionContext* SIGParser::ParseContext::table_definition(size_t i) {
  return getRuleContext<SIGParser::Table_definitionContext>(i);
}

std::vector<SIGParser::Rt_definitionContext *> SIGParser::ParseContext::rt_definition() {
  return getRuleContexts<SIGParser::Rt_definitionContext>();
}

SIGParser::Rt_definitionContext* SIGParser::ParseContext::rt_definition(size_t i) {
  return getRuleContext<SIGParser::Rt_definitionContext>(i);
}

std::vector<SIGParser::Workgraph_pso_definitionContext *> SIGParser::ParseContext::workgraph_pso_definition() {
  return getRuleContexts<SIGParser::Workgraph_pso_definitionContext>();
}

SIGParser::Workgraph_pso_definitionContext* SIGParser::ParseContext::workgraph_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Workgraph_pso_definitionContext>(i);
}

std::vector<SIGParser::Compute_pso_definitionContext *> SIGParser::ParseContext::compute_pso_definition() {
  return getRuleContexts<SIGParser::Compute_pso_definitionContext>();
}

SIGParser::Compute_pso_definitionContext* SIGParser::ParseContext::compute_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Compute_pso_definitionContext>(i);
}

std::vector<SIGParser::Graphics_pso_definitionContext *> SIGParser::ParseContext::graphics_pso_definition() {
  return getRuleContexts<SIGParser::Graphics_pso_definitionContext>();
}

SIGParser::Graphics_pso_definitionContext* SIGParser::ParseContext::graphics_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Graphics_pso_definitionContext>(i);
}

std::vector<SIGParser::Rtx_pso_definitionContext *> SIGParser::ParseContext::rtx_pso_definition() {
  return getRuleContexts<SIGParser::Rtx_pso_definitionContext>();
}

SIGParser::Rtx_pso_definitionContext* SIGParser::ParseContext::rtx_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_pso_definitionContext>(i);
}

std::vector<SIGParser::Rtx_pass_definitionContext *> SIGParser::ParseContext::rtx_pass_definition() {
  return getRuleContexts<SIGParser::Rtx_pass_definitionContext>();
}

SIGParser::Rtx_pass_definitionContext* SIGParser::ParseContext::rtx_pass_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_pass_definitionContext>(i);
}

std::vector<SIGParser::Rtx_raygen_definitionContext *> SIGParser::ParseContext::rtx_raygen_definition() {
  return getRuleContexts<SIGParser::Rtx_raygen_definitionContext>();
}

SIGParser::Rtx_raygen_definitionContext* SIGParser::ParseContext::rtx_raygen_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_raygen_definitionContext>(i);
}

std::vector<SIGParser::Pass_definitionContext *> SIGParser::ParseContext::pass_definition() {
  return getRuleContexts<SIGParser::Pass_definitionContext>();
}

SIGParser::Pass_definitionContext* SIGParser::ParseContext::pass_definition(size_t i) {
  return getRuleContext<SIGParser::Pass_definitionContext>(i);
}

std::vector<SIGParser::View_definitionContext *> SIGParser::ParseContext::view_definition() {
  return getRuleContexts<SIGParser::View_definitionContext>();
}

SIGParser::View_definitionContext* SIGParser::ParseContext::view_definition(size_t i) {
  return getRuleContext<SIGParser::View_definitionContext>(i);
}

std::vector<SIGParser::Pipeline_definitionContext *> SIGParser::ParseContext::pipeline_definition() {
  return getRuleContexts<SIGParser::Pipeline_definitionContext>();
}

SIGParser::Pipeline_definitionContext* SIGParser::ParseContext::pipeline_definition(size_t i) {
  return getRuleContext<SIGParser::Pipeline_definitionContext>(i);
}

std::vector<SIGParser::Enum_definitionContext *> SIGParser::ParseContext::enum_definition() {
  return getRuleContexts<SIGParser::Enum_definitionContext>();
}

SIGParser::Enum_definitionContext* SIGParser::ParseContext::enum_definition(size_t i) {
  return getRuleContext<SIGParser::Enum_definitionContext>(i);
}

std::vector<SIGParser::Const_definitionContext *> SIGParser::ParseContext::const_definition() {
  return getRuleContexts<SIGParser::Const_definitionContext>();
}

SIGParser::Const_definitionContext* SIGParser::ParseContext::const_definition(size_t i) {
  return getRuleContext<SIGParser::Const_definitionContext>(i);
}

std::vector<tree::TerminalNode *> SIGParser::ParseContext::COMMENT() {
  return getTokens(SIGParser::COMMENT);
}

tree::TerminalNode* SIGParser::ParseContext::COMMENT(size_t i) {
  return getToken(SIGParser::COMMENT, i);
}


size_t SIGParser::ParseContext::getRuleIndex() const {
  return SIGParser::RuleParse;
}

void SIGParser::ParseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParse(this);
}

void SIGParser::ParseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParse(this);
}


std::any SIGParser::ParseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitParse(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::ParseContext* SIGParser::parse() {
  ParseContext *_localctx = _tracker.createInstance<ParseContext>(_ctx, getState());
  enterRule(_localctx, 0, SIGParser::RuleParse);
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
    setState(199);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 546039777) != 0) {
      setState(197);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(182);
        layout_definition();
        break;
      }

      case 2: {
        setState(183);
        table_definition();
        break;
      }

      case 3: {
        setState(184);
        rt_definition();
        break;
      }

      case 4: {
        setState(185);
        workgraph_pso_definition();
        break;
      }

      case 5: {
        setState(186);
        compute_pso_definition();
        break;
      }

      case 6: {
        setState(187);
        graphics_pso_definition();
        break;
      }

      case 7: {
        setState(188);
        rtx_pso_definition();
        break;
      }

      case 8: {
        setState(189);
        rtx_pass_definition();
        break;
      }

      case 9: {
        setState(190);
        rtx_raygen_definition();
        break;
      }

      case 10: {
        setState(191);
        pass_definition();
        break;
      }

      case 11: {
        setState(192);
        view_definition();
        break;
      }

      case 12: {
        setState(193);
        pipeline_definition();
        break;
      }

      case 13: {
        setState(194);
        enum_definition();
        break;
      }

      case 14: {
        setState(195);
        const_definition();
        break;
      }

      case 15: {
        setState(196);
        match(SIGParser::COMMENT);
        break;
      }

      default:
        break;
      }
      setState(201);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(202);
    match(SIGParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Const_definitionContext ------------------------------------------------------------------

SIGParser::Const_definitionContext::Const_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Const_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

SIGParser::Options_assignContext* SIGParser::Const_definitionContext::options_assign() {
  return getRuleContext<SIGParser::Options_assignContext>(0);
}

tree::TerminalNode* SIGParser::Const_definitionContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Const_definitionContext::getRuleIndex() const {
  return SIGParser::RuleConst_definition;
}

void SIGParser::Const_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConst_definition(this);
}

void SIGParser::Const_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConst_definition(this);
}


std::any SIGParser::Const_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitConst_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Const_definitionContext* SIGParser::const_definition() {
  Const_definitionContext *_localctx = _tracker.createInstance<Const_definitionContext>(_ctx, getState());
  enterRule(_localctx, 2, SIGParser::RuleConst_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(204);
    match(SIGParser::T__0);
    setState(205);
    name_id();
    setState(206);
    options_assign();
    setState(207);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bind_optionContext ------------------------------------------------------------------

SIGParser::Bind_optionContext::Bind_optionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Flag_value_holderContext *> SIGParser::Bind_optionContext::flag_value_holder() {
  return getRuleContexts<SIGParser::Flag_value_holderContext>();
}

SIGParser::Flag_value_holderContext* SIGParser::Bind_optionContext::flag_value_holder(size_t i) {
  return getRuleContext<SIGParser::Flag_value_holderContext>(i);
}

SIGParser::Owner_idContext* SIGParser::Bind_optionContext::owner_id() {
  return getRuleContext<SIGParser::Owner_idContext>(0);
}

std::vector<tree::TerminalNode *> SIGParser::Bind_optionContext::PIPE() {
  return getTokens(SIGParser::PIPE);
}

tree::TerminalNode* SIGParser::Bind_optionContext::PIPE(size_t i) {
  return getToken(SIGParser::PIPE, i);
}

SIGParser::Raw_valueContext* SIGParser::Bind_optionContext::raw_value() {
  return getRuleContext<SIGParser::Raw_valueContext>(0);
}

SIGParser::Cond_exprContext* SIGParser::Bind_optionContext::cond_expr() {
  return getRuleContext<SIGParser::Cond_exprContext>(0);
}


size_t SIGParser::Bind_optionContext::getRuleIndex() const {
  return SIGParser::RuleBind_option;
}

void SIGParser::Bind_optionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBind_option(this);
}

void SIGParser::Bind_optionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBind_option(this);
}


std::any SIGParser::Bind_optionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitBind_option(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Bind_optionContext* SIGParser::bind_option() {
  Bind_optionContext *_localctx = _tracker.createInstance<Bind_optionContext>(_ctx, getState());
  enterRule(_localctx, 4, SIGParser::RuleBind_option);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(223);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(212);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
      case 1: {
        setState(209);
        owner_id();
        setState(210);
        match(SIGParser::T__1);
        break;
      }

      default:
        break;
      }
      setState(214);
      flag_value_holder();
      setState(217); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(215);
        match(SIGParser::PIPE);
        setState(216);
        flag_value_holder();
        setState(219); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == SIGParser::PIPE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(221);
      raw_value();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(222);
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

SIGParser::Cond_exprContext::Cond_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Cond_termContext *> SIGParser::Cond_exprContext::cond_term() {
  return getRuleContexts<SIGParser::Cond_termContext>();
}

SIGParser::Cond_termContext* SIGParser::Cond_exprContext::cond_term(size_t i) {
  return getRuleContext<SIGParser::Cond_termContext>(i);
}


size_t SIGParser::Cond_exprContext::getRuleIndex() const {
  return SIGParser::RuleCond_expr;
}

void SIGParser::Cond_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_expr(this);
}

void SIGParser::Cond_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_expr(this);
}


std::any SIGParser::Cond_exprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCond_expr(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Cond_exprContext* SIGParser::cond_expr() {
  Cond_exprContext *_localctx = _tracker.createInstance<Cond_exprContext>(_ctx, getState());
  enterRule(_localctx, 6, SIGParser::RuleCond_expr);
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
    setState(226); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(225);
      cond_term();
      setState(228); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1188598458039123968) != 0 || (((_la - 64) & ~ 0x3fULL) == 0) &&
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

SIGParser::Cond_termContext::Cond_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Qualified_refContext* SIGParser::Cond_termContext::qualified_ref() {
  return getRuleContext<SIGParser::Qualified_refContext>(0);
}

SIGParser::Function_idContext* SIGParser::Cond_termContext::function_id() {
  return getRuleContext<SIGParser::Function_idContext>(0);
}

SIGParser::Member_refContext* SIGParser::Cond_termContext::member_ref() {
  return getRuleContext<SIGParser::Member_refContext>(0);
}

SIGParser::Value_idContext* SIGParser::Cond_termContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

SIGParser::Cond_opContext* SIGParser::Cond_termContext::cond_op() {
  return getRuleContext<SIGParser::Cond_opContext>(0);
}


size_t SIGParser::Cond_termContext::getRuleIndex() const {
  return SIGParser::RuleCond_term;
}

void SIGParser::Cond_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_term(this);
}

void SIGParser::Cond_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_term(this);
}


std::any SIGParser::Cond_termContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCond_term(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Cond_termContext* SIGParser::cond_term() {
  Cond_termContext *_localctx = _tracker.createInstance<Cond_termContext>(_ctx, getState());
  enterRule(_localctx, 8, SIGParser::RuleCond_term);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(235);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(230);
      qualified_ref();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(231);
      function_id();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(232);
      member_ref();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(233);
      value_id();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(234);
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

SIGParser::Qualified_refContext::Qualified_refContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Owner_idContext* SIGParser::Qualified_refContext::owner_id() {
  return getRuleContext<SIGParser::Owner_idContext>(0);
}

SIGParser::Value_idContext* SIGParser::Qualified_refContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Qualified_refContext::getRuleIndex() const {
  return SIGParser::RuleQualified_ref;
}

void SIGParser::Qualified_refContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQualified_ref(this);
}

void SIGParser::Qualified_refContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQualified_ref(this);
}


std::any SIGParser::Qualified_refContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitQualified_ref(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Qualified_refContext* SIGParser::qualified_ref() {
  Qualified_refContext *_localctx = _tracker.createInstance<Qualified_refContext>(_ctx, getState());
  enterRule(_localctx, 10, SIGParser::RuleQualified_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(237);
    owner_id();
    setState(238);
    match(SIGParser::T__1);
    setState(239);
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

SIGParser::Member_refContext::Member_refContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Name_idContext *> SIGParser::Member_refContext::name_id() {
  return getRuleContexts<SIGParser::Name_idContext>();
}

SIGParser::Name_idContext* SIGParser::Member_refContext::name_id(size_t i) {
  return getRuleContext<SIGParser::Name_idContext>(i);
}

tree::TerminalNode* SIGParser::Member_refContext::DOT() {
  return getToken(SIGParser::DOT, 0);
}


size_t SIGParser::Member_refContext::getRuleIndex() const {
  return SIGParser::RuleMember_ref;
}

void SIGParser::Member_refContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMember_ref(this);
}

void SIGParser::Member_refContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMember_ref(this);
}


std::any SIGParser::Member_refContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitMember_ref(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Member_refContext* SIGParser::member_ref() {
  Member_refContext *_localctx = _tracker.createInstance<Member_refContext>(_ctx, getState());
  enterRule(_localctx, 12, SIGParser::RuleMember_ref);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    name_id();
    setState(242);
    match(SIGParser::DOT);
    setState(243);
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

SIGParser::Cond_opContext::Cond_opContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Cond_opContext::AND() {
  return getToken(SIGParser::AND, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::OR() {
  return getToken(SIGParser::OR, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::NOT() {
  return getToken(SIGParser::NOT, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::EQ() {
  return getToken(SIGParser::EQ, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::NEQ() {
  return getToken(SIGParser::NEQ, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::GTEQ() {
  return getToken(SIGParser::GTEQ, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::LTEQ() {
  return getToken(SIGParser::LTEQ, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::GT() {
  return getToken(SIGParser::GT, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::LT() {
  return getToken(SIGParser::LT, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::OPAR() {
  return getToken(SIGParser::OPAR, 0);
}

tree::TerminalNode* SIGParser::Cond_opContext::CPAR() {
  return getToken(SIGParser::CPAR, 0);
}


size_t SIGParser::Cond_opContext::getRuleIndex() const {
  return SIGParser::RuleCond_op;
}

void SIGParser::Cond_opContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond_op(this);
}

void SIGParser::Cond_opContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond_op(this);
}


std::any SIGParser::Cond_opContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCond_op(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Cond_opContext* SIGParser::cond_op() {
  Cond_opContext *_localctx = _tracker.createInstance<Cond_opContext>(_ctx, getState());
  enterRule(_localctx, 14, SIGParser::RuleCond_op);
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
    setState(245);
    _la = _input->LA(1);
    if (!((((_la - 46) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 46)) & 803323) != 0)) {
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

SIGParser::Flag_value_holderContext::Flag_value_holderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_idContext* SIGParser::Flag_value_holderContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Flag_value_holderContext::getRuleIndex() const {
  return SIGParser::RuleFlag_value_holder;
}

void SIGParser::Flag_value_holderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFlag_value_holder(this);
}

void SIGParser::Flag_value_holderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFlag_value_holder(this);
}


std::any SIGParser::Flag_value_holderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitFlag_value_holder(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Flag_value_holderContext* SIGParser::flag_value_holder() {
  Flag_value_holderContext *_localctx = _tracker.createInstance<Flag_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 16, SIGParser::RuleFlag_value_holder);

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

SIGParser::Raw_valueContext::Raw_valueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Raw_valueContext::RAWEXPR() {
  return getToken(SIGParser::RAWEXPR, 0);
}


size_t SIGParser::Raw_valueContext::getRuleIndex() const {
  return SIGParser::RuleRaw_value;
}

void SIGParser::Raw_valueContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRaw_value(this);
}

void SIGParser::Raw_valueContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRaw_value(this);
}


std::any SIGParser::Raw_valueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRaw_value(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Raw_valueContext* SIGParser::raw_value() {
  Raw_valueContext *_localctx = _tracker.createInstance<Raw_valueContext>(_ctx, getState());
  enterRule(_localctx, 18, SIGParser::RuleRaw_value);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(249);
    match(SIGParser::RAWEXPR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Options_assignContext ------------------------------------------------------------------

SIGParser::Options_assignContext::Options_assignContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Options_assignContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Bind_optionContext* SIGParser::Options_assignContext::bind_option() {
  return getRuleContext<SIGParser::Bind_optionContext>(0);
}


size_t SIGParser::Options_assignContext::getRuleIndex() const {
  return SIGParser::RuleOptions_assign;
}

void SIGParser::Options_assignContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOptions_assign(this);
}

void SIGParser::Options_assignContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOptions_assign(this);
}


std::any SIGParser::Options_assignContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitOptions_assign(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Options_assignContext* SIGParser::options_assign() {
  Options_assignContext *_localctx = _tracker.createInstance<Options_assignContext>(_ctx, getState());
  enterRule(_localctx, 20, SIGParser::RuleOptions_assign);

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
    match(SIGParser::ASSIGN);
    setState(252);
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

SIGParser::OptionContext::OptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::OptionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

SIGParser::Options_assignContext* SIGParser::OptionContext::options_assign() {
  return getRuleContext<SIGParser::Options_assignContext>(0);
}


size_t SIGParser::OptionContext::getRuleIndex() const {
  return SIGParser::RuleOption;
}

void SIGParser::OptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption(this);
}

void SIGParser::OptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption(this);
}


std::any SIGParser::OptionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitOption(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::OptionContext* SIGParser::option() {
  OptionContext *_localctx = _tracker.createInstance<OptionContext>(_ctx, getState());
  enterRule(_localctx, 22, SIGParser::RuleOption);
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
    setState(254);
    name_id();
    setState(256);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(255);
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

SIGParser::Option_blockContext::Option_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Option_blockContext::OSBRACE() {
  return getToken(SIGParser::OSBRACE, 0);
}

std::vector<SIGParser::OptionContext *> SIGParser::Option_blockContext::option() {
  return getRuleContexts<SIGParser::OptionContext>();
}

SIGParser::OptionContext* SIGParser::Option_blockContext::option(size_t i) {
  return getRuleContext<SIGParser::OptionContext>(i);
}

tree::TerminalNode* SIGParser::Option_blockContext::CSBRACE() {
  return getToken(SIGParser::CSBRACE, 0);
}


size_t SIGParser::Option_blockContext::getRuleIndex() const {
  return SIGParser::RuleOption_block;
}

void SIGParser::Option_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_block(this);
}

void SIGParser::Option_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_block(this);
}


std::any SIGParser::Option_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitOption_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Option_blockContext* SIGParser::option_block() {
  Option_blockContext *_localctx = _tracker.createInstance<Option_blockContext>(_ctx, getState());
  enterRule(_localctx, 24, SIGParser::RuleOption_block);
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
    setState(258);
    match(SIGParser::OSBRACE);
    setState(259);
    option();
    setState(264);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__2) {
      setState(260);
      match(SIGParser::T__2);
      setState(261);
      option();
      setState(266);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(267);
    match(SIGParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_count_idContext ------------------------------------------------------------------

SIGParser::Array_count_idContext::Array_count_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Array_count_idContext::INT_SCALAR() {
  return getToken(SIGParser::INT_SCALAR, 0);
}


size_t SIGParser::Array_count_idContext::getRuleIndex() const {
  return SIGParser::RuleArray_count_id;
}

void SIGParser::Array_count_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_count_id(this);
}

void SIGParser::Array_count_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_count_id(this);
}


std::any SIGParser::Array_count_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitArray_count_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Array_count_idContext* SIGParser::array_count_id() {
  Array_count_idContext *_localctx = _tracker.createInstance<Array_count_idContext>(_ctx, getState());
  enterRule(_localctx, 26, SIGParser::RuleArray_count_id);

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
    match(SIGParser::INT_SCALAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayContext ------------------------------------------------------------------

SIGParser::ArrayContext::ArrayContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::ArrayContext::OSBRACE() {
  return getToken(SIGParser::OSBRACE, 0);
}

tree::TerminalNode* SIGParser::ArrayContext::CSBRACE() {
  return getToken(SIGParser::CSBRACE, 0);
}

SIGParser::Array_count_idContext* SIGParser::ArrayContext::array_count_id() {
  return getRuleContext<SIGParser::Array_count_idContext>(0);
}


size_t SIGParser::ArrayContext::getRuleIndex() const {
  return SIGParser::RuleArray;
}

void SIGParser::ArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray(this);
}

void SIGParser::ArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray(this);
}


std::any SIGParser::ArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitArray(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::ArrayContext* SIGParser::array() {
  ArrayContext *_localctx = _tracker.createInstance<ArrayContext>(_ctx, getState());
  enterRule(_localctx, 28, SIGParser::RuleArray);
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
    setState(271);
    match(SIGParser::OSBRACE);
    setState(273);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::INT_SCALAR) {
      setState(272);
      array_count_id();
    }
    setState(275);
    match(SIGParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_declarationContext ------------------------------------------------------------------

SIGParser::Value_declarationContext::Value_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_idContext* SIGParser::Value_declarationContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::Value_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Value_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Value_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Value_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::ArrayContext* SIGParser::Value_declarationContext::array() {
  return getRuleContext<SIGParser::ArrayContext>(0);
}

tree::TerminalNode* SIGParser::Value_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Value_declarationContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Value_declarationContext::getRuleIndex() const {
  return SIGParser::RuleValue_declaration;
}

void SIGParser::Value_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_declaration(this);
}

void SIGParser::Value_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_declaration(this);
}


std::any SIGParser::Value_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitValue_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Value_declarationContext* SIGParser::value_declaration() {
  Value_declarationContext *_localctx = _tracker.createInstance<Value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 30, SIGParser::RuleValue_declaration);
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
    setState(280);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(277);
        option_block(); 
      }
      setState(282);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    }
    setState(283);
    type_id();
    setState(284);
    name_id();
    setState(286);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::OSBRACE) {
      setState(285);
      array();
    }
    setState(290);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(288);
      match(SIGParser::ASSIGN);
      setState(289);
      value_id();
    }
    setState(292);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Slot_declarationContext ------------------------------------------------------------------

SIGParser::Slot_declarationContext::Slot_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Slot_declarationContext::SLOT() {
  return getToken(SIGParser::SLOT, 0);
}

SIGParser::Name_idContext* SIGParser::Slot_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Slot_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Slot_declarationContext::getRuleIndex() const {
  return SIGParser::RuleSlot_declaration;
}

void SIGParser::Slot_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSlot_declaration(this);
}

void SIGParser::Slot_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSlot_declaration(this);
}


std::any SIGParser::Slot_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitSlot_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Slot_declarationContext* SIGParser::slot_declaration() {
  Slot_declarationContext *_localctx = _tracker.createInstance<Slot_declarationContext>(_ctx, getState());
  enterRule(_localctx, 32, SIGParser::RuleSlot_declaration);

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
    match(SIGParser::SLOT);
    setState(295);
    name_id();
    setState(296);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Sampler_declarationContext ------------------------------------------------------------------

SIGParser::Sampler_declarationContext::Sampler_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Sampler_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Sampler_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Sampler_declarationContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

tree::TerminalNode* SIGParser::Sampler_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Sampler_declarationContext::getRuleIndex() const {
  return SIGParser::RuleSampler_declaration;
}

void SIGParser::Sampler_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSampler_declaration(this);
}

void SIGParser::Sampler_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSampler_declaration(this);
}


std::any SIGParser::Sampler_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitSampler_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Sampler_declarationContext* SIGParser::sampler_declaration() {
  Sampler_declarationContext *_localctx = _tracker.createInstance<Sampler_declarationContext>(_ctx, getState());
  enterRule(_localctx, 34, SIGParser::RuleSampler_declaration);

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
    match(SIGParser::T__3);
    setState(299);
    name_id();
    setState(300);
    match(SIGParser::ASSIGN);
    setState(301);
    value_id();
    setState(302);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Define_declarationContext ------------------------------------------------------------------

SIGParser::Define_declarationContext::Define_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Define_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Define_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Define_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Define_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

tree::TerminalNode* SIGParser::Define_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Define_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}


size_t SIGParser::Define_declarationContext::getRuleIndex() const {
  return SIGParser::RuleDefine_declaration;
}

void SIGParser::Define_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefine_declaration(this);
}

void SIGParser::Define_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefine_declaration(this);
}


std::any SIGParser::Define_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitDefine_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Define_declarationContext* SIGParser::define_declaration() {
  Define_declarationContext *_localctx = _tracker.createInstance<Define_declarationContext>(_ctx, getState());
  enterRule(_localctx, 36, SIGParser::RuleDefine_declaration);
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
    setState(307);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(304);
        option_block(); 
      }
      setState(309);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    }
    setState(310);
    match(SIGParser::T__4);
    setState(311);
    name_id();
    setState(314);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(312);
      match(SIGParser::ASSIGN);
      setState(313);
      array_value_ids();
    }
    setState(316);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtv_formats_declarationContext ------------------------------------------------------------------

SIGParser::Rtv_formats_declarationContext::Rtv_formats_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtv_formats_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Rtv_formats_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}

tree::TerminalNode* SIGParser::Rtv_formats_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtv_formats_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtv_formats_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Rtv_formats_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRtv_formats_declaration;
}

void SIGParser::Rtv_formats_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtv_formats_declaration(this);
}

void SIGParser::Rtv_formats_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtv_formats_declaration(this);
}


std::any SIGParser::Rtv_formats_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtv_formats_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtv_formats_declarationContext* SIGParser::rtv_formats_declaration() {
  Rtv_formats_declarationContext *_localctx = _tracker.createInstance<Rtv_formats_declarationContext>(_ctx, getState());
  enterRule(_localctx, 38, SIGParser::RuleRtv_formats_declaration);

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
    setState(321);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(318);
        option_block(); 
      }
      setState(323);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    }
    setState(324);
    match(SIGParser::T__5);
    setState(325);
    match(SIGParser::ASSIGN);
    setState(326);
    array_value_ids();
    setState(327);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Blends_declarationContext ------------------------------------------------------------------

SIGParser::Blends_declarationContext::Blends_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Blends_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Blends_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}

tree::TerminalNode* SIGParser::Blends_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Blends_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Blends_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Blends_declarationContext::getRuleIndex() const {
  return SIGParser::RuleBlends_declaration;
}

void SIGParser::Blends_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlends_declaration(this);
}

void SIGParser::Blends_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlends_declaration(this);
}


std::any SIGParser::Blends_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitBlends_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Blends_declarationContext* SIGParser::blends_declaration() {
  Blends_declarationContext *_localctx = _tracker.createInstance<Blends_declarationContext>(_ctx, getState());
  enterRule(_localctx, 40, SIGParser::RuleBlends_declaration);

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
    setState(332);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(329);
        option_block(); 
      }
      setState(334);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx);
    }
    setState(335);
    match(SIGParser::T__6);
    setState(336);
    match(SIGParser::ASSIGN);
    setState(337);
    array_value_ids();
    setState(338);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PointerContext ------------------------------------------------------------------

SIGParser::PointerContext::PointerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::PointerContext::POINTER() {
  return getToken(SIGParser::POINTER, 0);
}


size_t SIGParser::PointerContext::getRuleIndex() const {
  return SIGParser::RulePointer;
}

void SIGParser::PointerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPointer(this);
}

void SIGParser::PointerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPointer(this);
}


std::any SIGParser::PointerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPointer(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::PointerContext* SIGParser::pointer() {
  PointerContext *_localctx = _tracker.createInstance<PointerContext>(_ctx, getState());
  enterRule(_localctx, 42, SIGParser::RulePointer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(340);
    match(SIGParser::POINTER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pso_paramContext ------------------------------------------------------------------

SIGParser::Pso_paramContext::Pso_paramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Pso_param_idContext* SIGParser::Pso_paramContext::pso_param_id() {
  return getRuleContext<SIGParser::Pso_param_idContext>(0);
}

tree::TerminalNode* SIGParser::Pso_paramContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Pso_paramContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

tree::TerminalNode* SIGParser::Pso_paramContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Pso_paramContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Pso_paramContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Pso_paramContext::getRuleIndex() const {
  return SIGParser::RulePso_param;
}

void SIGParser::Pso_paramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param(this);
}

void SIGParser::Pso_paramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param(this);
}


std::any SIGParser::Pso_paramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPso_param(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pso_paramContext* SIGParser::pso_param() {
  Pso_paramContext *_localctx = _tracker.createInstance<Pso_paramContext>(_ctx, getState());
  enterRule(_localctx, 44, SIGParser::RulePso_param);

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
    setState(345);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(342);
        option_block(); 
      }
      setState(347);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
    }
    setState(348);
    pso_param_id();
    setState(349);
    match(SIGParser::ASSIGN);
    setState(350);
    value_id();
    setState(351);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Class_no_templateContext ------------------------------------------------------------------

SIGParser::Class_no_templateContext::Class_no_templateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Class_no_templateContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Class_no_templateContext::getRuleIndex() const {
  return SIGParser::RuleClass_no_template;
}

void SIGParser::Class_no_templateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClass_no_template(this);
}

void SIGParser::Class_no_templateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClass_no_template(this);
}


std::any SIGParser::Class_no_templateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitClass_no_template(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Class_no_templateContext* SIGParser::class_no_template() {
  Class_no_templateContext *_localctx = _tracker.createInstance<Class_no_templateContext>(_ctx, getState());
  enterRule(_localctx, 46, SIGParser::RuleClass_no_template);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(353);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_with_templateContext ------------------------------------------------------------------

SIGParser::Type_with_templateContext::Type_with_templateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Class_no_templateContext* SIGParser::Type_with_templateContext::class_no_template() {
  return getRuleContext<SIGParser::Class_no_templateContext>(0);
}

tree::TerminalNode* SIGParser::Type_with_templateContext::LT() {
  return getToken(SIGParser::LT, 0);
}

tree::TerminalNode* SIGParser::Type_with_templateContext::GT() {
  return getToken(SIGParser::GT, 0);
}

SIGParser::PointerContext* SIGParser::Type_with_templateContext::pointer() {
  return getRuleContext<SIGParser::PointerContext>(0);
}

std::vector<SIGParser::Template_idContext *> SIGParser::Type_with_templateContext::template_id() {
  return getRuleContexts<SIGParser::Template_idContext>();
}

SIGParser::Template_idContext* SIGParser::Type_with_templateContext::template_id(size_t i) {
  return getRuleContext<SIGParser::Template_idContext>(i);
}


size_t SIGParser::Type_with_templateContext::getRuleIndex() const {
  return SIGParser::RuleType_with_template;
}

void SIGParser::Type_with_templateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_with_template(this);
}

void SIGParser::Type_with_templateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_with_template(this);
}


std::any SIGParser::Type_with_templateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitType_with_template(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Type_with_templateContext* SIGParser::type_with_template() {
  Type_with_templateContext *_localctx = _tracker.createInstance<Type_with_templateContext>(_ctx, getState());
  enterRule(_localctx, 48, SIGParser::RuleType_with_template);
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
    setState(355);
    class_no_template();
    setState(364);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::LT) {
      setState(356);
      match(SIGParser::LT);
      setState(360);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SIGParser::ID) {
        setState(357);
        template_id();
        setState(362);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(363);
      match(SIGParser::GT);
    }
    setState(367);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::POINTER) {
      setState(366);
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

SIGParser::Inherit_idContext::Inherit_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Inherit_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Inherit_idContext::getRuleIndex() const {
  return SIGParser::RuleInherit_id;
}

void SIGParser::Inherit_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit_id(this);
}

void SIGParser::Inherit_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit_id(this);
}


std::any SIGParser::Inherit_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitInherit_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Inherit_idContext* SIGParser::inherit_id() {
  Inherit_idContext *_localctx = _tracker.createInstance<Inherit_idContext>(_ctx, getState());
  enterRule(_localctx, 50, SIGParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(369);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Name_idContext ------------------------------------------------------------------

SIGParser::Name_idContext::Name_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Name_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Name_idContext::getRuleIndex() const {
  return SIGParser::RuleName_id;
}

void SIGParser::Name_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterName_id(this);
}

void SIGParser::Name_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitName_id(this);
}


std::any SIGParser::Name_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitName_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Name_idContext* SIGParser::name_id() {
  Name_idContext *_localctx = _tracker.createInstance<Name_idContext>(_ctx, getState());
  enterRule(_localctx, 52, SIGParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(371);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Option_idContext ------------------------------------------------------------------

SIGParser::Option_idContext::Option_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Option_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Option_idContext::getRuleIndex() const {
  return SIGParser::RuleOption_id;
}

void SIGParser::Option_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_id(this);
}

void SIGParser::Option_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_id(this);
}


std::any SIGParser::Option_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitOption_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Option_idContext* SIGParser::option_id() {
  Option_idContext *_localctx = _tracker.createInstance<Option_idContext>(_ctx, getState());
  enterRule(_localctx, 54, SIGParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(373);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Owner_idContext ------------------------------------------------------------------

SIGParser::Owner_idContext::Owner_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Owner_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Owner_idContext::getRuleIndex() const {
  return SIGParser::RuleOwner_id;
}

void SIGParser::Owner_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOwner_id(this);
}

void SIGParser::Owner_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOwner_id(this);
}


std::any SIGParser::Owner_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitOwner_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Owner_idContext* SIGParser::owner_id() {
  Owner_idContext *_localctx = _tracker.createInstance<Owner_idContext>(_ctx, getState());
  enterRule(_localctx, 56, SIGParser::RuleOwner_id);

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
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Template_idContext ------------------------------------------------------------------

SIGParser::Template_idContext::Template_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Template_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Template_idContext::getRuleIndex() const {
  return SIGParser::RuleTemplate_id;
}

void SIGParser::Template_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTemplate_id(this);
}

void SIGParser::Template_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTemplate_id(this);
}


std::any SIGParser::Template_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitTemplate_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Template_idContext* SIGParser::template_id() {
  Template_idContext *_localctx = _tracker.createInstance<Template_idContext>(_ctx, getState());
  enterRule(_localctx, 58, SIGParser::RuleTemplate_id);

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
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_idContext ------------------------------------------------------------------

SIGParser::Function_idContext::Function_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Function_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}

tree::TerminalNode* SIGParser::Function_idContext::OPAR() {
  return getToken(SIGParser::OPAR, 0);
}

tree::TerminalNode* SIGParser::Function_idContext::CPAR() {
  return getToken(SIGParser::CPAR, 0);
}

std::vector<SIGParser::Value_id_ignoreContext *> SIGParser::Function_idContext::value_id_ignore() {
  return getRuleContexts<SIGParser::Value_id_ignoreContext>();
}

SIGParser::Value_id_ignoreContext* SIGParser::Function_idContext::value_id_ignore(size_t i) {
  return getRuleContext<SIGParser::Value_id_ignoreContext>(i);
}


size_t SIGParser::Function_idContext::getRuleIndex() const {
  return SIGParser::RuleFunction_id;
}

void SIGParser::Function_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction_id(this);
}

void SIGParser::Function_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction_id(this);
}


std::any SIGParser::Function_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitFunction_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Function_idContext* SIGParser::function_id() {
  Function_idContext *_localctx = _tracker.createInstance<Function_idContext>(_ctx, getState());
  enterRule(_localctx, 60, SIGParser::RuleFunction_id);
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
    setState(379);
    match(SIGParser::ID);
    setState(380);
    match(SIGParser::OPAR);
    setState(382);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la - 70) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 70)) & 29360131) != 0) {
      setState(381);
      value_id_ignore();
    }
    setState(388);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__2) {
      setState(384);
      match(SIGParser::T__2);
      setState(385);
      value_id_ignore();
      setState(390);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(391);
    match(SIGParser::CPAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_idContext ------------------------------------------------------------------

SIGParser::Value_idContext::Value_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Shader_typeContext* SIGParser::Value_idContext::shader_type() {
  return getRuleContext<SIGParser::Shader_typeContext>(0);
}

tree::TerminalNode* SIGParser::Value_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}

tree::TerminalNode* SIGParser::Value_idContext::INT_SCALAR() {
  return getToken(SIGParser::INT_SCALAR, 0);
}

tree::TerminalNode* SIGParser::Value_idContext::FLOAT_SCALAR() {
  return getToken(SIGParser::FLOAT_SCALAR, 0);
}

SIGParser::Bool_typeContext* SIGParser::Value_idContext::bool_type() {
  return getRuleContext<SIGParser::Bool_typeContext>(0);
}

SIGParser::Function_idContext* SIGParser::Value_idContext::function_id() {
  return getRuleContext<SIGParser::Function_idContext>(0);
}

SIGParser::Array_value_idsContext* SIGParser::Value_idContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}


size_t SIGParser::Value_idContext::getRuleIndex() const {
  return SIGParser::RuleValue_id;
}

void SIGParser::Value_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_id(this);
}

void SIGParser::Value_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_id(this);
}


std::any SIGParser::Value_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitValue_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Value_idContext* SIGParser::value_id() {
  Value_idContext *_localctx = _tracker.createInstance<Value_idContext>(_ctx, getState());
  enterRule(_localctx, 62, SIGParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(400);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(393);
      shader_type();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(394);
      match(SIGParser::ID);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(395);
      match(SIGParser::INT_SCALAR);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(396);
      match(SIGParser::FLOAT_SCALAR);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(397);
      bool_type();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(398);
      function_id();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(399);
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

SIGParser::Value_id_ignoreContext::Value_id_ignoreContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Value_id_ignoreContext::ID() {
  return getToken(SIGParser::ID, 0);
}

tree::TerminalNode* SIGParser::Value_id_ignoreContext::INT_SCALAR() {
  return getToken(SIGParser::INT_SCALAR, 0);
}

tree::TerminalNode* SIGParser::Value_id_ignoreContext::FLOAT_SCALAR() {
  return getToken(SIGParser::FLOAT_SCALAR, 0);
}

SIGParser::Bool_typeContext* SIGParser::Value_id_ignoreContext::bool_type() {
  return getRuleContext<SIGParser::Bool_typeContext>(0);
}


size_t SIGParser::Value_id_ignoreContext::getRuleIndex() const {
  return SIGParser::RuleValue_id_ignore;
}

void SIGParser::Value_id_ignoreContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_id_ignore(this);
}

void SIGParser::Value_id_ignoreContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_id_ignore(this);
}


std::any SIGParser::Value_id_ignoreContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitValue_id_ignore(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Value_id_ignoreContext* SIGParser::value_id_ignore() {
  Value_id_ignoreContext *_localctx = _tracker.createInstance<Value_id_ignoreContext>(_ctx, getState());
  enterRule(_localctx, 64, SIGParser::RuleValue_id_ignore);

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
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(402);
        match(SIGParser::ID);
        break;
      }

      case SIGParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(403);
        match(SIGParser::INT_SCALAR);
        break;
      }

      case SIGParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(404);
        match(SIGParser::FLOAT_SCALAR);
        break;
      }

      case SIGParser::TRUE:
      case SIGParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(405);
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

SIGParser::Type_idContext::Type_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_with_templateContext* SIGParser::Type_idContext::type_with_template() {
  return getRuleContext<SIGParser::Type_with_templateContext>(0);
}


size_t SIGParser::Type_idContext::getRuleIndex() const {
  return SIGParser::RuleType_id;
}

void SIGParser::Type_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_id(this);
}

void SIGParser::Type_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_id(this);
}


std::any SIGParser::Type_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitType_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Type_idContext* SIGParser::type_id() {
  Type_idContext *_localctx = _tracker.createInstance<Type_idContext>(_ctx, getState());
  enterRule(_localctx, 66, SIGParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(408);
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

SIGParser::Insert_blockContext::Insert_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Insert_blockContext::INSERT_BLOCK() {
  return getToken(SIGParser::INSERT_BLOCK, 0);
}


size_t SIGParser::Insert_blockContext::getRuleIndex() const {
  return SIGParser::RuleInsert_block;
}

void SIGParser::Insert_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInsert_block(this);
}

void SIGParser::Insert_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInsert_block(this);
}


std::any SIGParser::Insert_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitInsert_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Insert_blockContext* SIGParser::insert_block() {
  Insert_blockContext *_localctx = _tracker.createInstance<Insert_blockContext>(_ctx, getState());
  enterRule(_localctx, 68, SIGParser::RuleInsert_block);

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
    match(SIGParser::INSERT_BLOCK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Path_idContext ------------------------------------------------------------------

SIGParser::Path_idContext::Path_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SIGParser::Path_idContext::ID() {
  return getTokens(SIGParser::ID);
}

tree::TerminalNode* SIGParser::Path_idContext::ID(size_t i) {
  return getToken(SIGParser::ID, i);
}

std::vector<tree::TerminalNode *> SIGParser::Path_idContext::DIV() {
  return getTokens(SIGParser::DIV);
}

tree::TerminalNode* SIGParser::Path_idContext::DIV(size_t i) {
  return getToken(SIGParser::DIV, i);
}


size_t SIGParser::Path_idContext::getRuleIndex() const {
  return SIGParser::RulePath_id;
}

void SIGParser::Path_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPath_id(this);
}

void SIGParser::Path_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPath_id(this);
}


std::any SIGParser::Path_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPath_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Path_idContext* SIGParser::path_id() {
  Path_idContext *_localctx = _tracker.createInstance<Path_idContext>(_ctx, getState());
  enterRule(_localctx, 70, SIGParser::RulePath_id);

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
    setState(416);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(412);
        match(SIGParser::ID);
        setState(413);
        match(SIGParser::DIV); 
      }
      setState(418);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    }
    setState(419);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InheritContext ------------------------------------------------------------------

SIGParser::InheritContext::InheritContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Inherit_idContext *> SIGParser::InheritContext::inherit_id() {
  return getRuleContexts<SIGParser::Inherit_idContext>();
}

SIGParser::Inherit_idContext* SIGParser::InheritContext::inherit_id(size_t i) {
  return getRuleContext<SIGParser::Inherit_idContext>(i);
}


size_t SIGParser::InheritContext::getRuleIndex() const {
  return SIGParser::RuleInherit;
}

void SIGParser::InheritContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit(this);
}

void SIGParser::InheritContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit(this);
}


std::any SIGParser::InheritContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitInherit(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::InheritContext* SIGParser::inherit() {
  InheritContext *_localctx = _tracker.createInstance<InheritContext>(_ctx, getState());
  enterRule(_localctx, 72, SIGParser::RuleInherit);

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
    setState(421);
    match(SIGParser::T__7);
    setState(422);
    inherit_id();
    setState(427);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(423);
        match(SIGParser::T__2);
        setState(424);
        inherit_id(); 
      }
      setState(429);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
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

SIGParser::Layout_statContext::Layout_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Slot_declarationContext* SIGParser::Layout_statContext::slot_declaration() {
  return getRuleContext<SIGParser::Slot_declarationContext>(0);
}

SIGParser::Sampler_declarationContext* SIGParser::Layout_statContext::sampler_declaration() {
  return getRuleContext<SIGParser::Sampler_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Layout_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Layout_statContext::getRuleIndex() const {
  return SIGParser::RuleLayout_stat;
}

void SIGParser::Layout_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_stat(this);
}

void SIGParser::Layout_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_stat(this);
}


std::any SIGParser::Layout_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitLayout_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Layout_statContext* SIGParser::layout_stat() {
  Layout_statContext *_localctx = _tracker.createInstance<Layout_statContext>(_ctx, getState());
  enterRule(_localctx, 74, SIGParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(433);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(430);
        slot_declaration();
        break;
      }

      case SIGParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(431);
        sampler_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(432);
        match(SIGParser::COMMENT);
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

SIGParser::Layout_blockContext::Layout_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Layout_statContext *> SIGParser::Layout_blockContext::layout_stat() {
  return getRuleContexts<SIGParser::Layout_statContext>();
}

SIGParser::Layout_statContext* SIGParser::Layout_blockContext::layout_stat(size_t i) {
  return getRuleContext<SIGParser::Layout_statContext>(i);
}


size_t SIGParser::Layout_blockContext::getRuleIndex() const {
  return SIGParser::RuleLayout_block;
}

void SIGParser::Layout_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_block(this);
}

void SIGParser::Layout_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_block(this);
}


std::any SIGParser::Layout_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitLayout_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Layout_blockContext* SIGParser::layout_block() {
  Layout_blockContext *_localctx = _tracker.createInstance<Layout_blockContext>(_ctx, getState());
  enterRule(_localctx, 76, SIGParser::RuleLayout_block);
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
    setState(438);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__3 || _la == SIGParser::SLOT

    || _la == SIGParser::COMMENT) {
      setState(435);
      layout_stat();
      setState(440);
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

SIGParser::Layout_definitionContext::Layout_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Layout_definitionContext::LAYOUT() {
  return getToken(SIGParser::LAYOUT, 0);
}

SIGParser::Name_idContext* SIGParser::Layout_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Layout_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Layout_blockContext* SIGParser::Layout_definitionContext::layout_block() {
  return getRuleContext<SIGParser::Layout_blockContext>(0);
}

tree::TerminalNode* SIGParser::Layout_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Layout_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Layout_definitionContext::getRuleIndex() const {
  return SIGParser::RuleLayout_definition;
}

void SIGParser::Layout_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_definition(this);
}

void SIGParser::Layout_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_definition(this);
}


std::any SIGParser::Layout_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitLayout_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Layout_definitionContext* SIGParser::layout_definition() {
  Layout_definitionContext *_localctx = _tracker.createInstance<Layout_definitionContext>(_ctx, getState());
  enterRule(_localctx, 78, SIGParser::RuleLayout_definition);
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
    setState(441);
    match(SIGParser::LAYOUT);
    setState(442);
    name_id();
    setState(444);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(443);
      inherit();
    }
    setState(446);
    match(SIGParser::OBRACE);
    setState(447);
    layout_block();
    setState(448);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_statContext ------------------------------------------------------------------

SIGParser::Table_statContext::Table_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_declarationContext* SIGParser::Table_statContext::value_declaration() {
  return getRuleContext<SIGParser::Value_declarationContext>(0);
}

SIGParser::Insert_blockContext* SIGParser::Table_statContext::insert_block() {
  return getRuleContext<SIGParser::Insert_blockContext>(0);
}

tree::TerminalNode* SIGParser::Table_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Table_statContext::getRuleIndex() const {
  return SIGParser::RuleTable_stat;
}

void SIGParser::Table_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_stat(this);
}

void SIGParser::Table_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_stat(this);
}


std::any SIGParser::Table_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitTable_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Table_statContext* SIGParser::table_stat() {
  Table_statContext *_localctx = _tracker.createInstance<Table_statContext>(_ctx, getState());
  enterRule(_localctx, 80, SIGParser::RuleTable_stat);

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
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(450);
        value_declaration();
        break;
      }

      case SIGParser::INSERT_BLOCK: {
        enterOuterAlt(_localctx, 2);
        setState(451);
        insert_block();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(452);
        match(SIGParser::COMMENT);
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

//----------------- Table_blockContext ------------------------------------------------------------------

SIGParser::Table_blockContext::Table_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Table_statContext *> SIGParser::Table_blockContext::table_stat() {
  return getRuleContexts<SIGParser::Table_statContext>();
}

SIGParser::Table_statContext* SIGParser::Table_blockContext::table_stat(size_t i) {
  return getRuleContext<SIGParser::Table_statContext>(i);
}


size_t SIGParser::Table_blockContext::getRuleIndex() const {
  return SIGParser::RuleTable_block;
}

void SIGParser::Table_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_block(this);
}

void SIGParser::Table_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_block(this);
}


std::any SIGParser::Table_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitTable_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Table_blockContext* SIGParser::table_block() {
  Table_blockContext *_localctx = _tracker.createInstance<Table_blockContext>(_ctx, getState());
  enterRule(_localctx, 82, SIGParser::RuleTable_block);
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
    setState(458);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 17733517313) != 0) {
      setState(455);
      table_stat();
      setState(460);
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

SIGParser::Table_definitionContext::Table_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Table_definitionContext::STRUCT() {
  return getToken(SIGParser::STRUCT, 0);
}

SIGParser::Name_idContext* SIGParser::Table_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Table_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Table_blockContext* SIGParser::Table_definitionContext::table_block() {
  return getRuleContext<SIGParser::Table_blockContext>(0);
}

tree::TerminalNode* SIGParser::Table_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Table_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Table_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Table_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Table_definitionContext::getRuleIndex() const {
  return SIGParser::RuleTable_definition;
}

void SIGParser::Table_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_definition(this);
}

void SIGParser::Table_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_definition(this);
}


std::any SIGParser::Table_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitTable_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Table_definitionContext* SIGParser::table_definition() {
  Table_definitionContext *_localctx = _tracker.createInstance<Table_definitionContext>(_ctx, getState());
  enterRule(_localctx, 84, SIGParser::RuleTable_definition);
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
    setState(464);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(461);
        option_block(); 
      }
      setState(466);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
    setState(467);
    match(SIGParser::STRUCT);
    setState(468);
    name_id();
    setState(470);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(469);
      inherit();
    }
    setState(472);
    match(SIGParser::OBRACE);
    setState(473);
    table_block();
    setState(474);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_color_declarationContext ------------------------------------------------------------------

SIGParser::Rt_color_declarationContext::Rt_color_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_idContext* SIGParser::Rt_color_declarationContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::Rt_color_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_color_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Rt_color_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRt_color_declaration;
}

void SIGParser::Rt_color_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_color_declaration(this);
}

void SIGParser::Rt_color_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_color_declaration(this);
}


std::any SIGParser::Rt_color_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRt_color_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rt_color_declarationContext* SIGParser::rt_color_declaration() {
  Rt_color_declarationContext *_localctx = _tracker.createInstance<Rt_color_declarationContext>(_ctx, getState());
  enterRule(_localctx, 86, SIGParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(476);
    type_id();
    setState(477);
    name_id();
    setState(478);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_ds_declarationContext ------------------------------------------------------------------

SIGParser::Rt_ds_declarationContext::Rt_ds_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rt_ds_declarationContext::DSV() {
  return getToken(SIGParser::DSV, 0);
}

SIGParser::Name_idContext* SIGParser::Rt_ds_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_ds_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Rt_ds_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRt_ds_declaration;
}

void SIGParser::Rt_ds_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_ds_declaration(this);
}

void SIGParser::Rt_ds_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_ds_declaration(this);
}


std::any SIGParser::Rt_ds_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRt_ds_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rt_ds_declarationContext* SIGParser::rt_ds_declaration() {
  Rt_ds_declarationContext *_localctx = _tracker.createInstance<Rt_ds_declarationContext>(_ctx, getState());
  enterRule(_localctx, 88, SIGParser::RuleRt_ds_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(480);
    match(SIGParser::DSV);
    setState(481);
    name_id();
    setState(482);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_statContext ------------------------------------------------------------------

SIGParser::Rt_statContext::Rt_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Rt_color_declarationContext* SIGParser::Rt_statContext::rt_color_declaration() {
  return getRuleContext<SIGParser::Rt_color_declarationContext>(0);
}

SIGParser::Rt_ds_declarationContext* SIGParser::Rt_statContext::rt_ds_declaration() {
  return getRuleContext<SIGParser::Rt_ds_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Rt_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rt_statContext::getRuleIndex() const {
  return SIGParser::RuleRt_stat;
}

void SIGParser::Rt_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_stat(this);
}

void SIGParser::Rt_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_stat(this);
}


std::any SIGParser::Rt_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRt_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rt_statContext* SIGParser::rt_stat() {
  Rt_statContext *_localctx = _tracker.createInstance<Rt_statContext>(_ctx, getState());
  enterRule(_localctx, 90, SIGParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(487);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(484);
        rt_color_declaration();
        break;
      }

      case SIGParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(485);
        rt_ds_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(486);
        match(SIGParser::COMMENT);
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

SIGParser::Rt_blockContext::Rt_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rt_statContext *> SIGParser::Rt_blockContext::rt_stat() {
  return getRuleContexts<SIGParser::Rt_statContext>();
}

SIGParser::Rt_statContext* SIGParser::Rt_blockContext::rt_stat(size_t i) {
  return getRuleContext<SIGParser::Rt_statContext>(i);
}


size_t SIGParser::Rt_blockContext::getRuleIndex() const {
  return SIGParser::RuleRt_block;
}

void SIGParser::Rt_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_block(this);
}

void SIGParser::Rt_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_block(this);
}


std::any SIGParser::Rt_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRt_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rt_blockContext* SIGParser::rt_block() {
  Rt_blockContext *_localctx = _tracker.createInstance<Rt_blockContext>(_ctx, getState());
  enterRule(_localctx, 92, SIGParser::RuleRt_block);
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
    setState(492);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & 265) != 0) {
      setState(489);
      rt_stat();
      setState(494);
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

SIGParser::Rt_definitionContext::Rt_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rt_definitionContext::RT() {
  return getToken(SIGParser::RT, 0);
}

SIGParser::Name_idContext* SIGParser::Rt_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rt_blockContext* SIGParser::Rt_definitionContext::rt_block() {
  return getRuleContext<SIGParser::Rt_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rt_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Rt_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRt_definition;
}

void SIGParser::Rt_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_definition(this);
}

void SIGParser::Rt_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_definition(this);
}


std::any SIGParser::Rt_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRt_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rt_definitionContext* SIGParser::rt_definition() {
  Rt_definitionContext *_localctx = _tracker.createInstance<Rt_definitionContext>(_ctx, getState());
  enterRule(_localctx, 94, SIGParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(495);
    match(SIGParser::RT);
    setState(496);
    name_id();
    setState(497);
    match(SIGParser::OBRACE);
    setState(498);
    rt_block();
    setState(499);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_value_holderContext ------------------------------------------------------------------

SIGParser::Array_value_holderContext::Array_value_holderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_idContext* SIGParser::Array_value_holderContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Array_value_holderContext::getRuleIndex() const {
  return SIGParser::RuleArray_value_holder;
}

void SIGParser::Array_value_holderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_holder(this);
}

void SIGParser::Array_value_holderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_holder(this);
}


std::any SIGParser::Array_value_holderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitArray_value_holder(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Array_value_holderContext* SIGParser::array_value_holder() {
  Array_value_holderContext *_localctx = _tracker.createInstance<Array_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 96, SIGParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(501);
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

SIGParser::Array_value_idsContext::Array_value_idsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Array_value_idsContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

std::vector<SIGParser::Array_value_holderContext *> SIGParser::Array_value_idsContext::array_value_holder() {
  return getRuleContexts<SIGParser::Array_value_holderContext>();
}

SIGParser::Array_value_holderContext* SIGParser::Array_value_idsContext::array_value_holder(size_t i) {
  return getRuleContext<SIGParser::Array_value_holderContext>(i);
}

tree::TerminalNode* SIGParser::Array_value_idsContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Array_value_idsContext::getRuleIndex() const {
  return SIGParser::RuleArray_value_ids;
}

void SIGParser::Array_value_idsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_ids(this);
}

void SIGParser::Array_value_idsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_ids(this);
}


std::any SIGParser::Array_value_idsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitArray_value_ids(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Array_value_idsContext* SIGParser::array_value_ids() {
  Array_value_idsContext *_localctx = _tracker.createInstance<Array_value_idsContext>(_ctx, getState());
  enterRule(_localctx, 98, SIGParser::RuleArray_value_ids);
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
    setState(503);
    match(SIGParser::OBRACE);
    setState(504);
    array_value_holder();
    setState(509);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__2) {
      setState(505);
      match(SIGParser::T__2);
      setState(506);
      array_value_holder();
      setState(511);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(512);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Root_sigContext ------------------------------------------------------------------

SIGParser::Root_sigContext::Root_sigContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Root_sigContext::ROOTSIG() {
  return getToken(SIGParser::ROOTSIG, 0);
}

tree::TerminalNode* SIGParser::Root_sigContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Name_idContext* SIGParser::Root_sigContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Root_sigContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Root_sigContext::getRuleIndex() const {
  return SIGParser::RuleRoot_sig;
}

void SIGParser::Root_sigContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRoot_sig(this);
}

void SIGParser::Root_sigContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRoot_sig(this);
}


std::any SIGParser::Root_sigContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRoot_sig(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Root_sigContext* SIGParser::root_sig() {
  Root_sigContext *_localctx = _tracker.createInstance<Root_sigContext>(_ctx, getState());
  enterRule(_localctx, 100, SIGParser::RuleRoot_sig);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(514);
    match(SIGParser::ROOTSIG);
    setState(515);
    match(SIGParser::ASSIGN);
    setState(516);
    name_id();
    setState(517);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShaderContext ------------------------------------------------------------------

SIGParser::ShaderContext::ShaderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Shader_typeContext* SIGParser::ShaderContext::shader_type() {
  return getRuleContext<SIGParser::Shader_typeContext>(0);
}

tree::TerminalNode* SIGParser::ShaderContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Path_idContext* SIGParser::ShaderContext::path_id() {
  return getRuleContext<SIGParser::Path_idContext>(0);
}

tree::TerminalNode* SIGParser::ShaderContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::ShaderContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::ShaderContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::ShaderContext::getRuleIndex() const {
  return SIGParser::RuleShader;
}

void SIGParser::ShaderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader(this);
}

void SIGParser::ShaderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader(this);
}


std::any SIGParser::ShaderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitShader(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::ShaderContext* SIGParser::shader() {
  ShaderContext *_localctx = _tracker.createInstance<ShaderContext>(_ctx, getState());
  enterRule(_localctx, 102, SIGParser::RuleShader);

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
    setState(522);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(519);
        option_block(); 
      }
      setState(524);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
    setState(525);
    shader_type();
    setState(526);
    match(SIGParser::ASSIGN);
    setState(527);
    path_id();
    setState(528);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_statContext ------------------------------------------------------------------

SIGParser::Compute_pso_statContext::Compute_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Compute_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

SIGParser::ShaderContext* SIGParser::Compute_pso_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

SIGParser::Define_declarationContext* SIGParser::Compute_pso_statContext::define_declaration() {
  return getRuleContext<SIGParser::Define_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Compute_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_stat;
}

void SIGParser::Compute_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_stat(this);
}

void SIGParser::Compute_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_stat(this);
}


std::any SIGParser::Compute_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Compute_pso_statContext* SIGParser::compute_pso_stat() {
  Compute_pso_statContext *_localctx = _tracker.createInstance<Compute_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 104, SIGParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(534);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(530);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(531);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(532);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(533);
      match(SIGParser::COMMENT);
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

SIGParser::Compute_pso_blockContext::Compute_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Compute_pso_statContext *> SIGParser::Compute_pso_blockContext::compute_pso_stat() {
  return getRuleContexts<SIGParser::Compute_pso_statContext>();
}

SIGParser::Compute_pso_statContext* SIGParser::Compute_pso_blockContext::compute_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Compute_pso_statContext>(i);
}


size_t SIGParser::Compute_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_block;
}

void SIGParser::Compute_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_block(this);
}

void SIGParser::Compute_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_block(this);
}


std::any SIGParser::Compute_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Compute_pso_blockContext* SIGParser::compute_pso_block() {
  Compute_pso_blockContext *_localctx = _tracker.createInstance<Compute_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 106, SIGParser::RuleCompute_pso_block);
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
    setState(539);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 134201376) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(536);
      compute_pso_stat();
      setState(541);
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

SIGParser::Compute_pso_definitionContext::Compute_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::COMPUTE_PSO() {
  return getToken(SIGParser::COMPUTE_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Compute_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Compute_pso_blockContext* SIGParser::Compute_pso_definitionContext::compute_pso_block() {
  return getRuleContext<SIGParser::Compute_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Compute_pso_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Compute_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Compute_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Compute_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_definition;
}

void SIGParser::Compute_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_definition(this);
}

void SIGParser::Compute_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_definition(this);
}


std::any SIGParser::Compute_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitCompute_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Compute_pso_definitionContext* SIGParser::compute_pso_definition() {
  Compute_pso_definitionContext *_localctx = _tracker.createInstance<Compute_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 108, SIGParser::RuleCompute_pso_definition);
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
    setState(545);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(542);
        option_block(); 
      }
      setState(547);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
    setState(548);
    match(SIGParser::COMPUTE_PSO);
    setState(549);
    name_id();
    setState(551);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(550);
      inherit();
    }
    setState(553);
    match(SIGParser::OBRACE);
    setState(554);
    compute_pso_block();
    setState(555);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_statContext ------------------------------------------------------------------

SIGParser::Graphics_pso_statContext::Graphics_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Graphics_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

SIGParser::ShaderContext* SIGParser::Graphics_pso_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

SIGParser::Define_declarationContext* SIGParser::Graphics_pso_statContext::define_declaration() {
  return getRuleContext<SIGParser::Define_declarationContext>(0);
}

SIGParser::Rtv_formats_declarationContext* SIGParser::Graphics_pso_statContext::rtv_formats_declaration() {
  return getRuleContext<SIGParser::Rtv_formats_declarationContext>(0);
}

SIGParser::Blends_declarationContext* SIGParser::Graphics_pso_statContext::blends_declaration() {
  return getRuleContext<SIGParser::Blends_declarationContext>(0);
}

SIGParser::Pso_paramContext* SIGParser::Graphics_pso_statContext::pso_param() {
  return getRuleContext<SIGParser::Pso_paramContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Graphics_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_stat;
}

void SIGParser::Graphics_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_stat(this);
}

void SIGParser::Graphics_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_stat(this);
}


std::any SIGParser::Graphics_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Graphics_pso_statContext* SIGParser::graphics_pso_stat() {
  Graphics_pso_statContext *_localctx = _tracker.createInstance<Graphics_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 110, SIGParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(564);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(557);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(558);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(559);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(560);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(561);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(562);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(563);
      match(SIGParser::COMMENT);
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

SIGParser::Graphics_pso_blockContext::Graphics_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Graphics_pso_statContext *> SIGParser::Graphics_pso_blockContext::graphics_pso_stat() {
  return getRuleContexts<SIGParser::Graphics_pso_statContext>();
}

SIGParser::Graphics_pso_statContext* SIGParser::Graphics_pso_blockContext::graphics_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Graphics_pso_statContext>(i);
}


size_t SIGParser::Graphics_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_block;
}

void SIGParser::Graphics_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_block(this);
}

void SIGParser::Graphics_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_block(this);
}


std::any SIGParser::Graphics_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Graphics_pso_blockContext* SIGParser::graphics_pso_block() {
  Graphics_pso_blockContext *_localctx = _tracker.createInstance<Graphics_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 112, SIGParser::RuleGraphics_pso_block);
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
    setState(569);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 70368744161504) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541065217) != 0) {
      setState(566);
      graphics_pso_stat();
      setState(571);
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

SIGParser::Graphics_pso_definitionContext::Graphics_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::GRAPHICS_PSO() {
  return getToken(SIGParser::GRAPHICS_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Graphics_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Graphics_pso_blockContext* SIGParser::Graphics_pso_definitionContext::graphics_pso_block() {
  return getRuleContext<SIGParser::Graphics_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Graphics_pso_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Graphics_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Graphics_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Graphics_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_definition;
}

void SIGParser::Graphics_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_definition(this);
}

void SIGParser::Graphics_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_definition(this);
}


std::any SIGParser::Graphics_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitGraphics_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Graphics_pso_definitionContext* SIGParser::graphics_pso_definition() {
  Graphics_pso_definitionContext *_localctx = _tracker.createInstance<Graphics_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 114, SIGParser::RuleGraphics_pso_definition);
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
    setState(575);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(572);
        option_block(); 
      }
      setState(577);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    }
    setState(578);
    match(SIGParser::GRAPHICS_PSO);
    setState(579);
    name_id();
    setState(581);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(580);
      inherit();
    }
    setState(583);
    match(SIGParser::OBRACE);
    setState(584);
    graphics_pso_block();
    setState(585);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_statContext ------------------------------------------------------------------

SIGParser::Rtx_pso_statContext::Rtx_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Rtx_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rtx_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_stat;
}

void SIGParser::Rtx_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_stat(this);
}

void SIGParser::Rtx_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_stat(this);
}


std::any SIGParser::Rtx_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pso_statContext* SIGParser::rtx_pso_stat() {
  Rtx_pso_statContext *_localctx = _tracker.createInstance<Rtx_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 116, SIGParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(589);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(587);
        root_sig();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(588);
        match(SIGParser::COMMENT);
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

SIGParser::Rtx_pso_blockContext::Rtx_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_pso_statContext *> SIGParser::Rtx_pso_blockContext::rtx_pso_stat() {
  return getRuleContexts<SIGParser::Rtx_pso_statContext>();
}

SIGParser::Rtx_pso_statContext* SIGParser::Rtx_pso_blockContext::rtx_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_pso_statContext>(i);
}


size_t SIGParser::Rtx_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_block;
}

void SIGParser::Rtx_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_block(this);
}

void SIGParser::Rtx_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_block(this);
}


std::any SIGParser::Rtx_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pso_blockContext* SIGParser::rtx_pso_block() {
  Rtx_pso_blockContext *_localctx = _tracker.createInstance<Rtx_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 118, SIGParser::RuleRtx_pso_block);
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
    setState(594);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(591);
      rtx_pso_stat();
      setState(596);
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

SIGParser::Rtx_pso_definitionContext::Rtx_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::RAYTRACE_PSO() {
  return getToken(SIGParser::RAYTRACE_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_pso_blockContext* SIGParser::Rtx_pso_definitionContext::rtx_pso_block() {
  return getRuleContext<SIGParser::Rtx_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Rtx_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_definition;
}

void SIGParser::Rtx_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_definition(this);
}

void SIGParser::Rtx_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_definition(this);
}


std::any SIGParser::Rtx_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pso_definitionContext* SIGParser::rtx_pso_definition() {
  Rtx_pso_definitionContext *_localctx = _tracker.createInstance<Rtx_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 120, SIGParser::RuleRtx_pso_definition);
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
    setState(597);
    match(SIGParser::RAYTRACE_PSO);
    setState(598);
    name_id();
    setState(600);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(599);
      inherit();
    }
    setState(602);
    match(SIGParser::OBRACE);
    setState(603);
    rtx_pso_block();
    setState(604);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_param_idContext ------------------------------------------------------------------

SIGParser::Node_param_idContext::Node_param_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SIGParser::Node_param_idContext::getRuleIndex() const {
  return SIGParser::RuleNode_param_id;
}

void SIGParser::Node_param_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_param_id(this);
}

void SIGParser::Node_param_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_param_id(this);
}


std::any SIGParser::Node_param_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_param_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_param_idContext* SIGParser::node_param_id() {
  Node_param_idContext *_localctx = _tracker.createInstance<Node_param_idContext>(_ctx, getState());
  enterRule(_localctx, 122, SIGParser::RuleNode_param_id);
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
    setState(606);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15872) != 0)) {
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

SIGParser::Node_paramContext::Node_paramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Node_param_idContext* SIGParser::Node_paramContext::node_param_id() {
  return getRuleContext<SIGParser::Node_param_idContext>(0);
}

tree::TerminalNode* SIGParser::Node_paramContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Node_paramContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

tree::TerminalNode* SIGParser::Node_paramContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Node_paramContext::getRuleIndex() const {
  return SIGParser::RuleNode_param;
}

void SIGParser::Node_paramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_param(this);
}

void SIGParser::Node_paramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_param(this);
}


std::any SIGParser::Node_paramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_param(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_paramContext* SIGParser::node_param() {
  Node_paramContext *_localctx = _tracker.createInstance<Node_paramContext>(_ctx, getState());
  enterRule(_localctx, 124, SIGParser::RuleNode_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(608);
    node_param_id();
    setState(609);
    match(SIGParser::ASSIGN);
    setState(610);
    value_id();
    setState(611);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_output_declContext ------------------------------------------------------------------

SIGParser::Node_output_declContext::Node_output_declContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Node_output_declContext::NODE_OUTPUT() {
  return getToken(SIGParser::NODE_OUTPUT, 0);
}

SIGParser::Type_idContext* SIGParser::Node_output_declContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::Node_output_declContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Node_output_declContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Node_output_declContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Node_output_declContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Node_output_declContext::getRuleIndex() const {
  return SIGParser::RuleNode_output_decl;
}

void SIGParser::Node_output_declContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_output_decl(this);
}

void SIGParser::Node_output_declContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_output_decl(this);
}


std::any SIGParser::Node_output_declContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_output_decl(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_output_declContext* SIGParser::node_output_decl() {
  Node_output_declContext *_localctx = _tracker.createInstance<Node_output_declContext>(_ctx, getState());
  enterRule(_localctx, 126, SIGParser::RuleNode_output_decl);

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
    setState(616);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(613);
        option_block(); 
      }
      setState(618);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    }
    setState(619);
    match(SIGParser::NODE_OUTPUT);
    setState(620);
    type_id();
    setState(621);
    name_id();
    setState(622);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Node_statContext ------------------------------------------------------------------

SIGParser::Node_statContext::Node_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Node_paramContext* SIGParser::Node_statContext::node_param() {
  return getRuleContext<SIGParser::Node_paramContext>(0);
}

SIGParser::Node_output_declContext* SIGParser::Node_statContext::node_output_decl() {
  return getRuleContext<SIGParser::Node_output_declContext>(0);
}

tree::TerminalNode* SIGParser::Node_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Node_statContext::getRuleIndex() const {
  return SIGParser::RuleNode_stat;
}

void SIGParser::Node_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_stat(this);
}

void SIGParser::Node_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_stat(this);
}


std::any SIGParser::Node_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_statContext* SIGParser::node_stat() {
  Node_statContext *_localctx = _tracker.createInstance<Node_statContext>(_ctx, getState());
  enterRule(_localctx, 128, SIGParser::RuleNode_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(627);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::T__8:
      case SIGParser::T__9:
      case SIGParser::T__10:
      case SIGParser::T__11:
      case SIGParser::T__12: {
        enterOuterAlt(_localctx, 1);
        setState(624);
        node_param();
        break;
      }

      case SIGParser::OSBRACE:
      case SIGParser::NODE_OUTPUT: {
        enterOuterAlt(_localctx, 2);
        setState(625);
        node_output_decl();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(626);
        match(SIGParser::COMMENT);
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

SIGParser::Node_blockContext::Node_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Node_statContext *> SIGParser::Node_blockContext::node_stat() {
  return getRuleContexts<SIGParser::Node_statContext>();
}

SIGParser::Node_statContext* SIGParser::Node_blockContext::node_stat(size_t i) {
  return getRuleContext<SIGParser::Node_statContext>(i);
}


size_t SIGParser::Node_blockContext::getRuleIndex() const {
  return SIGParser::RuleNode_block;
}

void SIGParser::Node_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_block(this);
}

void SIGParser::Node_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_block(this);
}


std::any SIGParser::Node_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_blockContext* SIGParser::node_block() {
  Node_blockContext *_localctx = _tracker.createInstance<Node_blockContext>(_ctx, getState());
  enterRule(_localctx, 130, SIGParser::RuleNode_block);
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
    setState(632);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15872) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 536875009) != 0) {
      setState(629);
      node_stat();
      setState(634);
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

SIGParser::Node_definitionContext::Node_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Node_definitionContext::NODE() {
  return getToken(SIGParser::NODE, 0);
}

SIGParser::Name_idContext* SIGParser::Node_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Node_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Node_blockContext* SIGParser::Node_definitionContext::node_block() {
  return getRuleContext<SIGParser::Node_blockContext>(0);
}

tree::TerminalNode* SIGParser::Node_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Node_definitionContext::getRuleIndex() const {
  return SIGParser::RuleNode_definition;
}

void SIGParser::Node_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNode_definition(this);
}

void SIGParser::Node_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNode_definition(this);
}


std::any SIGParser::Node_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitNode_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Node_definitionContext* SIGParser::node_definition() {
  Node_definitionContext *_localctx = _tracker.createInstance<Node_definitionContext>(_ctx, getState());
  enterRule(_localctx, 132, SIGParser::RuleNode_definition);

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
    match(SIGParser::NODE);
    setState(636);
    name_id();
    setState(637);
    match(SIGParser::OBRACE);
    setState(638);
    node_block();
    setState(639);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Workgraph_pso_statContext ------------------------------------------------------------------

SIGParser::Workgraph_pso_statContext::Workgraph_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Workgraph_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

SIGParser::ShaderContext* SIGParser::Workgraph_pso_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

SIGParser::Define_declarationContext* SIGParser::Workgraph_pso_statContext::define_declaration() {
  return getRuleContext<SIGParser::Define_declarationContext>(0);
}

SIGParser::Node_definitionContext* SIGParser::Workgraph_pso_statContext::node_definition() {
  return getRuleContext<SIGParser::Node_definitionContext>(0);
}

tree::TerminalNode* SIGParser::Workgraph_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Workgraph_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleWorkgraph_pso_stat;
}

void SIGParser::Workgraph_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_stat(this);
}

void SIGParser::Workgraph_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_stat(this);
}


std::any SIGParser::Workgraph_pso_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Workgraph_pso_statContext* SIGParser::workgraph_pso_stat() {
  Workgraph_pso_statContext *_localctx = _tracker.createInstance<Workgraph_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 134, SIGParser::RuleWorkgraph_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(646);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(641);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(642);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(643);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(644);
      node_definition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(645);
      match(SIGParser::COMMENT);
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

SIGParser::Workgraph_pso_blockContext::Workgraph_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Workgraph_pso_statContext *> SIGParser::Workgraph_pso_blockContext::workgraph_pso_stat() {
  return getRuleContexts<SIGParser::Workgraph_pso_statContext>();
}

SIGParser::Workgraph_pso_statContext* SIGParser::Workgraph_pso_blockContext::workgraph_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Workgraph_pso_statContext>(i);
}


size_t SIGParser::Workgraph_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleWorkgraph_pso_block;
}

void SIGParser::Workgraph_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_block(this);
}

void SIGParser::Workgraph_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_block(this);
}


std::any SIGParser::Workgraph_pso_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Workgraph_pso_blockContext* SIGParser::workgraph_pso_block() {
  Workgraph_pso_blockContext *_localctx = _tracker.createInstance<Workgraph_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 136, SIGParser::RuleWorkgraph_pso_block);
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
    setState(651);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 134201376) != 0 || (((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 541067265) != 0) {
      setState(648);
      workgraph_pso_stat();
      setState(653);
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

SIGParser::Workgraph_pso_definitionContext::Workgraph_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Workgraph_pso_definitionContext::WORKGRAPH_PSO() {
  return getToken(SIGParser::WORKGRAPH_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Workgraph_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Workgraph_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Workgraph_pso_blockContext* SIGParser::Workgraph_pso_definitionContext::workgraph_pso_block() {
  return getRuleContext<SIGParser::Workgraph_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Workgraph_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Workgraph_pso_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Workgraph_pso_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Workgraph_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Workgraph_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleWorkgraph_pso_definition;
}

void SIGParser::Workgraph_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWorkgraph_pso_definition(this);
}

void SIGParser::Workgraph_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWorkgraph_pso_definition(this);
}


std::any SIGParser::Workgraph_pso_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitWorkgraph_pso_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Workgraph_pso_definitionContext* SIGParser::workgraph_pso_definition() {
  Workgraph_pso_definitionContext *_localctx = _tracker.createInstance<Workgraph_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 138, SIGParser::RuleWorkgraph_pso_definition);
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
    setState(657);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(654);
        option_block(); 
      }
      setState(659);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    }
    setState(660);
    match(SIGParser::WORKGRAPH_PSO);
    setState(661);
    name_id();
    setState(663);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(662);
      inherit();
    }
    setState(665);
    match(SIGParser::OBRACE);
    setState(666);
    workgraph_pso_block();
    setState(667);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_statContext ------------------------------------------------------------------

SIGParser::Rtx_pass_statContext::Rtx_pass_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::ShaderContext* SIGParser::Rtx_pass_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}

SIGParser::Pso_paramContext* SIGParser::Rtx_pass_statContext::pso_param() {
  return getRuleContext<SIGParser::Pso_paramContext>(0);
}


size_t SIGParser::Rtx_pass_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_stat;
}

void SIGParser::Rtx_pass_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_stat(this);
}

void SIGParser::Rtx_pass_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_stat(this);
}


std::any SIGParser::Rtx_pass_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pass_statContext* SIGParser::rtx_pass_stat() {
  Rtx_pass_statContext *_localctx = _tracker.createInstance<Rtx_pass_statContext>(_ctx, getState());
  enterRule(_localctx, 140, SIGParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(672);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(669);
      shader();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(670);
      match(SIGParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(671);
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

SIGParser::Rtx_pass_blockContext::Rtx_pass_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_pass_statContext *> SIGParser::Rtx_pass_blockContext::rtx_pass_stat() {
  return getRuleContexts<SIGParser::Rtx_pass_statContext>();
}

SIGParser::Rtx_pass_statContext* SIGParser::Rtx_pass_blockContext::rtx_pass_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_pass_statContext>(i);
}


size_t SIGParser::Rtx_pass_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_block;
}

void SIGParser::Rtx_pass_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_block(this);
}

void SIGParser::Rtx_pass_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_block(this);
}


std::any SIGParser::Rtx_pass_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pass_blockContext* SIGParser::rtx_pass_block() {
  Rtx_pass_blockContext *_localctx = _tracker.createInstance<Rtx_pass_blockContext>(_ctx, getState());
  enterRule(_localctx, 142, SIGParser::RuleRtx_pass_block);
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
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 70368744161280) != 0 || _la == SIGParser::OSBRACE

    || _la == SIGParser::COMMENT) {
      setState(674);
      rtx_pass_stat();
      setState(679);
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

SIGParser::Rtx_pass_definitionContext::Rtx_pass_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::RAYTRACE_PASS() {
  return getToken(SIGParser::RAYTRACE_PASS, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_pass_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_pass_blockContext* SIGParser::Rtx_pass_definitionContext::rtx_pass_block() {
  return getRuleContext<SIGParser::Rtx_pass_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtx_pass_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtx_pass_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Rtx_pass_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_pass_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_definition;
}

void SIGParser::Rtx_pass_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_definition(this);
}

void SIGParser::Rtx_pass_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_definition(this);
}


std::any SIGParser::Rtx_pass_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_pass_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_pass_definitionContext* SIGParser::rtx_pass_definition() {
  Rtx_pass_definitionContext *_localctx = _tracker.createInstance<Rtx_pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 144, SIGParser::RuleRtx_pass_definition);
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
    setState(683);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(680);
        option_block(); 
      }
      setState(685);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    }
    setState(686);
    match(SIGParser::RAYTRACE_PASS);
    setState(687);
    name_id();
    setState(689);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(688);
      inherit();
    }
    setState(691);
    match(SIGParser::OBRACE);
    setState(692);
    rtx_pass_block();
    setState(693);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_statContext ------------------------------------------------------------------

SIGParser::Rtx_raygen_statContext::Rtx_raygen_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::ShaderContext* SIGParser::Rtx_raygen_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rtx_raygen_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_stat;
}

void SIGParser::Rtx_raygen_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_stat(this);
}

void SIGParser::Rtx_raygen_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_stat(this);
}


std::any SIGParser::Rtx_raygen_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_raygen_statContext* SIGParser::rtx_raygen_stat() {
  Rtx_raygen_statContext *_localctx = _tracker.createInstance<Rtx_raygen_statContext>(_ctx, getState());
  enterRule(_localctx, 146, SIGParser::RuleRtx_raygen_stat);

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
    switch (_input->LA(1)) {
      case SIGParser::T__13:
      case SIGParser::T__14:
      case SIGParser::T__15:
      case SIGParser::T__16:
      case SIGParser::T__17:
      case SIGParser::T__18:
      case SIGParser::T__19:
      case SIGParser::T__20:
      case SIGParser::T__21:
      case SIGParser::T__22:
      case SIGParser::T__23:
      case SIGParser::T__24:
      case SIGParser::T__25:
      case SIGParser::OSBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(695);
        shader();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(696);
        match(SIGParser::COMMENT);
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

SIGParser::Rtx_raygen_blockContext::Rtx_raygen_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_raygen_statContext *> SIGParser::Rtx_raygen_blockContext::rtx_raygen_stat() {
  return getRuleContexts<SIGParser::Rtx_raygen_statContext>();
}

SIGParser::Rtx_raygen_statContext* SIGParser::Rtx_raygen_blockContext::rtx_raygen_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_raygen_statContext>(i);
}


size_t SIGParser::Rtx_raygen_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_block;
}

void SIGParser::Rtx_raygen_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_block(this);
}

void SIGParser::Rtx_raygen_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_block(this);
}


std::any SIGParser::Rtx_raygen_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_raygen_blockContext* SIGParser::rtx_raygen_block() {
  Rtx_raygen_blockContext *_localctx = _tracker.createInstance<Rtx_raygen_blockContext>(_ctx, getState());
  enterRule(_localctx, 148, SIGParser::RuleRtx_raygen_block);
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
      ((1ULL << _la) & 134201344) != 0 || _la == SIGParser::OSBRACE

    || _la == SIGParser::COMMENT) {
      setState(699);
      rtx_raygen_stat();
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

//----------------- Rtx_raygen_definitionContext ------------------------------------------------------------------

SIGParser::Rtx_raygen_definitionContext::Rtx_raygen_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::RAYTRACE_RAYGEN() {
  return getToken(SIGParser::RAYTRACE_RAYGEN, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_raygen_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_raygen_blockContext* SIGParser::Rtx_raygen_definitionContext::rtx_raygen_block() {
  return getRuleContext<SIGParser::Rtx_raygen_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtx_raygen_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtx_raygen_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Rtx_raygen_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_raygen_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_definition;
}

void SIGParser::Rtx_raygen_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_definition(this);
}

void SIGParser::Rtx_raygen_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_definition(this);
}


std::any SIGParser::Rtx_raygen_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitRtx_raygen_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Rtx_raygen_definitionContext* SIGParser::rtx_raygen_definition() {
  Rtx_raygen_definitionContext *_localctx = _tracker.createInstance<Rtx_raygen_definitionContext>(_ctx, getState());
  enterRule(_localctx, 150, SIGParser::RuleRtx_raygen_definition);
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
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(705);
        option_block(); 
      }
      setState(710);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx);
    }
    setState(711);
    match(SIGParser::RAYTRACE_RAYGEN);
    setState(712);
    name_id();
    setState(714);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(713);
      inherit();
    }
    setState(716);
    match(SIGParser::OBRACE);
    setState(717);
    rtx_raygen_block();
    setState(718);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_declarationContext ------------------------------------------------------------------

SIGParser::View_declarationContext::View_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_idContext* SIGParser::View_declarationContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::View_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::View_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::View_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::View_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::View_declarationContext::getRuleIndex() const {
  return SIGParser::RuleView_declaration;
}

void SIGParser::View_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_declaration(this);
}

void SIGParser::View_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_declaration(this);
}


std::any SIGParser::View_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitView_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::View_declarationContext* SIGParser::view_declaration() {
  View_declarationContext *_localctx = _tracker.createInstance<View_declarationContext>(_ctx, getState());
  enterRule(_localctx, 152, SIGParser::RuleView_declaration);

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
    setState(723);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(720);
        option_block(); 
      }
      setState(725);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx);
    }
    setState(726);
    type_id();
    setState(727);
    name_id();
    setState(728);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- View_statContext ------------------------------------------------------------------

SIGParser::View_statContext::View_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::View_declarationContext* SIGParser::View_statContext::view_declaration() {
  return getRuleContext<SIGParser::View_declarationContext>(0);
}

tree::TerminalNode* SIGParser::View_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::View_statContext::getRuleIndex() const {
  return SIGParser::RuleView_stat;
}

void SIGParser::View_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_stat(this);
}

void SIGParser::View_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_stat(this);
}


std::any SIGParser::View_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitView_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::View_statContext* SIGParser::view_stat() {
  View_statContext *_localctx = _tracker.createInstance<View_statContext>(_ctx, getState());
  enterRule(_localctx, 154, SIGParser::RuleView_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(732);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(730);
        view_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(731);
        match(SIGParser::COMMENT);
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

SIGParser::View_blockContext::View_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::View_statContext *> SIGParser::View_blockContext::view_stat() {
  return getRuleContexts<SIGParser::View_statContext>();
}

SIGParser::View_statContext* SIGParser::View_blockContext::view_stat(size_t i) {
  return getRuleContext<SIGParser::View_statContext>(i);
}


size_t SIGParser::View_blockContext::getRuleIndex() const {
  return SIGParser::RuleView_block;
}

void SIGParser::View_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_block(this);
}

void SIGParser::View_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_block(this);
}


std::any SIGParser::View_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitView_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::View_blockContext* SIGParser::view_block() {
  View_blockContext *_localctx = _tracker.createInstance<View_blockContext>(_ctx, getState());
  enterRule(_localctx, 156, SIGParser::RuleView_block);
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
    setState(737);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(734);
      view_stat();
      setState(739);
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

SIGParser::View_definitionContext::View_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::View_definitionContext::VIEW() {
  return getToken(SIGParser::VIEW, 0);
}

SIGParser::Name_idContext* SIGParser::View_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::View_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::View_blockContext* SIGParser::View_definitionContext::view_block() {
  return getRuleContext<SIGParser::View_blockContext>(0);
}

tree::TerminalNode* SIGParser::View_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::View_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::View_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::View_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::View_definitionContext::getRuleIndex() const {
  return SIGParser::RuleView_definition;
}

void SIGParser::View_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterView_definition(this);
}

void SIGParser::View_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitView_definition(this);
}


std::any SIGParser::View_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitView_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::View_definitionContext* SIGParser::view_definition() {
  View_definitionContext *_localctx = _tracker.createInstance<View_definitionContext>(_ctx, getState());
  enterRule(_localctx, 158, SIGParser::RuleView_definition);
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
    setState(743);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(740);
        option_block(); 
      }
      setState(745);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
    }
    setState(746);
    match(SIGParser::VIEW);
    setState(747);
    name_id();
    setState(749);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(748);
      inherit();
    }
    setState(751);
    match(SIGParser::OBRACE);
    setState(752);
    view_block();
    setState(753);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pass_definitionContext ------------------------------------------------------------------

SIGParser::Pass_definitionContext::Pass_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Pass_definitionContext::PASS() {
  return getToken(SIGParser::PASS, 0);
}

SIGParser::Name_idContext* SIGParser::Pass_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Pass_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::View_blockContext* SIGParser::Pass_definitionContext::view_block() {
  return getRuleContext<SIGParser::View_blockContext>(0);
}

tree::TerminalNode* SIGParser::Pass_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Pass_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Pass_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Pass_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Pass_definitionContext::getRuleIndex() const {
  return SIGParser::RulePass_definition;
}

void SIGParser::Pass_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPass_definition(this);
}

void SIGParser::Pass_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPass_definition(this);
}


std::any SIGParser::Pass_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPass_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pass_definitionContext* SIGParser::pass_definition() {
  Pass_definitionContext *_localctx = _tracker.createInstance<Pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 160, SIGParser::RulePass_definition);
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
    setState(758);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(755);
        option_block(); 
      }
      setState(760);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
    }
    setState(761);
    match(SIGParser::PASS);
    setState(762);
    name_id();
    setState(764);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__7) {
      setState(763);
      inherit();
    }
    setState(766);
    match(SIGParser::OBRACE);
    setState(767);
    view_block();
    setState(768);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pipeline_statContext ------------------------------------------------------------------

SIGParser::Pipeline_statContext::Pipeline_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Pipeline_statContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Pipeline_statContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Pipeline_statContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Pipeline_statContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

tree::TerminalNode* SIGParser::Pipeline_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Pipeline_statContext::getRuleIndex() const {
  return SIGParser::RulePipeline_stat;
}

void SIGParser::Pipeline_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_stat(this);
}

void SIGParser::Pipeline_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_stat(this);
}


std::any SIGParser::Pipeline_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPipeline_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pipeline_statContext* SIGParser::pipeline_stat() {
  Pipeline_statContext *_localctx = _tracker.createInstance<Pipeline_statContext>(_ctx, getState());
  enterRule(_localctx, 162, SIGParser::RulePipeline_stat);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(780);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(773);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == SIGParser::OSBRACE) {
          setState(770);
          option_block();
          setState(775);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(776);
        name_id();
        setState(777);
        match(SIGParser::SCOL);
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(779);
        match(SIGParser::COMMENT);
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

SIGParser::Pipeline_blockContext::Pipeline_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Pipeline_statContext *> SIGParser::Pipeline_blockContext::pipeline_stat() {
  return getRuleContexts<SIGParser::Pipeline_statContext>();
}

SIGParser::Pipeline_statContext* SIGParser::Pipeline_blockContext::pipeline_stat(size_t i) {
  return getRuleContext<SIGParser::Pipeline_statContext>(i);
}


size_t SIGParser::Pipeline_blockContext::getRuleIndex() const {
  return SIGParser::RulePipeline_block;
}

void SIGParser::Pipeline_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_block(this);
}

void SIGParser::Pipeline_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_block(this);
}


std::any SIGParser::Pipeline_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPipeline_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pipeline_blockContext* SIGParser::pipeline_block() {
  Pipeline_blockContext *_localctx = _tracker.createInstance<Pipeline_blockContext>(_ctx, getState());
  enterRule(_localctx, 164, SIGParser::RulePipeline_block);
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
    setState(785);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & 553648129) != 0) {
      setState(782);
      pipeline_stat();
      setState(787);
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

SIGParser::Pipeline_definitionContext::Pipeline_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Pipeline_definitionContext::PIPELINE() {
  return getToken(SIGParser::PIPELINE, 0);
}

SIGParser::Name_idContext* SIGParser::Pipeline_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Pipeline_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Pipeline_blockContext* SIGParser::Pipeline_definitionContext::pipeline_block() {
  return getRuleContext<SIGParser::Pipeline_blockContext>(0);
}

tree::TerminalNode* SIGParser::Pipeline_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Pipeline_definitionContext::getRuleIndex() const {
  return SIGParser::RulePipeline_definition;
}

void SIGParser::Pipeline_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPipeline_definition(this);
}

void SIGParser::Pipeline_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPipeline_definition(this);
}


std::any SIGParser::Pipeline_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPipeline_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pipeline_definitionContext* SIGParser::pipeline_definition() {
  Pipeline_definitionContext *_localctx = _tracker.createInstance<Pipeline_definitionContext>(_ctx, getState());
  enterRule(_localctx, 166, SIGParser::RulePipeline_definition);

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
    match(SIGParser::PIPELINE);
    setState(789);
    name_id();
    setState(790);
    match(SIGParser::OBRACE);
    setState(791);
    pipeline_block();
    setState(792);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_value_declarationContext ------------------------------------------------------------------

SIGParser::Enum_value_declarationContext::Enum_value_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Enum_value_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Enum_value_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

tree::TerminalNode* SIGParser::Enum_value_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Enum_value_declarationContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Enum_value_declarationContext::getRuleIndex() const {
  return SIGParser::RuleEnum_value_declaration;
}

void SIGParser::Enum_value_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_value_declaration(this);
}

void SIGParser::Enum_value_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_value_declaration(this);
}


std::any SIGParser::Enum_value_declarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitEnum_value_declaration(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Enum_value_declarationContext* SIGParser::enum_value_declaration() {
  Enum_value_declarationContext *_localctx = _tracker.createInstance<Enum_value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 168, SIGParser::RuleEnum_value_declaration);
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
    setState(794);
    name_id();
    setState(797);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(795);
      match(SIGParser::ASSIGN);
      setState(796);
      value_id();
    }
    setState(799);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Enum_statContext ------------------------------------------------------------------

SIGParser::Enum_statContext::Enum_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Enum_value_declarationContext* SIGParser::Enum_statContext::enum_value_declaration() {
  return getRuleContext<SIGParser::Enum_value_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Enum_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Enum_statContext::getRuleIndex() const {
  return SIGParser::RuleEnum_stat;
}

void SIGParser::Enum_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_stat(this);
}

void SIGParser::Enum_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_stat(this);
}


std::any SIGParser::Enum_statContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitEnum_stat(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Enum_statContext* SIGParser::enum_stat() {
  Enum_statContext *_localctx = _tracker.createInstance<Enum_statContext>(_ctx, getState());
  enterRule(_localctx, 170, SIGParser::RuleEnum_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(803);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(801);
        enum_value_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(802);
        match(SIGParser::COMMENT);
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

SIGParser::Enum_blockContext::Enum_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Enum_statContext *> SIGParser::Enum_blockContext::enum_stat() {
  return getRuleContexts<SIGParser::Enum_statContext>();
}

SIGParser::Enum_statContext* SIGParser::Enum_blockContext::enum_stat(size_t i) {
  return getRuleContext<SIGParser::Enum_statContext>(i);
}


size_t SIGParser::Enum_blockContext::getRuleIndex() const {
  return SIGParser::RuleEnum_block;
}

void SIGParser::Enum_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_block(this);
}

void SIGParser::Enum_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_block(this);
}


std::any SIGParser::Enum_blockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitEnum_block(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Enum_blockContext* SIGParser::enum_block() {
  Enum_blockContext *_localctx = _tracker.createInstance<Enum_blockContext>(_ctx, getState());
  enterRule(_localctx, 172, SIGParser::RuleEnum_block);
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
    setState(808);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::ID

    || _la == SIGParser::COMMENT) {
      setState(805);
      enum_stat();
      setState(810);
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

SIGParser::Enum_definitionContext::Enum_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Enum_definitionContext::ENUM() {
  return getToken(SIGParser::ENUM, 0);
}

SIGParser::Name_idContext* SIGParser::Enum_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Enum_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Enum_blockContext* SIGParser::Enum_definitionContext::enum_block() {
  return getRuleContext<SIGParser::Enum_blockContext>(0);
}

tree::TerminalNode* SIGParser::Enum_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Enum_definitionContext::getRuleIndex() const {
  return SIGParser::RuleEnum_definition;
}

void SIGParser::Enum_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnum_definition(this);
}

void SIGParser::Enum_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnum_definition(this);
}


std::any SIGParser::Enum_definitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitEnum_definition(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Enum_definitionContext* SIGParser::enum_definition() {
  Enum_definitionContext *_localctx = _tracker.createInstance<Enum_definitionContext>(_ctx, getState());
  enterRule(_localctx, 174, SIGParser::RuleEnum_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(811);
    match(SIGParser::ENUM);
    setState(812);
    name_id();
    setState(813);
    match(SIGParser::OBRACE);
    setState(814);
    enum_block();
    setState(815);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Shader_typeContext ------------------------------------------------------------------

SIGParser::Shader_typeContext::Shader_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SIGParser::Shader_typeContext::getRuleIndex() const {
  return SIGParser::RuleShader_type;
}

void SIGParser::Shader_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader_type(this);
}

void SIGParser::Shader_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader_type(this);
}


std::any SIGParser::Shader_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitShader_type(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Shader_typeContext* SIGParser::shader_type() {
  Shader_typeContext *_localctx = _tracker.createInstance<Shader_typeContext>(_ctx, getState());
  enterRule(_localctx, 176, SIGParser::RuleShader_type);
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
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 134201344) != 0)) {
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

SIGParser::Pso_param_idContext::Pso_param_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SIGParser::Pso_param_idContext::getRuleIndex() const {
  return SIGParser::RulePso_param_id;
}

void SIGParser::Pso_param_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param_id(this);
}

void SIGParser::Pso_param_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param_id(this);
}


std::any SIGParser::Pso_param_idContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitPso_param_id(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Pso_param_idContext* SIGParser::pso_param_id() {
  Pso_param_idContext *_localctx = _tracker.createInstance<Pso_param_idContext>(_ctx, getState());
  enterRule(_localctx, 178, SIGParser::RulePso_param_id);
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
    setState(819);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 70368609959936) != 0)) {
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

SIGParser::Bool_typeContext::Bool_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Bool_typeContext::TRUE() {
  return getToken(SIGParser::TRUE, 0);
}

tree::TerminalNode* SIGParser::Bool_typeContext::FALSE() {
  return getToken(SIGParser::FALSE, 0);
}


size_t SIGParser::Bool_typeContext::getRuleIndex() const {
  return SIGParser::RuleBool_type;
}

void SIGParser::Bool_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBool_type(this);
}

void SIGParser::Bool_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBool_type(this);
}


std::any SIGParser::Bool_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SIGVisitor*>(visitor))
    return parserVisitor->visitBool_type(this);
  else
    return visitor->visitChildren(this);
}

SIGParser::Bool_typeContext* SIGParser::bool_type() {
  Bool_typeContext *_localctx = _tracker.createInstance<Bool_typeContext>(_ctx, getState());
  enterRule(_localctx, 180, SIGParser::RuleBool_type);
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
    setState(821);
    _la = _input->LA(1);
    if (!(_la == SIGParser::TRUE

    || _la == SIGParser::FALSE)) {
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

void SIGParser::initialize() {
  ::antlr4::internal::call_once(sigParserOnceFlag, sigParserInitialize);
}
