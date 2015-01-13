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
           contents = make_shared<directory>(inode_ptr(this));
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

inode::inode(inode_t init_type, inode_ptr parent):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
           throw logic_error ("invalid constructor for plain inode");
           break;
      case DIR_INODE:
           contents = make_shared<directory>(inode_ptr(this), parent);
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}
int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
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

inode_ptr directory::mkdir (const string& dirname) {
    DEBUGF ('i', dirname);
    if (dirents.find(dirname) != dirents.end())
        throw yshell_exn ("dirname exists");
    inode_ptr me = dirents["."];
    inode_ptr dir = make_shared<inode>(DIR_INODE, me);
    return dir;
}

inode_ptr directory::mkfile (const string& filename) {
    DEBUGF ('i', filename);
    if (dirents.find(filename) != dirents.end())
        throw yshell_exn ("filename exists");
    inode_ptr file = make_shared<inode>(PLAIN_INODE);
    dirents.insert(make_pair(filename, file));
    return file;
}

inode_state::inode_state() {
    root = make_shared<inode>(DIR_INODE);
    DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\"");
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

directory::directory(inode_ptr current_inode) {
    dirents = map<string, inode_ptr>();
    dirents.insert(make_pair(".", current_inode));
    dirents.insert(make_pair("..", current_inode));
}

directory::directory(inode_ptr current_inode, inode_ptr parent_inode) {
    dirents.insert(make_pair(".", current_inode));
    dirents.insert(make_pair("..", parent_inode));
}
