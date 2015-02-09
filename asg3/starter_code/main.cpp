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

int main (int argc, char** argv) {
    sys_info::set_execname (argv[0]);
    scan_options (argc, argv);
    // Use istream pointer to use one variable for cin and files
    istream* fp;
    string filename;
    str_str_map test;
    string key;
    string value;
    for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
        filename = *argp;
        if (filename == "-")
            fp = &cin;
        else {
            fp = new ifstream(filename);
            if ( !fp->good() ) {
                cerr << "keyvalue: could not open file: " << *argp << endl;
                continue;
            }
        }
        for (;;) {
            string line;
            getline (*fp, line);
            string l = trim(line);
            if (fp->eof()) break;
            if (l.size() == 0 or l[0] == '#')
                continue;
            size_t pos = line.find_first_of ("=");
            cout << "Statement type:";
            if (pos == string::npos) {
                key = line;
                cout << " key " << endl;
                cout << "Key: " << key << endl;
                cout << "Value: " << value << endl;
                continue;
            } else {
                key   = trim(line.substr(0, pos == 0 ? 0 : pos - 1));
                value = trim(line.substr(pos + 1));
            }
            if (key == "" and value == "")
                cout << " = " << endl;
            else if (key == "")
                cout << " = value" << endl;
            else if (value == "")
                cout << " key = " << endl;
            else
                cout << " key = value " << endl;
            cout << "Key: " << key << endl;
            cout << "Value: " << value << endl;
        }
    }

    cout << "EXIT_SUCCESS" << endl;
    return EXIT_SUCCESS;

    for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
        str_str_pair pair (*argp, to_string<int> (argp - argv));
        cout << "Before insert: " << pair << endl;
        test.insert (pair);
    }

    for (str_str_map::iterator itor = test.begin();
            itor != test.end(); ++itor) {
        cout << "During iteration: " << *itor << endl;
    }

    str_str_map::iterator itor = test.begin();
    test.erase (itor);

    cout << "EXIT_SUCCESS" << endl;
    return EXIT_SUCCESS;
}

