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

using namespace std;

void errorExit(const char *msg) {
    char output[100] = {0};
	strcat(output, "ERROR: ");
	strcat(output, msg);
	strcat(output, "\n");
    perror(output);
    exit(1);
}

vector<string> split(string &s, const char *delim) {  
    vector<string> ret;
    int last = 0;
    int index = s.find(delim, last);
    while (index != string::npos) {
        ret.push_back(s.substr(last, index-last));
        last = index + strlen(delim);
        index = s.find(delim, last);
    }
    index = s.size();
    if (index-last > 0) {
        ret.push_back(s.substr(last, index-last));  
    }
    return ret;
}

string readFile(string fileName) {
    ifstream file(fileName);
    stringstream file_buf;
    file_buf << file.rdbuf();
    return file_buf.str();
}