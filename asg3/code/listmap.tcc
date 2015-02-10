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
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (link* next, link* prev,
        const value_type& value):
    link (static_cast<node*>(next), 
            static_cast<node*>(prev)), value (value) {
    }
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
    node* n;
    if (anchor_.next == anchor()) {
        n = new node(anchor(), anchor(), pair);
        anchor_.next = n;
        anchor_.prev = n;
        return iterator(n);
    }
    for(n = anchor_.next; n != anchor(); n = n->next) {
        TRACE ('l', "node->value: " << n->value);
        if (n->value.first == pair.first) {
            n->value.second = pair.second;
            return iterator(n);
        }
    }
    n = new node(anchor(), anchor_.prev, pair);
    anchor_.prev->next = n;
    anchor_.prev = n;
    return iterator(n);
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {
    TRACE ('l', that);
    node* n;
    for(n = anchor_.next; n != anchor(); n = n->next) {
        if (n->value.first == that)
            return iterator(n);
    }
    TRACE('l', "key: " << " not found");
    return end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
    TRACE ('l', &*position);
    position.where->prev->next = position.where->next;
    TRACE ('l', "position.where->prev->next = " << position.where->next);
    position.where->next->prev = position.where->prev;
    TRACE ('l', "position.where->next->prev = " << position.where->prev);
    return iterator(position.where->next);
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

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

