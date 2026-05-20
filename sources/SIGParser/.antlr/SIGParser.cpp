
// Generated from c:/Users/Bohdan/Documents/GitHub/Spectrum/sources/SIGParser/SIG.g4 by ANTLR 4.13.1


#include "SIGListener.h"

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
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
SIGParserStaticData *sigParserStaticData = nullptr;

void sigParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (sigParserStaticData != nullptr) {
    return;
  }
#else
  assert(sigParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SIGParserStaticData>(
    std::vector<std::string>{
      "parse", "bind_option", "options_assign", "option", "option_block", 
      "array_count_id", "array", "value_declaration", "slot_declaration", 
      "sampler_declaration", "define_declaration", "rtv_formats_declaration", 
      "blends_declaration", "pointer", "pso_param", "class_no_template", 
      "type_with_template", "inherit_id", "name_id", "option_id", "owner_id", 
      "template_id", "function_id", "value_id", "value_id_ignore", "type_id", 
      "insert_block", "path_id", "inherit", "layout_stat", "layout_block", 
      "layout_definition", "table_stat", "table_block", "table_definition", 
      "rt_color_declaration", "rt_ds_declaration", "rt_stat", "rt_block", 
      "rt_definition", "array_value_holder", "array_value_ids", "root_sig", 
      "shader", "compute_pso_stat", "compute_pso_block", "compute_pso_definition", 
      "graphics_pso_stat", "graphics_pso_block", "graphics_pso_definition", 
      "rtx_pso_stat", "rtx_pso_block", "rtx_pso_definition", "workgraph_pso_stat", 
      "workgraph_pso_block", "workgraph_pso_definition", "rtx_pass_stat", 
      "rtx_pass_block", "rtx_pass_definition", "rtx_raygen_stat", "rtx_raygen_block", 
      "rtx_raygen_definition", "view_declaration", "view_stat", "view_block", 
      "view_definition", "pass_definition", "pipeline_stat", "pipeline_block", 
      "pipeline_definition", "shader_type", "pso_param_id", "bool_type"
    },
    std::vector<std::string>{
      "", "'::'", "','", "'Sampler'", "'define'", "'rtv'", "'blend'", "':'", 
      "'compute'", "'vertex'", "'pixel'", "'domain'", "'hull'", "'geometry'", 
      "'miss'", "'closest_hit'", "'raygen'", "'amplification'", "'mesh'", 
      "'shader'", "'ds'", "'cull'", "'depth_func'", "'depth_write'", "'conservative'", 
      "'enable_depth'", "'topology'", "'enable_stencil'", "'stencil_func'", 
      "'stencil_pass_op'", "'stencil_read_mask'", "'stencil_write_mask'", 
      "'recursion_depth'", "'payload'", "'per_material'", "'local'", "'||'", 
      "'&&'", "'=='", "'!='", "'>'", "'<'", "'>='", "'<='", "'+'", "'-'", 
      "'/'", "'%'", "'^'", "'!'", "';'", "'='", "'('", "')'", "'{'", "'}'", 
      "'['", "']'", "'true'", "'false'", "'log'", "'layout'", "'struct'", 
      "'ComputePSO'", "'GraphicsPSO'", "'RaytracePSO'", "'WorkgraphPSO'", 
      "'RaytraceRaygen'", "'RaytracePass'", "'PassNode'", "'PassView'", 
      "'Pipeline'", "'slot'", "'rt'", "'RTV'", "'DSV'", "'root'", "", "", 
      "", "", "", "", "'*'", "'%{'", "'}%'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "OR", "AND", "EQ", "NEQ", "GT", "LT", "GTEQ", "LTEQ", "PLUS", 
      "MINUS", "DIV", "MOD", "POW", "NOT", "SCOL", "ASSIGN", "OPAR", "CPAR", 
      "OBRACE", "CBRACE", "OSBRACE", "CSBRACE", "TRUE", "FALSE", "LOG", 
      "LAYOUT", "STRUCT", "COMPUTE_PSO", "GRAPHICS_PSO", "RAYTRACE_PSO", 
      "WORKGRAPH_PSO", "RAYTRACE_RAYGEN", "RAYTRACE_PASS", "PASS", "VIEW", 
      "PIPELINE", "SLOT", "RT", "RTV", "DSV", "ROOTSIG", "ID", "INT_SCALAR", 
      "FLOAT_SCALAR", "STRING", "COMMENT", "SPACE", "POINTER", "INSERT_START", 
      "INSERT_END", "INSERT_BLOCK"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,86,669,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,7,
  	63,2,64,7,64,2,65,7,65,2,66,7,66,2,67,7,67,2,68,7,68,2,69,7,69,2,70,7,
  	70,2,71,7,71,2,72,7,72,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
  	0,1,0,5,0,160,8,0,10,0,12,0,163,9,0,1,0,1,0,1,1,1,1,1,1,3,1,170,8,1,1,
  	1,1,1,1,2,1,2,1,2,1,3,1,3,3,3,179,8,3,1,4,1,4,1,4,1,4,5,4,185,8,4,10,
  	4,12,4,188,9,4,1,4,1,4,1,5,1,5,1,6,1,6,3,6,196,8,6,1,6,1,6,1,7,5,7,201,
  	8,7,10,7,12,7,204,9,7,1,7,1,7,1,7,3,7,209,8,7,1,7,1,7,3,7,213,8,7,1,7,
  	1,7,1,8,1,8,1,8,1,8,1,9,1,9,1,9,1,9,1,9,1,9,1,10,5,10,228,8,10,10,10,
  	12,10,231,9,10,1,10,1,10,1,10,1,10,3,10,237,8,10,1,10,1,10,1,11,5,11,
  	242,8,11,10,11,12,11,245,9,11,1,11,1,11,1,11,1,11,1,11,1,12,5,12,253,
  	8,12,10,12,12,12,256,9,12,1,12,1,12,1,12,1,12,1,12,1,13,1,13,1,14,5,14,
  	266,8,14,10,14,12,14,269,9,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,1,16,
  	1,16,1,16,5,16,281,8,16,10,16,12,16,284,9,16,1,16,3,16,287,8,16,1,16,
  	3,16,290,8,16,1,17,1,17,1,18,1,18,1,19,1,19,1,20,1,20,1,21,1,21,1,22,
  	1,22,1,22,3,22,305,8,22,1,22,1,22,5,22,309,8,22,10,22,12,22,312,9,22,
  	1,22,1,22,1,23,1,23,1,23,1,23,1,23,1,23,1,23,3,23,323,8,23,1,24,1,24,
  	1,24,1,24,3,24,329,8,24,1,25,1,25,1,26,1,26,1,27,1,27,5,27,337,8,27,10,
  	27,12,27,340,9,27,1,27,1,27,1,28,1,28,1,28,1,28,5,28,348,8,28,10,28,12,
  	28,351,9,28,1,29,1,29,1,29,3,29,356,8,29,1,30,5,30,359,8,30,10,30,12,
  	30,362,9,30,1,31,1,31,1,31,3,31,367,8,31,1,31,1,31,1,31,1,31,1,32,1,32,
  	1,32,3,32,376,8,32,1,33,5,33,379,8,33,10,33,12,33,382,9,33,1,34,5,34,
  	385,8,34,10,34,12,34,388,9,34,1,34,1,34,1,34,3,34,393,8,34,1,34,1,34,
  	1,34,1,34,1,35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,37,1,37,1,37,3,37,
  	410,8,37,1,38,5,38,413,8,38,10,38,12,38,416,9,38,1,39,1,39,1,39,1,39,
  	1,39,1,39,1,40,1,40,1,41,1,41,1,41,1,41,5,41,430,8,41,10,41,12,41,433,
  	9,41,1,41,1,41,1,42,1,42,1,42,1,42,1,42,1,43,5,43,443,8,43,10,43,12,43,
  	446,9,43,1,43,1,43,1,43,1,43,1,43,1,44,1,44,1,44,1,44,3,44,457,8,44,1,
  	45,5,45,460,8,45,10,45,12,45,463,9,45,1,46,1,46,1,46,3,46,468,8,46,1,
  	46,1,46,1,46,1,46,1,47,1,47,1,47,1,47,1,47,1,47,1,47,3,47,481,8,47,1,
  	48,5,48,484,8,48,10,48,12,48,487,9,48,1,49,5,49,490,8,49,10,49,12,49,
  	493,9,49,1,49,1,49,1,49,3,49,498,8,49,1,49,1,49,1,49,1,49,1,50,1,50,3,
  	50,506,8,50,1,51,5,51,509,8,51,10,51,12,51,512,9,51,1,52,1,52,1,52,3,
  	52,517,8,52,1,52,1,52,1,52,1,52,1,53,1,53,1,53,1,53,3,53,527,8,53,1,54,
  	5,54,530,8,54,10,54,12,54,533,9,54,1,55,1,55,1,55,3,55,538,8,55,1,55,
  	1,55,1,55,1,55,1,56,1,56,1,56,3,56,547,8,56,1,57,5,57,550,8,57,10,57,
  	12,57,553,9,57,1,58,5,58,556,8,58,10,58,12,58,559,9,58,1,58,1,58,1,58,
  	3,58,564,8,58,1,58,1,58,1,58,1,58,1,59,1,59,3,59,572,8,59,1,60,5,60,575,
  	8,60,10,60,12,60,578,9,60,1,61,5,61,581,8,61,10,61,12,61,584,9,61,1,61,
  	1,61,1,61,3,61,589,8,61,1,61,1,61,1,61,1,61,1,62,5,62,596,8,62,10,62,
  	12,62,599,9,62,1,62,1,62,1,62,1,62,1,63,1,63,3,63,607,8,63,1,64,5,64,
  	610,8,64,10,64,12,64,613,9,64,1,65,5,65,616,8,65,10,65,12,65,619,9,65,
  	1,65,1,65,1,65,3,65,624,8,65,1,65,1,65,1,65,1,65,1,66,5,66,631,8,66,10,
  	66,12,66,634,9,66,1,66,1,66,1,66,3,66,639,8,66,1,66,1,66,1,66,1,66,1,
  	67,1,67,1,67,1,67,3,67,649,8,67,1,68,5,68,652,8,68,10,68,12,68,655,9,
  	68,1,69,1,69,1,69,1,69,1,69,1,69,1,70,1,70,1,71,1,71,1,72,1,72,1,72,15,
  	202,229,243,254,267,338,349,386,444,491,557,582,597,617,632,0,73,0,2,
  	4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,
  	52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,
  	98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,
  	134,136,138,140,142,144,0,3,1,0,8,19,1,0,20,35,1,0,58,59,690,0,161,1,
  	0,0,0,2,169,1,0,0,0,4,173,1,0,0,0,6,176,1,0,0,0,8,180,1,0,0,0,10,191,
  	1,0,0,0,12,193,1,0,0,0,14,202,1,0,0,0,16,216,1,0,0,0,18,220,1,0,0,0,20,
  	229,1,0,0,0,22,243,1,0,0,0,24,254,1,0,0,0,26,262,1,0,0,0,28,267,1,0,0,
  	0,30,275,1,0,0,0,32,277,1,0,0,0,34,291,1,0,0,0,36,293,1,0,0,0,38,295,
  	1,0,0,0,40,297,1,0,0,0,42,299,1,0,0,0,44,301,1,0,0,0,46,322,1,0,0,0,48,
  	328,1,0,0,0,50,330,1,0,0,0,52,332,1,0,0,0,54,338,1,0,0,0,56,343,1,0,0,
  	0,58,355,1,0,0,0,60,360,1,0,0,0,62,363,1,0,0,0,64,375,1,0,0,0,66,380,
  	1,0,0,0,68,386,1,0,0,0,70,398,1,0,0,0,72,402,1,0,0,0,74,409,1,0,0,0,76,
  	414,1,0,0,0,78,417,1,0,0,0,80,423,1,0,0,0,82,425,1,0,0,0,84,436,1,0,0,
  	0,86,444,1,0,0,0,88,456,1,0,0,0,90,461,1,0,0,0,92,464,1,0,0,0,94,480,
  	1,0,0,0,96,485,1,0,0,0,98,491,1,0,0,0,100,505,1,0,0,0,102,510,1,0,0,0,
  	104,513,1,0,0,0,106,526,1,0,0,0,108,531,1,0,0,0,110,534,1,0,0,0,112,546,
  	1,0,0,0,114,551,1,0,0,0,116,557,1,0,0,0,118,571,1,0,0,0,120,576,1,0,0,
  	0,122,582,1,0,0,0,124,597,1,0,0,0,126,606,1,0,0,0,128,611,1,0,0,0,130,
  	617,1,0,0,0,132,632,1,0,0,0,134,648,1,0,0,0,136,653,1,0,0,0,138,656,1,
  	0,0,0,140,662,1,0,0,0,142,664,1,0,0,0,144,666,1,0,0,0,146,160,3,62,31,
  	0,147,160,3,68,34,0,148,160,3,78,39,0,149,160,3,110,55,0,150,160,3,92,
  	46,0,151,160,3,98,49,0,152,160,3,104,52,0,153,160,3,116,58,0,154,160,
  	3,122,61,0,155,160,3,132,66,0,156,160,3,130,65,0,157,160,3,138,69,0,158,
  	160,5,81,0,0,159,146,1,0,0,0,159,147,1,0,0,0,159,148,1,0,0,0,159,149,
  	1,0,0,0,159,150,1,0,0,0,159,151,1,0,0,0,159,152,1,0,0,0,159,153,1,0,0,
  	0,159,154,1,0,0,0,159,155,1,0,0,0,159,156,1,0,0,0,159,157,1,0,0,0,159,
  	158,1,0,0,0,160,163,1,0,0,0,161,159,1,0,0,0,161,162,1,0,0,0,162,164,1,
  	0,0,0,163,161,1,0,0,0,164,165,5,0,0,1,165,1,1,0,0,0,166,167,3,40,20,0,
  	167,168,5,1,0,0,168,170,1,0,0,0,169,166,1,0,0,0,169,170,1,0,0,0,170,171,
  	1,0,0,0,171,172,3,46,23,0,172,3,1,0,0,0,173,174,5,51,0,0,174,175,3,2,
  	1,0,175,5,1,0,0,0,176,178,3,36,18,0,177,179,3,4,2,0,178,177,1,0,0,0,178,
  	179,1,0,0,0,179,7,1,0,0,0,180,181,5,56,0,0,181,186,3,6,3,0,182,183,5,
  	2,0,0,183,185,3,6,3,0,184,182,1,0,0,0,185,188,1,0,0,0,186,184,1,0,0,0,
  	186,187,1,0,0,0,187,189,1,0,0,0,188,186,1,0,0,0,189,190,5,57,0,0,190,
  	9,1,0,0,0,191,192,5,78,0,0,192,11,1,0,0,0,193,195,5,56,0,0,194,196,3,
  	10,5,0,195,194,1,0,0,0,195,196,1,0,0,0,196,197,1,0,0,0,197,198,5,57,0,
  	0,198,13,1,0,0,0,199,201,3,8,4,0,200,199,1,0,0,0,201,204,1,0,0,0,202,
  	203,1,0,0,0,202,200,1,0,0,0,203,205,1,0,0,0,204,202,1,0,0,0,205,206,3,
  	50,25,0,206,208,3,36,18,0,207,209,3,12,6,0,208,207,1,0,0,0,208,209,1,
  	0,0,0,209,212,1,0,0,0,210,211,5,51,0,0,211,213,3,46,23,0,212,210,1,0,
  	0,0,212,213,1,0,0,0,213,214,1,0,0,0,214,215,5,50,0,0,215,15,1,0,0,0,216,
  	217,5,72,0,0,217,218,3,36,18,0,218,219,5,50,0,0,219,17,1,0,0,0,220,221,
  	5,3,0,0,221,222,3,36,18,0,222,223,5,51,0,0,223,224,3,46,23,0,224,225,
  	5,50,0,0,225,19,1,0,0,0,226,228,3,8,4,0,227,226,1,0,0,0,228,231,1,0,0,
  	0,229,230,1,0,0,0,229,227,1,0,0,0,230,232,1,0,0,0,231,229,1,0,0,0,232,
  	233,5,4,0,0,233,236,3,36,18,0,234,235,5,51,0,0,235,237,3,82,41,0,236,
  	234,1,0,0,0,236,237,1,0,0,0,237,238,1,0,0,0,238,239,5,50,0,0,239,21,1,
  	0,0,0,240,242,3,8,4,0,241,240,1,0,0,0,242,245,1,0,0,0,243,244,1,0,0,0,
  	243,241,1,0,0,0,244,246,1,0,0,0,245,243,1,0,0,0,246,247,5,5,0,0,247,248,
  	5,51,0,0,248,249,3,82,41,0,249,250,5,50,0,0,250,23,1,0,0,0,251,253,3,
  	8,4,0,252,251,1,0,0,0,253,256,1,0,0,0,254,255,1,0,0,0,254,252,1,0,0,0,
  	255,257,1,0,0,0,256,254,1,0,0,0,257,258,5,6,0,0,258,259,5,51,0,0,259,
  	260,3,82,41,0,260,261,5,50,0,0,261,25,1,0,0,0,262,263,5,83,0,0,263,27,
  	1,0,0,0,264,266,3,8,4,0,265,264,1,0,0,0,266,269,1,0,0,0,267,268,1,0,0,
  	0,267,265,1,0,0,0,268,270,1,0,0,0,269,267,1,0,0,0,270,271,3,142,71,0,
  	271,272,5,51,0,0,272,273,3,46,23,0,273,274,5,50,0,0,274,29,1,0,0,0,275,
  	276,5,77,0,0,276,31,1,0,0,0,277,286,3,30,15,0,278,282,5,41,0,0,279,281,
  	3,42,21,0,280,279,1,0,0,0,281,284,1,0,0,0,282,280,1,0,0,0,282,283,1,0,
  	0,0,283,285,1,0,0,0,284,282,1,0,0,0,285,287,5,40,0,0,286,278,1,0,0,0,
  	286,287,1,0,0,0,287,289,1,0,0,0,288,290,3,26,13,0,289,288,1,0,0,0,289,
  	290,1,0,0,0,290,33,1,0,0,0,291,292,5,77,0,0,292,35,1,0,0,0,293,294,5,
  	77,0,0,294,37,1,0,0,0,295,296,5,77,0,0,296,39,1,0,0,0,297,298,5,77,0,
  	0,298,41,1,0,0,0,299,300,5,77,0,0,300,43,1,0,0,0,301,302,5,77,0,0,302,
  	304,5,52,0,0,303,305,3,48,24,0,304,303,1,0,0,0,304,305,1,0,0,0,305,310,
  	1,0,0,0,306,307,5,2,0,0,307,309,3,48,24,0,308,306,1,0,0,0,309,312,1,0,
  	0,0,310,308,1,0,0,0,310,311,1,0,0,0,311,313,1,0,0,0,312,310,1,0,0,0,313,
  	314,5,53,0,0,314,45,1,0,0,0,315,323,3,140,70,0,316,323,5,77,0,0,317,323,
  	5,78,0,0,318,323,5,79,0,0,319,323,3,144,72,0,320,323,3,44,22,0,321,323,
  	3,82,41,0,322,315,1,0,0,0,322,316,1,0,0,0,322,317,1,0,0,0,322,318,1,0,
  	0,0,322,319,1,0,0,0,322,320,1,0,0,0,322,321,1,0,0,0,323,47,1,0,0,0,324,
  	329,5,77,0,0,325,329,5,78,0,0,326,329,5,79,0,0,327,329,3,144,72,0,328,
  	324,1,0,0,0,328,325,1,0,0,0,328,326,1,0,0,0,328,327,1,0,0,0,329,49,1,
  	0,0,0,330,331,3,32,16,0,331,51,1,0,0,0,332,333,5,86,0,0,333,53,1,0,0,
  	0,334,335,5,77,0,0,335,337,5,46,0,0,336,334,1,0,0,0,337,340,1,0,0,0,338,
  	339,1,0,0,0,338,336,1,0,0,0,339,341,1,0,0,0,340,338,1,0,0,0,341,342,5,
  	77,0,0,342,55,1,0,0,0,343,344,5,7,0,0,344,349,3,34,17,0,345,346,5,2,0,
  	0,346,348,3,34,17,0,347,345,1,0,0,0,348,351,1,0,0,0,349,350,1,0,0,0,349,
  	347,1,0,0,0,350,57,1,0,0,0,351,349,1,0,0,0,352,356,3,16,8,0,353,356,3,
  	18,9,0,354,356,5,81,0,0,355,352,1,0,0,0,355,353,1,0,0,0,355,354,1,0,0,
  	0,356,59,1,0,0,0,357,359,3,58,29,0,358,357,1,0,0,0,359,362,1,0,0,0,360,
  	358,1,0,0,0,360,361,1,0,0,0,361,61,1,0,0,0,362,360,1,0,0,0,363,364,5,
  	61,0,0,364,366,3,36,18,0,365,367,3,56,28,0,366,365,1,0,0,0,366,367,1,
  	0,0,0,367,368,1,0,0,0,368,369,5,54,0,0,369,370,3,60,30,0,370,371,5,55,
  	0,0,371,63,1,0,0,0,372,376,3,14,7,0,373,376,3,52,26,0,374,376,5,81,0,
  	0,375,372,1,0,0,0,375,373,1,0,0,0,375,374,1,0,0,0,376,65,1,0,0,0,377,
  	379,3,64,32,0,378,377,1,0,0,0,379,382,1,0,0,0,380,378,1,0,0,0,380,381,
  	1,0,0,0,381,67,1,0,0,0,382,380,1,0,0,0,383,385,3,8,4,0,384,383,1,0,0,
  	0,385,388,1,0,0,0,386,387,1,0,0,0,386,384,1,0,0,0,387,389,1,0,0,0,388,
  	386,1,0,0,0,389,390,5,62,0,0,390,392,3,36,18,0,391,393,3,56,28,0,392,
  	391,1,0,0,0,392,393,1,0,0,0,393,394,1,0,0,0,394,395,5,54,0,0,395,396,
  	3,66,33,0,396,397,5,55,0,0,397,69,1,0,0,0,398,399,3,50,25,0,399,400,3,
  	36,18,0,400,401,5,50,0,0,401,71,1,0,0,0,402,403,5,75,0,0,403,404,3,36,
  	18,0,404,405,5,50,0,0,405,73,1,0,0,0,406,410,3,70,35,0,407,410,3,72,36,
  	0,408,410,5,81,0,0,409,406,1,0,0,0,409,407,1,0,0,0,409,408,1,0,0,0,410,
  	75,1,0,0,0,411,413,3,74,37,0,412,411,1,0,0,0,413,416,1,0,0,0,414,412,
  	1,0,0,0,414,415,1,0,0,0,415,77,1,0,0,0,416,414,1,0,0,0,417,418,5,73,0,
  	0,418,419,3,36,18,0,419,420,5,54,0,0,420,421,3,76,38,0,421,422,5,55,0,
  	0,422,79,1,0,0,0,423,424,3,46,23,0,424,81,1,0,0,0,425,426,5,54,0,0,426,
  	431,3,80,40,0,427,428,5,2,0,0,428,430,3,80,40,0,429,427,1,0,0,0,430,433,
  	1,0,0,0,431,429,1,0,0,0,431,432,1,0,0,0,432,434,1,0,0,0,433,431,1,0,0,
  	0,434,435,5,55,0,0,435,83,1,0,0,0,436,437,5,76,0,0,437,438,5,51,0,0,438,
  	439,3,36,18,0,439,440,5,50,0,0,440,85,1,0,0,0,441,443,3,8,4,0,442,441,
  	1,0,0,0,443,446,1,0,0,0,444,445,1,0,0,0,444,442,1,0,0,0,445,447,1,0,0,
  	0,446,444,1,0,0,0,447,448,3,140,70,0,448,449,5,51,0,0,449,450,3,54,27,
  	0,450,451,5,50,0,0,451,87,1,0,0,0,452,457,3,84,42,0,453,457,3,86,43,0,
  	454,457,3,20,10,0,455,457,5,81,0,0,456,452,1,0,0,0,456,453,1,0,0,0,456,
  	454,1,0,0,0,456,455,1,0,0,0,457,89,1,0,0,0,458,460,3,88,44,0,459,458,
  	1,0,0,0,460,463,1,0,0,0,461,459,1,0,0,0,461,462,1,0,0,0,462,91,1,0,0,
  	0,463,461,1,0,0,0,464,465,5,63,0,0,465,467,3,36,18,0,466,468,3,56,28,
  	0,467,466,1,0,0,0,467,468,1,0,0,0,468,469,1,0,0,0,469,470,5,54,0,0,470,
  	471,3,90,45,0,471,472,5,55,0,0,472,93,1,0,0,0,473,481,3,84,42,0,474,481,
  	3,86,43,0,475,481,3,20,10,0,476,481,3,22,11,0,477,481,3,24,12,0,478,481,
  	3,28,14,0,479,481,5,81,0,0,480,473,1,0,0,0,480,474,1,0,0,0,480,475,1,
  	0,0,0,480,476,1,0,0,0,480,477,1,0,0,0,480,478,1,0,0,0,480,479,1,0,0,0,
  	481,95,1,0,0,0,482,484,3,94,47,0,483,482,1,0,0,0,484,487,1,0,0,0,485,
  	483,1,0,0,0,485,486,1,0,0,0,486,97,1,0,0,0,487,485,1,0,0,0,488,490,3,
  	8,4,0,489,488,1,0,0,0,490,493,1,0,0,0,491,492,1,0,0,0,491,489,1,0,0,0,
  	492,494,1,0,0,0,493,491,1,0,0,0,494,495,5,64,0,0,495,497,3,36,18,0,496,
  	498,3,56,28,0,497,496,1,0,0,0,497,498,1,0,0,0,498,499,1,0,0,0,499,500,
  	5,54,0,0,500,501,3,96,48,0,501,502,5,55,0,0,502,99,1,0,0,0,503,506,3,
  	84,42,0,504,506,5,81,0,0,505,503,1,0,0,0,505,504,1,0,0,0,506,101,1,0,
  	0,0,507,509,3,100,50,0,508,507,1,0,0,0,509,512,1,0,0,0,510,508,1,0,0,
  	0,510,511,1,0,0,0,511,103,1,0,0,0,512,510,1,0,0,0,513,514,5,65,0,0,514,
  	516,3,36,18,0,515,517,3,56,28,0,516,515,1,0,0,0,516,517,1,0,0,0,517,518,
  	1,0,0,0,518,519,5,54,0,0,519,520,3,102,51,0,520,521,5,55,0,0,521,105,
  	1,0,0,0,522,527,3,84,42,0,523,527,3,86,43,0,524,527,3,20,10,0,525,527,
  	5,81,0,0,526,522,1,0,0,0,526,523,1,0,0,0,526,524,1,0,0,0,526,525,1,0,
  	0,0,527,107,1,0,0,0,528,530,3,106,53,0,529,528,1,0,0,0,530,533,1,0,0,
  	0,531,529,1,0,0,0,531,532,1,0,0,0,532,109,1,0,0,0,533,531,1,0,0,0,534,
  	535,5,66,0,0,535,537,3,36,18,0,536,538,3,56,28,0,537,536,1,0,0,0,537,
  	538,1,0,0,0,538,539,1,0,0,0,539,540,5,54,0,0,540,541,3,108,54,0,541,542,
  	5,55,0,0,542,111,1,0,0,0,543,547,3,86,43,0,544,547,5,81,0,0,545,547,3,
  	28,14,0,546,543,1,0,0,0,546,544,1,0,0,0,546,545,1,0,0,0,547,113,1,0,0,
  	0,548,550,3,112,56,0,549,548,1,0,0,0,550,553,1,0,0,0,551,549,1,0,0,0,
  	551,552,1,0,0,0,552,115,1,0,0,0,553,551,1,0,0,0,554,556,3,8,4,0,555,554,
  	1,0,0,0,556,559,1,0,0,0,557,558,1,0,0,0,557,555,1,0,0,0,558,560,1,0,0,
  	0,559,557,1,0,0,0,560,561,5,68,0,0,561,563,3,36,18,0,562,564,3,56,28,
  	0,563,562,1,0,0,0,563,564,1,0,0,0,564,565,1,0,0,0,565,566,5,54,0,0,566,
  	567,3,114,57,0,567,568,5,55,0,0,568,117,1,0,0,0,569,572,3,86,43,0,570,
  	572,5,81,0,0,571,569,1,0,0,0,571,570,1,0,0,0,572,119,1,0,0,0,573,575,
  	3,118,59,0,574,573,1,0,0,0,575,578,1,0,0,0,576,574,1,0,0,0,576,577,1,
  	0,0,0,577,121,1,0,0,0,578,576,1,0,0,0,579,581,3,8,4,0,580,579,1,0,0,0,
  	581,584,1,0,0,0,582,583,1,0,0,0,582,580,1,0,0,0,583,585,1,0,0,0,584,582,
  	1,0,0,0,585,586,5,67,0,0,586,588,3,36,18,0,587,589,3,56,28,0,588,587,
  	1,0,0,0,588,589,1,0,0,0,589,590,1,0,0,0,590,591,5,54,0,0,591,592,3,120,
  	60,0,592,593,5,55,0,0,593,123,1,0,0,0,594,596,3,8,4,0,595,594,1,0,0,0,
  	596,599,1,0,0,0,597,598,1,0,0,0,597,595,1,0,0,0,598,600,1,0,0,0,599,597,
  	1,0,0,0,600,601,3,50,25,0,601,602,3,36,18,0,602,603,5,50,0,0,603,125,
  	1,0,0,0,604,607,3,124,62,0,605,607,5,81,0,0,606,604,1,0,0,0,606,605,1,
  	0,0,0,607,127,1,0,0,0,608,610,3,126,63,0,609,608,1,0,0,0,610,613,1,0,
  	0,0,611,609,1,0,0,0,611,612,1,0,0,0,612,129,1,0,0,0,613,611,1,0,0,0,614,
  	616,3,8,4,0,615,614,1,0,0,0,616,619,1,0,0,0,617,618,1,0,0,0,617,615,1,
  	0,0,0,618,620,1,0,0,0,619,617,1,0,0,0,620,621,5,70,0,0,621,623,3,36,18,
  	0,622,624,3,56,28,0,623,622,1,0,0,0,623,624,1,0,0,0,624,625,1,0,0,0,625,
  	626,5,54,0,0,626,627,3,128,64,0,627,628,5,55,0,0,628,131,1,0,0,0,629,
  	631,3,8,4,0,630,629,1,0,0,0,631,634,1,0,0,0,632,633,1,0,0,0,632,630,1,
  	0,0,0,633,635,1,0,0,0,634,632,1,0,0,0,635,636,5,69,0,0,636,638,3,36,18,
  	0,637,639,3,56,28,0,638,637,1,0,0,0,638,639,1,0,0,0,639,640,1,0,0,0,640,
  	641,5,54,0,0,641,642,3,128,64,0,642,643,5,55,0,0,643,133,1,0,0,0,644,
  	645,3,36,18,0,645,646,5,50,0,0,646,649,1,0,0,0,647,649,5,81,0,0,648,644,
  	1,0,0,0,648,647,1,0,0,0,649,135,1,0,0,0,650,652,3,134,67,0,651,650,1,
  	0,0,0,652,655,1,0,0,0,653,651,1,0,0,0,653,654,1,0,0,0,654,137,1,0,0,0,
  	655,653,1,0,0,0,656,657,5,71,0,0,657,658,3,36,18,0,658,659,5,54,0,0,659,
  	660,3,136,68,0,660,661,5,55,0,0,661,139,1,0,0,0,662,663,7,0,0,0,663,141,
  	1,0,0,0,664,665,7,1,0,0,665,143,1,0,0,0,666,667,7,2,0,0,667,145,1,0,0,
  	0,64,159,161,169,178,186,195,202,208,212,229,236,243,254,267,282,286,
  	289,304,310,322,328,338,349,355,360,366,375,380,386,392,409,414,431,444,
  	456,461,467,480,485,491,497,505,510,516,526,531,537,546,551,557,563,571,
  	576,582,588,597,606,611,617,623,632,638,648,653
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
    setState(161);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & 33751009) != 0)) {
      setState(159);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(146);
        layout_definition();
        break;
      }

      case 2: {
        setState(147);
        table_definition();
        break;
      }

      case 3: {
        setState(148);
        rt_definition();
        break;
      }

      case 4: {
        setState(149);
        workgraph_pso_definition();
        break;
      }

      case 5: {
        setState(150);
        compute_pso_definition();
        break;
      }

      case 6: {
        setState(151);
        graphics_pso_definition();
        break;
      }

      case 7: {
        setState(152);
        rtx_pso_definition();
        break;
      }

      case 8: {
        setState(153);
        rtx_pass_definition();
        break;
      }

      case 9: {
        setState(154);
        rtx_raygen_definition();
        break;
      }

      case 10: {
        setState(155);
        pass_definition();
        break;
      }

      case 11: {
        setState(156);
        view_definition();
        break;
      }

      case 12: {
        setState(157);
        pipeline_definition();
        break;
      }

      case 13: {
        setState(158);
        match(SIGParser::COMMENT);
        break;
      }

      default:
        break;
      }
      setState(163);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(164);
    match(SIGParser::EOF);
   
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

