// Author:  Andrew Edwards
// Email:   ancedwar@ucsc.edu
// ID:      1253060
// Date:    2015 Jan 18

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
           contents = make_shared<plain_file>();
           break;
      case DIR_INODE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

int inode::get_type() const {
    return type;
}

file_base_ptr inode::get_contents() const{
    return contents;
}

size_t inode::size() const {
    if (type == PLAIN_INODE)
        return plain_file_ptr_of(contents)->size();
    else
        return directory_ptr_of(contents)->size();
}

void inode::set_name(const string& iname) {
    name = iname;
}

string inode::get_name() const {
    return name;
}

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}

size_t plain_file::size() const {
   size_t size {0};
   size = data.size();
   for (auto word = data.begin();
             word != data.end();
             word++) {
      size += word->size();
   }
   if (size > 1) size -= 1;
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

size_t directory::size() const {
   size_t size {0};
   size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename,
                        const string& pathname) {
    auto it = dirents.find(filename);
    if (it == dirents.end())
        throw yshell_exn ("rm: " + pathname 
                                 + ": no such file or directory");
    if (it->second->get_type() == PLAIN_INODE)
        dirents.erase(it);
    else {
        inode_ptr p = it->second;
        directory_ptr dp = directory_ptr_of(p->get_contents()); 
        if (dp->size() != 2)
            throw yshell_exn ("rm: " + pathname
                                     + ": directory must be empty");
        dirents.erase(it);
    }
   DEBUGF ('i', filename);
}

void directory::remove_r (const string& filename,
                          const string& pathname) {
    auto it = dirents.find(filename);
    directory_ptr dp; 
    if (it == dirents.end())
        throw yshell_exn ("rmr: " + pathname 
                                  + ": no such directory");
    dp = directory_ptr_of(it->second->get_contents());
    dp->rec_empty();
    dirents.erase(it->first);
   DEBUGF ('i', filename);
}

void directory::rec_empty() {
    inode_ptr p;
    directory_ptr dp;
    for (auto it =  dirents.begin();
              it != dirents.end();
              ) {
        p = it->second;
        DEBUGF ('i', it->first);
        if (it->first == "." || it->first == "..") {
            dirents.erase(it++);
            continue;
        }
        if (p->get_type() == PLAIN_INODE)
            dirents.erase(it++);
        else {
            dp = directory_ptr_of(p->get_contents());
            dp->rec_empty();
            dirents.erase(it++);
        }
    }
}

vector<inode_ptr> directory::subdirs() {
    vector<inode_ptr> subdirs;
    for (auto it = dirents.begin();
              it != dirents.end();
              ++it) {
        if (it->second->get_type() == DIR_INODE
            && it->first != "."
            && it->first != "..")
            subdirs.push_back(it->second);
    }
    return subdirs;
}

inode_ptr directory::mkdir(const string& dirname) {
    DEBUGF ('i', dirname);
    if (dirents.find(dirname) != dirents.end())
        throw yshell_exn ("mkdir: " + dirname + ": dirname exists");
    inode_ptr parent = dirents.at(".");
    inode_ptr dirnode = make_shared<inode>(DIR_INODE);
    dirnode->set_name(dirname);
    directory_ptr dir = directory_ptr_of(dirnode->get_contents());
    dir->set_parent_child(parent, dirnode);
    dirents.insert(make_pair(dirname, dirnode));
    return dirnode;
}

inode_ptr directory::mkfile (const string& filename) {
    DEBUGF ('i', filename);
    if (dirents.find(filename) != dirents.end())
        throw logic_error ("filename exists");
    inode_ptr file = make_shared<inode>(PLAIN_INODE);
    dirents.insert(make_pair(filename, file));
    return file;
}

void directory::set_root(inode_ptr root) {
    dirents.insert(make_pair(".", root));
    dirents.insert(make_pair("..", root));
    root->set_name("/");
}

void directory::set_parent_child(inode_ptr parent, inode_ptr child) {
    dirents.insert(make_pair("..", parent));
    dirents.insert(make_pair(".", child));
}

inode_ptr directory::lookup(const string& name) {
    auto it = dirents.find(name);
    if (it == dirents.end())
        return nullptr;
    return it->second;
}

void directory::ls(ostream& out) {
    string suffix;
    for (auto it = dirents.begin();
              it != dirents.end();
              it++) {
        if (it->second->get_type() == DIR_INODE
            && it->first != "."
            && it->first != "..")
            suffix = "/";
        else
            suffix = "";
        out << setw(6) << it->second->get_inode_nr()
            << setw(6) << it->second->size()
            << "\t" << it->first + suffix << endl;
    }
}

const wordvec& directory::cat(const string& name,
                              const string& pathname) {
    auto it = dirents.find(name);
    if (it == dirents.end()) {
        throw yshell_exn ("cat: " + pathname + ": No such file");
    } else if (it->second->get_type() == DIR_INODE) {
        throw yshell_exn ("cat: " + pathname + ": Not a file");
    } else {
        plain_file_ptr fp = plain_file_ptr_of(
                                it->second->get_contents());
        return fp->readfile();
    }
}

void directory::make(const string& name, const string& pathname) {
    inode_ptr np;
    auto it = dirents.find(name);
    if (it == dirents.end()) {
        mkfile(name);
    } else if (it->second->get_type() == DIR_INODE) {
        throw yshell_exn ("make: " + pathname 
                + ": filename exists as directory");
    } else {
        wordvec v;
        plain_file_ptr fp = plain_file_ptr_of(
                                it->second->get_contents());
        fp->writefile(v);
    }
}

void directory::make(const string& name, 
                     const string& pathname,
                     wordvec& data) {
    inode_ptr np;
    auto it = dirents.find(name);
    if (it == dirents.end()) {
        np = mkfile(name);
        plain_file_ptr fp = plain_file_ptr_of(np->get_contents());
        fp->writefile(data);
    } else if (it->second->get_type() == DIR_INODE) {
        throw yshell_exn ("make: " + pathname 
                + ": filename exists as directory");
    } else {
        plain_file_ptr fp = plain_file_ptr_of(
                                it->second->get_contents());
        fp->writefile(data);
    }
}

inode_state::inode_state() {
    root = make_shared<inode>(DIR_INODE);
    cwd = root;
    directory_ptr_of(root->contents)->set_root(root);
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\"");
}

