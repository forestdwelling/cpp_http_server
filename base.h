#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>

void errorExit(const char *sc);
std::vector<std::string> split(std::string &s, const char *delim);
std::string readFile(std::string fileName);

#endif