SIGParser::Value_idContext* SIGParser::Bind_optionContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

SIGParser::Owner_idContext* SIGParser::Bind_optionContext::owner_id() {
  return getRuleContext<SIGParser::Owner_idContext>(0);
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

SIGParser::Bind_optionContext* SIGParser::bind_option() {
  Bind_optionContext *_localctx = _tracker.createInstance<Bind_optionContext>(_ctx, getState());
  enterRule(_localctx, 2, SIGParser::RuleBind_option);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(169);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      setState(166);
      owner_id();
      setState(167);
      match(SIGParser::T__0);
      break;
    }

    default:
      break;
    }
    setState(171);
    value_id();
   
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

SIGParser::Options_assignContext* SIGParser::options_assign() {
  Options_assignContext *_localctx = _tracker.createInstance<Options_assignContext>(_ctx, getState());
  enterRule(_localctx, 4, SIGParser::RuleOptions_assign);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(173);
    match(SIGParser::ASSIGN);
    setState(174);
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

SIGParser::OptionContext* SIGParser::option() {
  OptionContext *_localctx = _tracker.createInstance<OptionContext>(_ctx, getState());
  enterRule(_localctx, 6, SIGParser::RuleOption);
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
    setState(176);
    name_id();
    setState(178);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(177);
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

SIGParser::Option_blockContext* SIGParser::option_block() {
  Option_blockContext *_localctx = _tracker.createInstance<Option_blockContext>(_ctx, getState());
  enterRule(_localctx, 8, SIGParser::RuleOption_block);
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
    setState(180);
    match(SIGParser::OSBRACE);
    setState(181);
    option();
    setState(186);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__1) {
      setState(182);
      match(SIGParser::T__1);
      setState(183);
      option();
      setState(188);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(189);
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

SIGParser::Array_count_idContext* SIGParser::array_count_id() {
  Array_count_idContext *_localctx = _tracker.createInstance<Array_count_idContext>(_ctx, getState());
  enterRule(_localctx, 10, SIGParser::RuleArray_count_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(191);
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

SIGParser::ArrayContext* SIGParser::array() {
  ArrayContext *_localctx = _tracker.createInstance<ArrayContext>(_ctx, getState());
  enterRule(_localctx, 12, SIGParser::RuleArray);
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
    setState(193);
    match(SIGParser::OSBRACE);
    setState(195);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::INT_SCALAR) {
      setState(194);
      array_count_id();
    }
    setState(197);
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

SIGParser::Value_declarationContext* SIGParser::value_declaration() {
  Value_declarationContext *_localctx = _tracker.createInstance<Value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 14, SIGParser::RuleValue_declaration);
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
    setState(202);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(199);
        option_block(); 
      }
      setState(204);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    }
    setState(205);
    type_id();
    setState(206);
    name_id();
    setState(208);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::OSBRACE) {
      setState(207);
      array();
    }
    setState(212);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(210);
      match(SIGParser::ASSIGN);
      setState(211);
      value_id();
    }
    setState(214);
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

SIGParser::Slot_declarationContext* SIGParser::slot_declaration() {
  Slot_declarationContext *_localctx = _tracker.createInstance<Slot_declarationContext>(_ctx, getState());
  enterRule(_localctx, 16, SIGParser::RuleSlot_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(216);
    match(SIGParser::SLOT);
    setState(217);
    name_id();
    setState(218);
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

SIGParser::Sampler_declarationContext* SIGParser::sampler_declaration() {
  Sampler_declarationContext *_localctx = _tracker.createInstance<Sampler_declarationContext>(_ctx, getState());
  enterRule(_localctx, 18, SIGParser::RuleSampler_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(220);
    match(SIGParser::T__2);
    setState(221);
    name_id();
    setState(222);
    match(SIGParser::ASSIGN);
    setState(223);
    value_id();
    setState(224);
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

SIGParser::Define_declarationContext* SIGParser::define_declaration() {
  Define_declarationContext *_localctx = _tracker.createInstance<Define_declarationContext>(_ctx, getState());
  enterRule(_localctx, 20, SIGParser::RuleDefine_declaration);
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
    setState(229);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(226);
        option_block(); 
      }
      setState(231);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    }
    setState(232);
    match(SIGParser::T__3);
    setState(233);
    name_id();
    setState(236);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(234);
      match(SIGParser::ASSIGN);
      setState(235);
      array_value_ids();
    }
    setState(238);
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

SIGParser::Rtv_formats_declarationContext* SIGParser::rtv_formats_declaration() {
  Rtv_formats_declarationContext *_localctx = _tracker.createInstance<Rtv_formats_declarationContext>(_ctx, getState());
  enterRule(_localctx, 22, SIGParser::RuleRtv_formats_declaration);

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
    setState(243);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(240);
        option_block(); 
      }
      setState(245);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    }
    setState(246);
    match(SIGParser::T__4);
    setState(247);
    match(SIGParser::ASSIGN);
    setState(248);
    array_value_ids();
    setState(249);
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

SIGParser::Blends_declarationContext* SIGParser::blends_declaration() {
  Blends_declarationContext *_localctx = _tracker.createInstance<Blends_declarationContext>(_ctx, getState());
  enterRule(_localctx, 24, SIGParser::RuleBlends_declaration);

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
    setState(254);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(251);
        option_block(); 
      }
      setState(256);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    }
    setState(257);
    match(SIGParser::T__5);
    setState(258);
    match(SIGParser::ASSIGN);
    setState(259);
    array_value_ids();
    setState(260);
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

SIGParser::PointerContext* SIGParser::pointer() {
  PointerContext *_localctx = _tracker.createInstance<PointerContext>(_ctx, getState());
  enterRule(_localctx, 26, SIGParser::RulePointer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(262);
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

SIGParser::Pso_paramContext* SIGParser::pso_param() {
  Pso_paramContext *_localctx = _tracker.createInstance<Pso_paramContext>(_ctx, getState());
  enterRule(_localctx, 28, SIGParser::RulePso_param);

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
    setState(267);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(264);
        option_block(); 
      }
      setState(269);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    }
    setState(270);
    pso_param_id();
    setState(271);
    match(SIGParser::ASSIGN);
    setState(272);
    value_id();
    setState(273);
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

SIGParser::Class_no_templateContext* SIGParser::class_no_template() {
  Class_no_templateContext *_localctx = _tracker.createInstance<Class_no_templateContext>(_ctx, getState());
  enterRule(_localctx, 30, SIGParser::RuleClass_no_template);

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

SIGParser::Type_with_templateContext* SIGParser::type_with_template() {
  Type_with_templateContext *_localctx = _tracker.createInstance<Type_with_templateContext>(_ctx, getState());
  enterRule(_localctx, 32, SIGParser::RuleType_with_template);
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
    class_no_template();
    setState(286);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::LT) {
      setState(278);
      match(SIGParser::LT);
      setState(282);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SIGParser::ID) {
        setState(279);
        template_id();
        setState(284);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(285);
      match(SIGParser::GT);
    }
    setState(289);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::POINTER) {
      setState(288);
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

SIGParser::Inherit_idContext* SIGParser::inherit_id() {
  Inherit_idContext *_localctx = _tracker.createInstance<Inherit_idContext>(_ctx, getState());
  enterRule(_localctx, 34, SIGParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(291);
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

SIGParser::Name_idContext* SIGParser::name_id() {
  Name_idContext *_localctx = _tracker.createInstance<Name_idContext>(_ctx, getState());
  enterRule(_localctx, 36, SIGParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(293);
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

SIGParser::Option_idContext* SIGParser::option_id() {
  Option_idContext *_localctx = _tracker.createInstance<Option_idContext>(_ctx, getState());
  enterRule(_localctx, 38, SIGParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
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

SIGParser::Owner_idContext* SIGParser::owner_id() {
  Owner_idContext *_localctx = _tracker.createInstance<Owner_idContext>(_ctx, getState());
  enterRule(_localctx, 40, SIGParser::RuleOwner_id);

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

SIGParser::Template_idContext* SIGParser::template_id() {
  Template_idContext *_localctx = _tracker.createInstance<Template_idContext>(_ctx, getState());
  enterRule(_localctx, 42, SIGParser::RuleTemplate_id);

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

SIGParser::Function_idContext* SIGParser::function_id() {
  Function_idContext *_localctx = _tracker.createInstance<Function_idContext>(_ctx, getState());
  enterRule(_localctx, 44, SIGParser::RuleFunction_id);
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
    setState(301);
    match(SIGParser::ID);
    setState(302);
    match(SIGParser::OPAR);
    setState(304);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 58) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 58)) & 3670019) != 0)) {
      setState(303);
      value_id_ignore();
    }
    setState(310);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__1) {
      setState(306);
      match(SIGParser::T__1);
      setState(307);
      value_id_ignore();
      setState(312);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(313);
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

SIGParser::Value_idContext* SIGParser::value_id() {
  Value_idContext *_localctx = _tracker.createInstance<Value_idContext>(_ctx, getState());
  enterRule(_localctx, 46, SIGParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(322);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(315);
      shader_type();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(316);
      match(SIGParser::ID);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(317);
      match(SIGParser::INT_SCALAR);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(318);
      match(SIGParser::FLOAT_SCALAR);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(319);
      bool_type();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(320);
      function_id();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(321);
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

SIGParser::Value_id_ignoreContext* SIGParser::value_id_ignore() {
  Value_id_ignoreContext *_localctx = _tracker.createInstance<Value_id_ignoreContext>(_ctx, getState());
  enterRule(_localctx, 48, SIGParser::RuleValue_id_ignore);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(328);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(324);
        match(SIGParser::ID);
        break;
      }

      case SIGParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(325);
        match(SIGParser::INT_SCALAR);
        break;
      }

      case SIGParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(326);
        match(SIGParser::FLOAT_SCALAR);
        break;
      }

      case SIGParser::TRUE:
      case SIGParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(327);
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

SIGParser::Type_idContext* SIGParser::type_id() {
  Type_idContext *_localctx = _tracker.createInstance<Type_idContext>(_ctx, getState());
  enterRule(_localctx, 50, SIGParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(330);
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

SIGParser::Insert_blockContext* SIGParser::insert_block() {
  Insert_blockContext *_localctx = _tracker.createInstance<Insert_blockContext>(_ctx, getState());
  enterRule(_localctx, 52, SIGParser::RuleInsert_block);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(332);
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

SIGParser::Path_idContext* SIGParser::path_id() {
  Path_idContext *_localctx = _tracker.createInstance<Path_idContext>(_ctx, getState());
  enterRule(_localctx, 54, SIGParser::RulePath_id);

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
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(334);
        match(SIGParser::ID);
        setState(335);
        match(SIGParser::DIV); 
      }
      setState(340);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
    setState(341);
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

SIGParser::InheritContext* SIGParser::inherit() {
  InheritContext *_localctx = _tracker.createInstance<InheritContext>(_ctx, getState());
  enterRule(_localctx, 56, SIGParser::RuleInherit);

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
    setState(343);
    match(SIGParser::T__6);
    setState(344);
    inherit_id();
    setState(349);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(345);
        match(SIGParser::T__1);
        setState(346);
        inherit_id(); 
      }
      setState(351);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx);
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

SIGParser::Layout_statContext* SIGParser::layout_stat() {
  Layout_statContext *_localctx = _tracker.createInstance<Layout_statContext>(_ctx, getState());
  enterRule(_localctx, 58, SIGParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(355);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(352);
        slot_declaration();
        break;
      }

      case SIGParser::T__2: {
        enterOuterAlt(_localctx, 2);
        setState(353);
        sampler_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(354);
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

SIGParser::Layout_blockContext* SIGParser::layout_block() {
  Layout_blockContext *_localctx = _tracker.createInstance<Layout_blockContext>(_ctx, getState());
  enterRule(_localctx, 60, SIGParser::RuleLayout_block);
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
    setState(360);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__2 || _la == SIGParser::SLOT

    || _la == SIGParser::COMMENT) {
      setState(357);
      layout_stat();
      setState(362);
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

SIGParser::Layout_definitionContext* SIGParser::layout_definition() {
  Layout_definitionContext *_localctx = _tracker.createInstance<Layout_definitionContext>(_ctx, getState());
  enterRule(_localctx, 62, SIGParser::RuleLayout_definition);
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
    setState(363);
    match(SIGParser::LAYOUT);
    setState(364);
    name_id();
    setState(366);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(365);
      inherit();
    }
    setState(368);
    match(SIGParser::OBRACE);
    setState(369);
    layout_block();
    setState(370);
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

SIGParser::Table_statContext* SIGParser::table_stat() {
  Table_statContext *_localctx = _tracker.createInstance<Table_statContext>(_ctx, getState());
  enterRule(_localctx, 64, SIGParser::RuleTable_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(375);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(372);
        value_declaration();
        break;
      }

      case SIGParser::INSERT_BLOCK: {
        enterOuterAlt(_localctx, 2);
        setState(373);
        insert_block();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(374);
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

SIGParser::Table_blockContext* SIGParser::table_block() {
  Table_blockContext *_localctx = _tracker.createInstance<Table_blockContext>(_ctx, getState());
  enterRule(_localctx, 66, SIGParser::RuleTable_block);
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
    setState(380);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & 1109393409) != 0)) {
      setState(377);
      table_stat();
      setState(382);
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

SIGParser::Table_definitionContext* SIGParser::table_definition() {
  Table_definitionContext *_localctx = _tracker.createInstance<Table_definitionContext>(_ctx, getState());
  enterRule(_localctx, 68, SIGParser::RuleTable_definition);
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
    setState(386);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(383);
        option_block(); 
      }
      setState(388);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    }
    setState(389);
    match(SIGParser::STRUCT);
    setState(390);
    name_id();
    setState(392);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(391);
      inherit();
    }
    setState(394);
    match(SIGParser::OBRACE);
    setState(395);
    table_block();
    setState(396);
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

SIGParser::Rt_color_declarationContext* SIGParser::rt_color_declaration() {
  Rt_color_declarationContext *_localctx = _tracker.createInstance<Rt_color_declarationContext>(_ctx, getState());
  enterRule(_localctx, 70, SIGParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(398);
    type_id();
    setState(399);
    name_id();
    setState(400);
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

SIGParser::Rt_ds_declarationContext* SIGParser::rt_ds_declaration() {
  Rt_ds_declarationContext *_localctx = _tracker.createInstance<Rt_ds_declarationContext>(_ctx, getState());
  enterRule(_localctx, 72, SIGParser::RuleRt_ds_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(402);
    match(SIGParser::DSV);
    setState(403);
    name_id();
    setState(404);
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

SIGParser::Rt_statContext* SIGParser::rt_stat() {
  Rt_statContext *_localctx = _tracker.createInstance<Rt_statContext>(_ctx, getState());
  enterRule(_localctx, 74, SIGParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(409);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(406);
        rt_color_declaration();
        break;
      }

      case SIGParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(407);
        rt_ds_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(408);
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

SIGParser::Rt_blockContext* SIGParser::rt_block() {
  Rt_blockContext *_localctx = _tracker.createInstance<Rt_blockContext>(_ctx, getState());
  enterRule(_localctx, 76, SIGParser::RuleRt_block);
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
    setState(414);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 69) != 0)) {
      setState(411);
      rt_stat();
      setState(416);
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

SIGParser::Rt_definitionContext* SIGParser::rt_definition() {
  Rt_definitionContext *_localctx = _tracker.createInstance<Rt_definitionContext>(_ctx, getState());
  enterRule(_localctx, 78, SIGParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(417);
    match(SIGParser::RT);
    setState(418);
    name_id();
    setState(419);
    match(SIGParser::OBRACE);
    setState(420);
    rt_block();
    setState(421);
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

SIGParser::Array_value_holderContext* SIGParser::array_value_holder() {
  Array_value_holderContext *_localctx = _tracker.createInstance<Array_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 80, SIGParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(423);
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

SIGParser::Array_value_idsContext* SIGParser::array_value_ids() {
  Array_value_idsContext *_localctx = _tracker.createInstance<Array_value_idsContext>(_ctx, getState());
  enterRule(_localctx, 82, SIGParser::RuleArray_value_ids);
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
    setState(425);
    match(SIGParser::OBRACE);
    setState(426);
    array_value_holder();
    setState(431);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__1) {
      setState(427);
      match(SIGParser::T__1);
      setState(428);
      array_value_holder();
      setState(433);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(434);
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

SIGParser::Root_sigContext* SIGParser::root_sig() {
  Root_sigContext *_localctx = _tracker.createInstance<Root_sigContext>(_ctx, getState());
  enterRule(_localctx, 84, SIGParser::RuleRoot_sig);

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
    match(SIGParser::ROOTSIG);
    setState(437);
    match(SIGParser::ASSIGN);
    setState(438);
    name_id();
    setState(439);
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

SIGParser::ShaderContext* SIGParser::shader() {
  ShaderContext *_localctx = _tracker.createInstance<ShaderContext>(_ctx, getState());
  enterRule(_localctx, 86, SIGParser::RuleShader);

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
    setState(444);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(441);
        option_block(); 
      }
      setState(446);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    }
    setState(447);
    shader_type();
    setState(448);
    match(SIGParser::ASSIGN);
    setState(449);
    path_id();
    setState(450);
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

SIGParser::Compute_pso_statContext* SIGParser::compute_pso_stat() {
  Compute_pso_statContext *_localctx = _tracker.createInstance<Compute_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 88, SIGParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(456);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(452);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(453);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(454);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(455);
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

SIGParser::Compute_pso_blockContext* SIGParser::compute_pso_block() {
  Compute_pso_blockContext *_localctx = _tracker.createInstance<Compute_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 90, SIGParser::RuleCompute_pso_block);
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
    setState(461);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72057594038976272) != 0) || _la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(458);
      compute_pso_stat();
      setState(463);
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

SIGParser::Compute_pso_definitionContext* SIGParser::compute_pso_definition() {
  Compute_pso_definitionContext *_localctx = _tracker.createInstance<Compute_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 92, SIGParser::RuleCompute_pso_definition);
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
    setState(464);
    match(SIGParser::COMPUTE_PSO);
    setState(465);
    name_id();
    setState(467);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(466);
      inherit();
    }
    setState(469);
    match(SIGParser::OBRACE);
    setState(470);
    compute_pso_block();
    setState(471);
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

SIGParser::Graphics_pso_statContext* SIGParser::graphics_pso_stat() {
  Graphics_pso_statContext *_localctx = _tracker.createInstance<Graphics_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 94, SIGParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(480);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(473);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(474);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(475);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(476);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(477);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(478);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(479);
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

SIGParser::Graphics_pso_blockContext* SIGParser::graphics_pso_block() {
  Graphics_pso_blockContext *_localctx = _tracker.createInstance<Graphics_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 96, SIGParser::RuleGraphics_pso_block);
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
    setState(485);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72057662757404528) != 0) || _la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(482);
      graphics_pso_stat();
      setState(487);
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

SIGParser::Graphics_pso_definitionContext* SIGParser::graphics_pso_definition() {
  Graphics_pso_definitionContext *_localctx = _tracker.createInstance<Graphics_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 98, SIGParser::RuleGraphics_pso_definition);
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
    setState(491);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(488);
        option_block(); 
      }
      setState(493);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
    setState(494);
    match(SIGParser::GRAPHICS_PSO);
    setState(495);
    name_id();
    setState(497);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(496);
      inherit();
    }
    setState(499);
    match(SIGParser::OBRACE);
    setState(500);
    graphics_pso_block();
    setState(501);
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

SIGParser::Rtx_pso_statContext* SIGParser::rtx_pso_stat() {
  Rtx_pso_statContext *_localctx = _tracker.createInstance<Rtx_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 100, SIGParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(505);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(503);
        root_sig();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(504);
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

SIGParser::Rtx_pso_blockContext* SIGParser::rtx_pso_block() {
  Rtx_pso_blockContext *_localctx = _tracker.createInstance<Rtx_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 102, SIGParser::RuleRtx_pso_block);
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
    setState(510);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(507);
      rtx_pso_stat();
      setState(512);
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

SIGParser::Rtx_pso_definitionContext* SIGParser::rtx_pso_definition() {
  Rtx_pso_definitionContext *_localctx = _tracker.createInstance<Rtx_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 104, SIGParser::RuleRtx_pso_definition);
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
    setState(513);
    match(SIGParser::RAYTRACE_PSO);
    setState(514);
    name_id();
    setState(516);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(515);
      inherit();
    }
    setState(518);
    match(SIGParser::OBRACE);
    setState(519);
    rtx_pso_block();
    setState(520);
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

SIGParser::Workgraph_pso_statContext* SIGParser::workgraph_pso_stat() {
  Workgraph_pso_statContext *_localctx = _tracker.createInstance<Workgraph_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 106, SIGParser::RuleWorkgraph_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(526);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(522);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(523);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(524);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(525);
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

SIGParser::Workgraph_pso_blockContext* SIGParser::workgraph_pso_block() {
  Workgraph_pso_blockContext *_localctx = _tracker.createInstance<Workgraph_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 108, SIGParser::RuleWorkgraph_pso_block);
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
    setState(531);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72057594038976272) != 0) || _la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(528);
      workgraph_pso_stat();
      setState(533);
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

SIGParser::Workgraph_pso_definitionContext* SIGParser::workgraph_pso_definition() {
  Workgraph_pso_definitionContext *_localctx = _tracker.createInstance<Workgraph_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 110, SIGParser::RuleWorkgraph_pso_definition);
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
    setState(534);
    match(SIGParser::WORKGRAPH_PSO);
    setState(535);
    name_id();
    setState(537);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(536);
      inherit();
    }
    setState(539);
    match(SIGParser::OBRACE);
    setState(540);
    workgraph_pso_block();
    setState(541);
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

SIGParser::Rtx_pass_statContext* SIGParser::rtx_pass_stat() {
  Rtx_pass_statContext *_localctx = _tracker.createInstance<Rtx_pass_statContext>(_ctx, getState());
  enterRule(_localctx, 112, SIGParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(546);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(543);
      shader();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(544);
      match(SIGParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(545);
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

SIGParser::Rtx_pass_blockContext* SIGParser::rtx_pass_block() {
  Rtx_pass_blockContext *_localctx = _tracker.createInstance<Rtx_pass_blockContext>(_ctx, getState());
  enterRule(_localctx, 114, SIGParser::RuleRtx_pass_block);
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
    setState(551);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72057662757404416) != 0) || _la == SIGParser::COMMENT) {
      setState(548);
      rtx_pass_stat();
      setState(553);
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

SIGParser::Rtx_pass_definitionContext* SIGParser::rtx_pass_definition() {
  Rtx_pass_definitionContext *_localctx = _tracker.createInstance<Rtx_pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 116, SIGParser::RuleRtx_pass_definition);
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
    setState(557);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(554);
        option_block(); 
      }
      setState(559);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx);
    }
    setState(560);
    match(SIGParser::RAYTRACE_PASS);
    setState(561);
    name_id();
    setState(563);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(562);
      inherit();
    }
    setState(565);
    match(SIGParser::OBRACE);
    setState(566);
    rtx_pass_block();
    setState(567);
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

SIGParser::Rtx_raygen_statContext* SIGParser::rtx_raygen_stat() {
  Rtx_raygen_statContext *_localctx = _tracker.createInstance<Rtx_raygen_statContext>(_ctx, getState());
  enterRule(_localctx, 118, SIGParser::RuleRtx_raygen_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(571);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::T__7:
      case SIGParser::T__8:
      case SIGParser::T__9:
      case SIGParser::T__10:
      case SIGParser::T__11:
      case SIGParser::T__12:
      case SIGParser::T__13:
      case SIGParser::T__14:
      case SIGParser::T__15:
      case SIGParser::T__16:
      case SIGParser::T__17:
      case SIGParser::T__18:
      case SIGParser::OSBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(569);
        shader();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(570);
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

SIGParser::Rtx_raygen_blockContext* SIGParser::rtx_raygen_block() {
  Rtx_raygen_blockContext *_localctx = _tracker.createInstance<Rtx_raygen_blockContext>(_ctx, getState());
  enterRule(_localctx, 120, SIGParser::RuleRtx_raygen_block);
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
    setState(576);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72057594038976256) != 0) || _la == SIGParser::COMMENT) {
      setState(573);
      rtx_raygen_stat();
      setState(578);
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

SIGParser::Rtx_raygen_definitionContext* SIGParser::rtx_raygen_definition() {
  Rtx_raygen_definitionContext *_localctx = _tracker.createInstance<Rtx_raygen_definitionContext>(_ctx, getState());
  enterRule(_localctx, 122, SIGParser::RuleRtx_raygen_definition);
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
    setState(582);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(579);
        option_block(); 
      }
      setState(584);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx);
    }
    setState(585);
    match(SIGParser::RAYTRACE_RAYGEN);
    setState(586);
    name_id();
    setState(588);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(587);
      inherit();
    }
    setState(590);
    match(SIGParser::OBRACE);
    setState(591);
    rtx_raygen_block();
    setState(592);
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

SIGParser::View_declarationContext* SIGParser::view_declaration() {
  View_declarationContext *_localctx = _tracker.createInstance<View_declarationContext>(_ctx, getState());
  enterRule(_localctx, 124, SIGParser::RuleView_declaration);

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
    setState(597);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(594);
        option_block(); 
      }
      setState(599);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx);
    }
    setState(600);
    type_id();
    setState(601);
    name_id();
    setState(602);
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

SIGParser::View_statContext* SIGParser::view_stat() {
  View_statContext *_localctx = _tracker.createInstance<View_statContext>(_ctx, getState());
  enterRule(_localctx, 126, SIGParser::RuleView_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(606);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(604);
        view_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(605);
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

SIGParser::View_blockContext* SIGParser::view_block() {
  View_blockContext *_localctx = _tracker.createInstance<View_blockContext>(_ctx, getState());
  enterRule(_localctx, 128, SIGParser::RuleView_block);
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
    setState(611);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & 35651585) != 0)) {
      setState(608);
      view_stat();
      setState(613);
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

SIGParser::View_definitionContext* SIGParser::view_definition() {
  View_definitionContext *_localctx = _tracker.createInstance<View_definitionContext>(_ctx, getState());
  enterRule(_localctx, 130, SIGParser::RuleView_definition);
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
    setState(617);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(614);
        option_block(); 
      }
      setState(619);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    }
    setState(620);
    match(SIGParser::VIEW);
    setState(621);
    name_id();
    setState(623);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(622);
      inherit();
    }
    setState(625);
    match(SIGParser::OBRACE);
    setState(626);
    view_block();
    setState(627);
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

SIGParser::Pass_definitionContext* SIGParser::pass_definition() {
  Pass_definitionContext *_localctx = _tracker.createInstance<Pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 132, SIGParser::RulePass_definition);
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
    setState(632);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(629);
        option_block(); 
      }
      setState(634);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx);
    }
    setState(635);
    match(SIGParser::PASS);
    setState(636);
    name_id();
    setState(638);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(637);
      inherit();
    }
    setState(640);
    match(SIGParser::OBRACE);
    setState(641);
    view_block();
    setState(642);
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

