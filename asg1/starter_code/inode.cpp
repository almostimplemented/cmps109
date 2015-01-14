// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

#include <iostream>
#include <stdexcept>

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

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir(const string& dirname) {
    DEBUGF ('i', dirname);
    if (dirents.find(dirname) != dirents.end())
        throw yshell_exn ("dirname exists");
    inode_ptr parent = dirents["."];
    inode_ptr dirnode = make_shared<inode>(DIR_INODE);
    directory_ptr dir = directory_ptr_of(dirnode->get_contents());
    dir->set_parent_child(parent, dirnode);
    dirents.insert(make_pair(dirname, dirnode));
    return dirnode;
}

inode_ptr directory::mkfile (const string& filename) {
    DEBUGF ('i', filename);
    if (dirents.find(filename) != dirents.end())
        throw yshell_exn ("filename exists");
    inode_ptr file = make_shared<inode>(PLAIN_INODE);
    dirents.insert(make_pair(filename, file));
    return file;
}

void directory::set_root(inode_ptr root) {
    dirents.insert(make_pair(".", root));
    dirents.insert(make_pair("..", root));
}

void directory::set_parent_child(inode_ptr parent, inode_ptr child) {
    dirents.insert(make_pair("..", parent));
    dirents.insert(make_pair(".", child));
}

inode_ptr directory::lookup(const string& name) {
    auto it = dirents.find(name);
    if (it == dirents.end())
        throw yshell_exn("directory or filename \"" + 
                name + "\" does not exist");
    return it->second;
}

vector<string> directory::entries() {
    vector<string> v;
    for (auto it = dirents.begin();
            it != dirents.end();
            it++) {
        v.push_back(to_string(it->second->get_inode_nr()) + "\t" +
         to_string(it->second->size())
          + "\t" + it->first);
    }
    return v;
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
        from = found + 1;
    }
    return p;
}

void inode_state::mkdir(const string& pathname) {
    inode_ptr p = resolve_pathname(pathname);
    directory_ptr dir = directory_ptr_of(p->get_contents());
    size_t found = pathname.find_last_of("/");
    if (found == string::npos)
        dir->mkdir(pathname);
    else
        dir->mkdir(pathname.substr(found+1));
}

vector<string> inode_state::ls() {
    vector<string> v;
    directory_ptr dir = directory_ptr_of(cwd->contents);
    return dir->entries();
}

vector<string> inode_state::ls(const string& pathname) {
    inode_ptr p = resolve_pathname(pathname);
    vector<string> v;
    if (p->get_type() == PLAIN_INODE) {
        v.push_back(to_string(p->get_inode_nr()) + "\t" +
                    to_string(p->size())         + "\t" +
                    pathname);
        return v;
    }
    directory_ptr dir = directory_ptr_of(p->contents);
    if (pathname.back() != '/') {
        size_t found = pathname.find_last_of("/");
        if (found == string::npos)
            p = dir->lookup(pathname);
        else
            p = dir->lookup(pathname.substr(found+1));
        dir = directory_ptr_of(p->contents);
    }
    return dir->entries();
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}
