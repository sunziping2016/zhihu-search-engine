//
// Created by sun on 12/10/16.
//

#include "trie.h"

using namespace std;

bool trie_tree::insert(const myu32string &str, size_t pos, size_t len) {
    trie_tree_node *temp = &root;
    for (size_t i = pos, end = pos + len; i < end; ++i) {
        trie_tree_node::map_type::iterator iter = temp->children.find(str[i]);
        if (iter == temp->children.end())
            iter = temp->children.insert(mymake_pair(str[i], new trie_tree_node)).first;
        temp = (*iter).second;
    }
    if (temp->is_key)
        return false;
    temp->is_key = true;
    return true;
}

size_t trie_tree::match(const myu32string &str, size_t pos) {
    size_t longest_match = pos, i = pos;
    trie_tree_node *temp = &root;
    while (true) {
        if (temp->is_key)
            longest_match = i;
        if (i == str.size())
            break;
        trie_tree_node::map_type::iterator iter = temp->children.find(str[i]);
        if (iter == temp->children.end())
            break;
        temp = (*iter).second;
        ++i;
    }
    return longest_match - pos;
}
