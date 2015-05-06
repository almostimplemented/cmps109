// $Id: util.cpp,v 1.4 2014-04-24 18:14:51-07 - - $

using namespace std;

#include "util.h"

string trim(const string& str) {
   size_t first = str.find_first_not_of (" \t");
   if (first == string::npos) return "";
   size_t last = str.find_last_not_of (" \t");
   return str.substr (first, last - first + 1);
}
