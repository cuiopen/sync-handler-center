#ifndef _CENTER_PACKAGE_H
#define _CENTER_PACKAGE_H
#include "service_group.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
using std::vector;
using std::string;
using std::map;
using std::ofstream;
class CenterPackHandler
{
	uint32_t version_id;
public:
	CenterPackHandler():version_id(0) {}
	bool ReqPack(char*in_buf, int buf_len, const vector<string> &list);
	bool AckPack(const char* out_buf, const int buf_len, map<string, ServiceGroupInfo> &list);
    uint32_t GetVersion();
};

#endif