inode_ptr inode_state::resolve_pathname(const string& pathname) {
    inode_ptr p;
    size_t from = 0, found = 0;
    if (pathname.at(0) == '/') {
        p = root;
        from = 1;
    } else {
        p = cwd;
    }
    directory_ptr dir;
    while (true) {
        found = pathname.find_first_of("/", from);
        DEBUGF ('i', "pathname: \"" << pathname 
                << "\", from: \"" << from
                << "\", found: \"" << found << "\"");
        if (found == string::npos) {
            break;
        }
        dir = directory_ptr_of(p->get_contents());
        p = dir->lookup(pathname.substr(from, found - from));
        if (p == nullptr)
            return p;
        from = found + 1;
    }
    return p;
}

void inode_state::cat(const string& pathname, ostream& out) {
    inode_ptr p = resolve_pathname(pathname);
    if (p == nullptr)
        throw yshell_exn("cat: " + pathname + "No such file");
    string name;
    size_t found = pathname.find_last_of("/");
    if (found == string::npos)
        name = pathname;
    else
        name = pathname.substr(found + 1);
    wordvec data = directory_ptr_of(p->contents)->cat(name, pathname);
    if (data.size() > 0)
        out << data << endl;
}

void inode_state::cd() {
    cwd = root;
}

void inode_state::cd(const string& pathname) {
    inode_ptr p;
    if (pathname.back() != '/')
        p = resolve_pathname(pathname + '/');
    else
        p = resolve_pathname(pathname);
    if (p == nullptr)
        throw yshell_exn("cd: " + pathname +
                         "No such directory");
    cwd = p;
}

void inode_state::ls(ostream& out) {
    out << ".:" << endl;
    directory_ptr dir = directory_ptr_of(cwd->contents);
    dir->ls(out);
}

void inode_state::ls(const string& pathname, ostream& out) {
    inode_ptr p = resolve_pathname(pathname);
    if (p == nullptr)
        throw yshell_exn ("ls: " + pathname +
                         ": No such file or directory");
    if (pathname.back() != '/' || pathname == "/")
        out << pathname << ":" << endl;
    else 
        out << pathname.substr(0, pathname.size() - 1) << ":" << endl;
    if (p->get_type() == PLAIN_INODE) {
        out << setw(6) << p->get_inode_nr()
            << setw(6) << p->size()
            << "\t" << pathname << endl;
        return;
    }
    directory_ptr dir = directory_ptr_of(p->contents);
    if (pathname.back() != '/') {
        size_t found = pathname.find_last_of("/");
        if (found == string::npos)
            p = dir->lookup(pathname);
        else
            p = dir->lookup(pathname.substr(found+1));
        if (p == nullptr) {
            throw yshell_exn ("ls: " + pathname +
                             ": No such file or directory");
        }
        dir = directory_ptr_of(p->contents);
    }
    dir->ls(out);
}

void inode_state::lsr(ostream& out) {
    ls(out);
    directory_ptr dir = directory_ptr_of(cwd->contents);
    vector<inode_ptr> subdirs (dir->subdirs());
    for (size_t i = 0;
                i < subdirs.size();
                i++)
        lsr("./" + subdirs.at(i)->get_name() + "/", out);
}

