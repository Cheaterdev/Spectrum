// nonstd lib uses std on cpp23+
// jinja2cpp is compiled with c++14
// force using nonstd version
#define nsel_CONFIG_SELECT_EXPECTED 1

#include <jinja2cpp/template.h>
#include <jinja2cpp/binding/rapid_json.h>
#include <jinja2cpp/user_callable.h>
#include <jinja2cpp/template_env.h>
#include <jinja2cpp/generic_list_iterator.h>