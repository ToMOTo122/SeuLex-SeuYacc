#ifndef EMITTER_H
#define EMITTER_H

#include "DFAGenerator.h"
#include <string>

void generateYaccCode(const ParseTables& tables, const std::string& outputPath);

#endif
