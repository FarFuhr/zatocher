#include "stringutils.h"

#include <algorithm>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

bool toBool(std::string str){
    if(str.compare("0") == 0)
        return false;
    else if(str.compare("1") == 0)
        return true;

    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    std::istringstream is(str);
    bool ret;
    is >> std::boolalpha >> ret;

    return ret;
}
