// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

void fn_cat (inode_state& state, const wordvec& words){
    if (words.size() < 2) {
        throw yshell_exn ("cat: must specify file"); 
    }
    for (size_t i = 1; i < words.size(); i++) {
        state.cat(words.at(i), cout);
    }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_cd (inode_state& state, const wordvec& words){
    if (words.size() == 1)
        state.cd();
    else if (words.size() == 2)
        state.cd(words.at(1));
    else
        throw yshell_exn ("cd: only one operand may be given");
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec args(words.begin() + 1, words.end());
   cout << args << endl; 
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit_exn();
}

void fn_ls (inode_state& state, const wordvec& words){
    if (words.size() == 1)
        state.ls(cout);
    else
        for (size_t i = 1; i < words.size(); i++)
            state.ls(words.at(i), cout);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
    if (words.at(1).back() == '/')
        throw yshell_exn("make: " +words.at(1)+ ": invalid filename");
    if (words.size() == 2)
        state.make(words.at(1));
    else if (words.size() > 2) {
        wordvec args(words.begin() + 2, words.end());
        state.make(words.at(1), args);
    } else {
        throw yshell_exn("make: must specify filename");
    }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() != 2) 
       throw yshell_exn ("usage: mkdir dirname");
   state.mkdir(words.at(1));
}

void fn_prompt (inode_state& state, const wordvec& words){
    state.set_prompt(words);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
    state.pwd(cout);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

