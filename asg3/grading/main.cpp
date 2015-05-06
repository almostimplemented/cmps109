// $Id: main.cpp,v 1.7 2015-02-03 15:39:46-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_pair = xpair<const string,string>;
using str_str_map = listmap<string,string>;

void scan_options (int argc, char** argv) {
    opterr = 0;
    for (;;) {
        int option = getopt (argc, argv, "@:");
        if (option == EOF) break;
        switch (option) {
            case '@':
                traceflags::setflags (optarg);
                break;
            default:
                complain() << "-" << (char) optopt << ": invalid option"
                    << endl;
                break;
        }
    }
}

void map_loop(str_str_map& m, string& filename, istream& infile) {
    string key;
    string value;
    for (int i = 1 ;; ++i) {
        string line;
        getline (infile, line);
        string l = trim(line);
        if (infile.eof()) break;
        cout << filename << ": " << i << ": " << line << endl;
        if (l.size() == 0 or l[0] == '#')
            continue;
        size_t pos = line.find_first_of ("=");
        if (pos == string::npos) {
            key = trim(line);
            str_str_map::iterator it = m.find(key);
            if (it == m.end())
                cout << key << ": key not found" << endl;
            else
                cout << *it << endl;
            continue;
        } else {
            key   = trim(line.substr(0, pos == 0 ? 0 : pos));
            value = trim(line.substr(pos + 1));
        }
        if (key == "" and value == "") {
            for (str_str_map::iterator it  = m.begin();
                                      it != m.end();
                                      ++it)
                cout << *it << endl;
        } else if (key == "") {
            for (str_str_map::iterator it  = m.begin();
                                      it != m.end();
                                      ++it)
                if (it->second == value)
                    cout << *it << endl;
        } else if (value == "") {
            str_str_map::iterator it = m.find(key);
            if (it != m.end())
                m.erase(it);
        } else {
            str_str_pair pair(key, value);
            m.insert(pair);
            cout << pair << endl;
        }
    }
}

int main (int argc, char** argv) {
    str_str_map m;
    string execname(argv[0]);
    int exit_status = 0;
    scan_options (argc, argv);
    // Use istream pointer to use one variable for cin and files
    string filename("-");
    if (argc == 1) map_loop(m, filename, cin);
    for (int i = 1; i < argc; i++) {
        filename = string(argv[i]);
        if (filename == "-") {
            map_loop(m, filename, cin);
        } else {
            ifstream infile(filename);
            if ( infile.fail() ) {
                cerr << execname << ": " << filename 
                     << ": No such file or directory" << endl;
                exit_status = 1;
                continue;
            } else {
                map_loop(m, filename, infile);
                infile.close();
            }
        }
    }
    return exit_status;
}

