#ifndef MY_LIST_H_INCLUDE
#define MY_LIST_H_INCLUDE

#include <memory>      // for std::allocator

#include "myfunctional.h"

template<typename T, class Allocator>
class mylist_node {
public:
    mylist_node<T, Allocator> *prev, *next;
    T *data;
    Allocator alloc;

    explicit mylist_node(const Allocator &a) : prev(NULL), next(NULL), data(NULL), alloc(a) {}
    mylist_node(const T &orig, const Allocator &a) : prev(NULL), next(NULL), alloc(a) {
        data = alloc.allocate(1);
        alloc.construct(data, orig);
    }
    ~mylist_node() {
        if (data)
            alloc.destroy(data);
    }
private:
    mylist_node(const mylist_node &orig);
    mylist_node & operator = (const mylist_node &orig);
};

template<typename T, class Allocator>
class mylist;

template<typename T, class Allocator>
class mylist_iterator {
public:
    typedef T                                   value_type;
    typedef mylist_node<T, Allocator>           node_type;
    typedef mylist_iterator<T, Allocator>       iterator;
    typedef mylist_iterator<const T, Allocator> const_iterator;
    typedef mylist<T, Allocator>                list_type;

    friend list_type;
    friend const_iterator;

    mylist_iterator() : node(NULL) {}
    mylist_iterator(node_type *node) : node(node) {}
    mylist_iterator(const iterator &other) : node(other.node) {}
    mylist_iterator &operator = (const iterator &other) {
        node = other.node;
        return *this;
    }
    void swap(iterator &other) {
        node_type *temp = other.node;
        other.node = node;
        node = temp;
    }
    value_type &operator * () const {
        return *node->data;
    }
    value_type *operator -> () const {
        return node->data;
    }
    bool operator == (const const_iterator &other) const {
        return node == other.node;
    }
    bool operator != (const const_iterator &other) const {
        return !(*this == other);
    }
    iterator &operator ++ () {
        node = node->next;
        return *this;
    }
    const_iterator operator ++ (int) {
        const_iterator temp(*this);
        node = node->next;
        return temp;
    }
    iterator &operator -- () {
        node = node->prev;
        return *this;
    }
    const_iterator operator -- (int) {
        const_iterator temp(*this);
        node = node->prev;
        return temp;
    }
private:
    node_type *node;
};


template<typename T, class Allocator>
class mylist_iterator<const T, Allocator> {
public:
    typedef const T                                value_type;
    typedef mylist_node<T, Allocator>              node_type;
    typedef mylist_iterator<T, Allocator>          iterator;
    typedef mylist_iterator<const T, Allocator>    const_iterator;
    typedef mylist<T, Allocator>                   list_type;

    friend list_type;
    friend iterator;

    mylist_iterator() : node(NULL) {}
    mylist_iterator(node_type *node) : node(node) {}
    mylist_iterator(const iterator &other) : node(other.node) {}
    mylist_iterator(const const_iterator &other) : node(other.node) {}
    mylist_iterator &operator = (const const_iterator &other) {
        node = other.node;
        return *this;
    }
    void swap(const_iterator &other) {
        node_type *temp = other.node;
        other.node = node;
        node = temp;
    }
    value_type &operator * () const {
        return *node->data;
    }
    value_type *operator -> () const {
        return node->data;
    }

    bool operator == (const const_iterator &other) const {
        return node == other.node;
    }
    bool operator != (const const_iterator &other) const {
        return !(*this == other);
    }

    const_iterator &operator ++ () {
        node = node->next;
        return *this;
    }
    const_iterator operator ++ (int) {
        const_iterator temp(*this);
        node = node->next;
        return temp;
    }
    const_iterator &operator -- () {
        node = node->prev;
        return *this;
    }
    const_iterator operator -- (int) {
        const_iterator temp(*this);
        node = node->prev;
        return temp;
    }
private:
    node_type *node;
};

template<typename T, class Allocator = std::allocator<T> >
class mylist {
public:
    typedef T                                            value_type;
    typedef mylist<T, Allocator>                         list_type;
    typedef mylist_node<value_type, Allocator>           node_type;
    typedef mylist_iterator<value_type, Allocator>       iterator;
    typedef mylist_iterator<const value_type, Allocator> const_iterator;
    typedef typename Allocator::reference                reference;
    typedef typename Allocator::const_reference          const_reference;

    explicit mylist(const Allocator &alloc = Allocator())
            : alloc(alloc), m_size(0) {
        root = new node_type(alloc);
        root->next = root->prev = root;
    }
    template <typename InputIterator>
    mylist(InputIterator first, InputIterator last, const Allocator &alloc = Allocator())
            : alloc(alloc), m_size(0) {
        root = new node_type(alloc);
        root->next = root->prev = root;
        assign(first, last);
    }
    mylist(const list_type &orig)
            : alloc(orig.alloc), m_size(0) {
        root = new node_type(alloc);
        root->next = root->prev = root;
        assign(orig.begin(), orig.end());
    }
    mylist & operator = (const list_type &orig) {
        assign(orig.begin(), orig.end());
    }
    virtual ~mylist() {
        clear();
        delete root;
    }
    template <class InputIterator>
    void assign(InputIterator first, InputIterator last) {
        clear();
        root->next = root->prev = root;
        for(; first != last; ++first)
            push_back(*first);
    }
    void clear() {
        while(root->next != root)
            remove(root->next);
    }
    iterator erase(const_iterator iter) {
        iterator temp_iter(iter.node->next);
        remove(iter.node);
        return temp_iter;
    }
    void push_back(const value_type &value) {
        insert(root, new node_type(value, alloc));
    }
    void push_front(const value_type &value) {
        insert(root->next, new node_type(value, alloc));
    }
    void pop_back() {
        remove(root);
    }
    void pop_front() {
        remove(root->next);
    }
    iterator insert(iterator iter, const value_type &value) {
        node_type *node = new node_type(value, alloc);
        insert(iter->node, node);
        return iterator(node);
    }
    iterator begin() {
        return iterator(root->next);
    }
    iterator end() {
        return iterator(root);
    }
    const_iterator begin() const {
        return const_iterator(root->next);
    }
    const_iterator end() const {
        return const_iterator(root);
    }
    const_iterator cbegin() const {
        return const_iterator(root->next);
    }
    const_iterator cend() const {
        return const_iterator(root);
    }
    reference front() {
        return *root->next->data;
    }
    const_reference front() const {
        return *root->next->data;
    }
    reference back() {
        return *root->prev->data;
    }
    const_reference back() const {
        return *root->prev->data;
    }
    std::size_t size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
    template<typename Func = myless<value_type> >
    void sort(Func func = Func()) {
        node_type *temp;
        for (node_type *outer = root; outer->prev != root->next; outer = outer->prev)
            for (node_type *inner = root->next; inner->next != outer; inner = inner->next) {
                if (!func(*inner->data, *inner->next->data)) {
                    temp = inner->next;
                    inner->prev->next = temp;
                    temp->next->prev = inner;
                    inner->next = temp->next;
                    temp->prev = inner->prev;
                    inner->prev = temp;
                    temp->next = inner;
                    inner = temp;
                }
            }
    }
protected:
    void remove(node_type *node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        --m_size;
    }
    void insert(node_type *node, node_type *newnode) {
        node->prev->next = newnode;
        newnode->prev = node->prev;
        node->prev = newnode;
        newnode->next = node;
        ++m_size;
    }
private:
    node_type *root;
    Allocator alloc;
    std::size_t m_size;
};
#endif