SIGParser::Pipeline_statContext* SIGParser::pipeline_stat() {
  Pipeline_statContext *_localctx = _tracker.createInstance<Pipeline_statContext>(_ctx, getState());
  enterRule(_localctx, 134, SIGParser::RulePipeline_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(648);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(644);
        name_id();
        setState(645);
        match(SIGParser::SCOL);
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(647);
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

SIGParser::Pipeline_blockContext* SIGParser::pipeline_block() {
  Pipeline_blockContext *_localctx = _tracker.createInstance<Pipeline_blockContext>(_ctx, getState());
  enterRule(_localctx, 136, SIGParser::RulePipeline_block);
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
    setState(653);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::ID

    || _la == SIGParser::COMMENT) {
      setState(650);
      pipeline_stat();
      setState(655);
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

SIGParser::Pipeline_definitionContext* SIGParser::pipeline_definition() {
  Pipeline_definitionContext *_localctx = _tracker.createInstance<Pipeline_definitionContext>(_ctx, getState());
  enterRule(_localctx, 138, SIGParser::RulePipeline_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(656);
    match(SIGParser::PIPELINE);
    setState(657);
    name_id();
    setState(658);
    match(SIGParser::OBRACE);
    setState(659);
    pipeline_block();
    setState(660);
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

SIGParser::Shader_typeContext* SIGParser::shader_type() {
  Shader_typeContext *_localctx = _tracker.createInstance<Shader_typeContext>(_ctx, getState());
  enterRule(_localctx, 140, SIGParser::RuleShader_type);
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
    setState(662);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1048320) != 0))) {
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

SIGParser::Pso_param_idContext* SIGParser::pso_param_id() {
  Pso_param_idContext *_localctx = _tracker.createInstance<Pso_param_idContext>(_ctx, getState());
  enterRule(_localctx, 142, SIGParser::RulePso_param_id);
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
    setState(664);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68718428160) != 0))) {
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

SIGParser::Bool_typeContext* SIGParser::bool_type() {
  Bool_typeContext *_localctx = _tracker.createInstance<Bool_typeContext>(_ctx, getState());
  enterRule(_localctx, 144, SIGParser::RuleBool_type);
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
    setState(666);
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
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  sigParserInitialize();
#else
  ::antlr4::internal::call_once(sigParserOnceFlag, sigParserInitialize);
#endif
}
