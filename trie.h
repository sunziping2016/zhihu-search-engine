//
// Created by sun on 12/10/16.
//

#ifndef HTML_PARSER_TRIE_H
#define HTML_PARSER_TRIE_H

#include "mystring.h"
#include "myhashmap.h"

class trie_tree {
public:
    trie_tree() {}
    //trie_tree(trie_tree &&other);
    //trie_tree &operator = (trie_tree &&other);
    trie_tree(const trie_tree &other);
    trie_tree &operator = (const trie_tree &other);

    bool insert(const myu32string &str, std::size_t pos, std::size_t len);
    bool insert(const myu32string &str, std::size_t pos = 0);
    std::size_t match(const myu32string &str, std::size_t pos = 0);

private:
    struct trie_tree_node {
        typedef myhashmap<char32_t, trie_tree_node *> map_type;

        trie_tree_node();
        //trie_tree_node(trie_tree_node &&other);
        //trie_tree_node &operator = (trie_tree_node &&other);
        ~trie_tree_node();
        trie_tree_node(const trie_tree_node &other);
        trie_tree_node &operator = (const trie_tree_node &other);

        bool is_key;
        map_type children;
    };

    trie_tree_node root;
};

#endif //HTML_PARSER_TRIE_H
