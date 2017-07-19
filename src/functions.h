#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <string>
#include <vector>
#include <sstream>

/**
 * @把name_list_str转化为vector
 * @para name_list_str name相关的字符串
 */

std::vector<std::string> GetNameList(std::string & name_list_str)
{
    std::istringstream is(name_list_str);
    std::string server_name;
    std::vector<std::string> name_list;
    while (getline(is, server_name, '|'))
    {
        DEBUG_LOG("server_name is %s", server_name.c_str());
        name_list.push_back(server_name);
    }

    return name_list;
}


#endif
