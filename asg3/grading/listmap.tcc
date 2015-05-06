// $Id: listmap.tcc,v 1.5 2014-07-09 11:50:34-07 - - $

#include "listmap.h"
#include "trace.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (node*, node*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
        const value_type& value):
    link (next, prev), value (value) {
    }

//
// listmap::node::node (link*, link*, const value_type&)
//
//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
    TRACE ('l', (void*) this);
    while (anchor_.next != anchor())
        erase(begin());
}

//
// listmap::empty()
//
template <typename Key, typename Value, class Less>
bool listmap<Key,Value,Less>::empty() const {
    return anchor_.next == anchor();
}

//
// listmap::iterator listmap::begin()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::begin() {
    return iterator (anchor_.next);
}

//
// listmap::iterator listmap::end()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::end() {
    return iterator (anchor());
}

//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
    TRACE ('l', &pair << "->" << pair);
    iterator i;
    for(i = begin(); i != end() and less(i->first, pair.first); ++i);
    if (i != end() and not less(pair.first, i->first)) {
        // Key found; update:
        i->second = pair.second;
        return i;
    }
    // Key not found; make new
    node* n = new node(i.where, i.where->prev, pair);
    i.where->prev->next = n;
    i.where->prev = n;
    return iterator(n);
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {
    TRACE ('l', that);
    iterator i;
    for(i = begin(); i != end() and less(i->first, that); ++i);
    if (i != end() and not less(that, i->first)) {
        // Key found; return:
        return i;
    }

    return end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
    TRACE ('l',&*position);
    position.where->prev->next = position.where->next;
    TRACE ('l',"position.where->prev->next = " << position.where->next);
    position.where->next->prev = position.where->prev;
    TRACE ('l',"position.where->next->prev = " << position.where->prev);
    iterator retval(position.where->next);
    position.erase();
    return retval;
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

// 
// void listmap::iterator::erase()
//
template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::iterator::erase() {
    delete where;
}

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
    TRACE ('l', where);
    return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
    TRACE ('l', where);
    return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
    TRACE ('l', where);
    where = where->next;
    return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
    TRACE ('l', where);
    where = where->prev;
    return *this;
}

//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
(const iterator& that) const {
    return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
(const iterator& that) const {
    return this->where != that.where;
}

