#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <string>

#include "Test.h"
#include "Variable.h"

typedef std::map<std::string, Variable> MapVariable;

extern MapVariable Variables;

void ReadConf(TestSuite &suite);

#endif /* COMMON_H */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
