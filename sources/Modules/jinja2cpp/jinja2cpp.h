#pragma once
// jinja2cpp wrapper.
// jinja2cpp is compiled against C++14; the nonstd expected<> ABI must be
// selected before its headers are parsed into the IFC.
#ifndef nsel_CONFIG_SELECT_EXPECTED
#  define nsel_CONFIG_SELECT_EXPECTED 1
#endif

#include <jinja2cpp/template.h>
#include <jinja2cpp/binding/rapid_json.h>
#include <jinja2cpp/user_callable.h>
#include <jinja2cpp/template_env.h>
#include <jinja2cpp/generic_list_iterator.h>
