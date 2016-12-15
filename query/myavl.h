//
// Created by sun on 12/14/16.
//

#ifndef AVL_MYAVL_H
#define AVL_MYAVL_H

#include <utility>
#include <iostream>

#include "mypair.h"
#include "myfunctional.h"

template<typename Key, typename Compare, typename Allocator>
class myavl_node {
public:
    typedef myavl_node<Key, Compare, Allocator> node_type;

    template<typename K, typename C, typename A>
    friend std::ostream &operator << (std::ostream &out, const myavl_node<K, C, A> &node);

    node_type *lchild, *rchild;
    Key *data;
    std::size_t height;
    Allocator alloc;

    myavl_node(const Key &orig, const Allocator &a)
            : lchild(nullptr), rchild(nullptr), data(nullptr), height(1), alloc(a) {
        data = alloc.allocate(1);
        alloc.construct(data, orig);
    }
    myavl_node(Key &&orig, const Allocator &a)
            : lchild(nullptr), rchild(nullptr), data(nullptr), height(1), alloc(a) {
        data = alloc.allocate(1);
        alloc.construct(data, std::move(orig));
    }
    ~myavl_node() {
        if (data)
            alloc.destroy(data);
    }
};

template<typename Key, typename Compare, typename Allocator>
std::ostream &operator << (std::ostream &out, const myavl_node<Key, Compare, Allocator> &node) {
    if (node.lchild)
        out << *node.lchild;
    if (node.data)
        out << *node.data << " ";
    if (node.rchild)
        out << *node.rchild;
    return out;
};

template<typename Key, typename Compare = myless<Key>, typename Allocator = std::allocator<Key> >
class myavl {
public:
    typedef Key value_type;
    typedef myavl<Key, Compare, Allocator> avl_type;
    typedef myavl_node<Key, Compare, Allocator> node_type;
    typedef typename Allocator::reference reference;
    typedef typename Allocator::const_reference const_reference;

    template<typename K, typename C, typename A>
    friend std::ostream &operator << (std::ostream &out, const myavl<K, C, A> &avl);

    explicit myavl(const Compare &comp = Compare(), const Allocator &alloc = Allocator())
            : root(nullptr), m_size(0), comp(comp), alloc(alloc) {}
    ~myavl() {
        clear();
    }
    void insert(const value_type &value) {
        root = insert_dfs(root, new node_type(value, alloc));
        ++m_size;
    }
    void insert(value_type &&value) {
        root = insert_dfs(root, new node_type(std::move(value), alloc));
        ++m_size;
    }
    void remove(const value_type &value) {
        root = remove_dfs(root, value);
        --m_size;
    }
    void clear() {
        remove_recursively(root);
        root = nullptr;
        m_size = 0;
    }
    bool find(const value_type &value) const {
        return find_dfs(root, value);
    }
protected:
    static void remove_recursively(node_type *node) {
        if (node) {
            remove_recursively(node->lchild);
            remove_recursively(node->rchild);
            delete node;
        }
    }
    node_type *insert_dfs(node_type *node, node_type *new_node) {
        if (!node)
            return new_node;
        else if (comp(*new_node->data, *node->data))
            node->lchild = insert_dfs(node->lchild, new_node);
        else
            node->rchild = insert_dfs(node->rchild, new_node);
        return adjust(node);
    }
    static std::size_t height(node_type *node) {
        return node ? node->height : 0;
    }
    static node_type *adjust(node_type *node) {
        std::size_t left = height(node->lchild), right = height(node->rchild);
        if (left == right + 2) {
            if (height(node->lchild->lchild) >= height(node->lchild->rchild))
                node = right_rotate(node);
            else
                node = left_right_rotate(node);
        } else if (right == left + 2) {
            if (height(node->rchild->rchild) >= height(node->rchild->lchild))
                node = left_rotate(node);
            else
                node = right_left_rotate(node);
        }
        node->height = mymax(height(node->lchild), height(node->rchild)) + 1;
        return node;
    }
    static node_type *left_rotate(node_type *p) {
        node_type *q = p->rchild;
        p->rchild = q->lchild;
        q->lchild = p;
        p->height = mymax(height(p->lchild), height(p->rchild)) + 1;
        q->height = mymax(height(q->lchild), height(q->rchild)) + 1;
        return q;
    }
    static node_type *right_rotate(node_type *p) {
        node_type *q = p->lchild;
        p->lchild = q->rchild;
        q->rchild = p;
        p->height = mymax(height(p->lchild), height(p->rchild)) + 1;
        q->height = mymax(height(q->lchild), height(q->rchild)) + 1;
        return q;
    }
    static node_type *right_left_rotate(node_type *node) {
        node->rchild = right_rotate(node->rchild);
        return left_rotate(node);
    }
    static node_type *left_right_rotate(node_type *node) {
        node->lchild = left_rotate(node->lchild);
        return right_rotate(node);
    }
    bool find_dfs(node_type *node, const value_type &value) {
        if (!node)
            return false;
        else if (comp(value, *node->data))
            return find_dfs(node->lchild, value);
        else if (comp(*node->data, value))
            return find_dfs(node->rchild, value);
        else
            return true;
    }
    node_type *remove_dfs(node_type *node, const value_type &value) {
        if (node == nullptr)
            return nullptr;
        if (comp(value, *node->data))
            node->lchild = remove_dfs(node->lchild, value);
        else if (comp(*node->data, value))
            node->rchild = remove_dfs(node->rchild, value);
        else if (node->rchild == nullptr) {
            node_type *temp = node->lchild;
            delete node;
            return temp;
        } else {
            node_type *temp = node->rchild;
            while (temp->lchild) temp = temp->lchild;
            node->data = temp->data;
            node->rchild = remove_dfs(node->rchild, *temp->data);
        }
        node = adjust(node);
        return node;
    }

private:
    node_type *root;
    std::size_t m_size;
    Compare comp;
    Allocator alloc;
};

template<typename Key, typename Compare, typename Allocator>
std::ostream &operator << (std::ostream &out, const myavl<Key, Compare, Allocator> &avl) {
    out << *avl.root;
    return out;
};

#endif //AVL_MYAVL_H