void inode_state::lsr(const string& pathname, ostream& out) {
    string suffix;
    inode_ptr p; 
    if (pathname.back() == '/')
        suffix = "";
    else
        suffix = "/";
    p = resolve_pathname(pathname + suffix);
    if (p == nullptr)
        throw yshell_exn ("lsr: " + pathname +
                         ": No such file or directory");
    ls(pathname, out);
    directory_ptr dir = directory_ptr_of(p->contents);
    vector<inode_ptr> subdirs (dir->subdirs());
    for (size_t i = 0;
                i < subdirs.size();
                i++)
        lsr(pathname + suffix + subdirs.at(i)->get_name() + '/', out);
}

void inode_state::make(const string& pathname) {
    inode_ptr p = resolve_pathname(pathname);
    if (p == nullptr)
        throw yshell_exn ("make: " + pathname + 
                            ": invalid path");
    string name;
    size_t found = pathname.find_last_of("/");
    if (found == string::npos)
        name = pathname;
    else
        name = pathname.substr(found + 1);
    directory_ptr_of(p->contents)->make(name, pathname);
}

void inode_state::make(const string& pathname, wordvec& data) {
    inode_ptr p = resolve_pathname(pathname);
    if (p == nullptr)
        throw yshell_exn ("make: " + pathname + 
                            ": invalid path");
    string name;
    size_t found = pathname.find_last_of("/");
    if (found == string::npos)
        name = pathname;
    else
        name = pathname.substr(found + 1);
    directory_ptr_of(p->contents)->make(name, pathname, data);
}

void inode_state::mkdir(const string& pathname) {
    string name;
    if (pathname.back() == '/')
        name = pathname.substr(0, pathname.size() - 1);
    else 
        name = pathname;
    inode_ptr p = resolve_pathname(name);
    if (p == nullptr)
        throw yshell_exn ("mkdir: " + pathname + 
                            ": invalid path");
    directory_ptr dir = directory_ptr_of(p->get_contents());
    size_t found = name.find_last_of("/");
    if (found == string::npos)
        dir->mkdir(name);
    else
        dir->mkdir(name.substr(found+1));
}

string inode_state::get_prompt () const {
    return prompt;
}

void inode_state::set_prompt(const wordvec& words) {
    if (words.size() == 1)
        prompt = "% ";
    else {
        prompt = "";
        for (size_t i = 1;
                i < words.size();
                i++)
            prompt += words.at(i) + " ";
    }
}

void inode_state::pwd(ostream& out) {
    wordvec name_stack;
    inode_ptr p = cwd;
    directory_ptr dp;
    while (p != root) {
        name_stack.push_back(p->name);
        dp = directory_ptr_of(p->contents);
        p  = dp->lookup("..");
    }
    if (name_stack.size() == 0) {
        out << '/' << endl;
        return;
    }
    while (name_stack.size() > 0) {
        out << '/';
        out << name_stack.back();
        name_stack.pop_back();
    }
    out << endl;
}

void inode_state::rm(const string& pathname) {
    string target_name, pname;
    bool is_dir;
    if (pathname.back() == '/') {
        pname = pathname.substr(0, pathname.size() - 1);
        is_dir = true;
    } else { 
        pname = pathname;
    }
    inode_ptr p = resolve_pathname(pname);
    size_t found = pname.find_last_of("/");
    if (found == string::npos)
        target_name = pname;
    else
        target_name = pathname.substr(found + 1);
    directory_ptr dir = directory_ptr_of(p->contents);
    p = dir->lookup(target_name);
    if (p == nullptr)
        throw yshell_exn ("rm: " + pathname 
                + ": No such file or directory");
    if (p->type == PLAIN_INODE && is_dir)
        throw yshell_exn ("rm: " + pathname + ": is not a directory");
    dir->remove(target_name, pathname);
}

void inode_state::rmr(const string& pathname) {
    string target_name, pname;
    if (pathname.back() == '/') {
        pname = pathname.substr(0, pathname.size() - 1);
    } else { 
        pname = pathname;
    }
    inode_ptr p = resolve_pathname(pname);
    size_t found = pname.find_last_of("/");
    if (found == string::npos)
        target_name = pname;
    else
        target_name = pathname.substr(found + 1);
    directory_ptr dir = directory_ptr_of(p->contents);
    p = dir->lookup(target_name);
    if (p == nullptr)
        throw yshell_exn ("rm: " + pathname 
                + ": No such file or directory");
    if (p->type == PLAIN_INODE)
        throw yshell_exn ("rm: " + pathname + ": is not a directory");
    dir->remove_r(target_name, pathname);
}

void inode_state::terminate() {
    directory_ptr dp = directory_ptr_of(root->contents);
    dp->rec_empty();
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}
