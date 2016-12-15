//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYHASHSET_H
#define HTML_PARSER_MYHASHSET_H

#include <memory>      // for std::allocator
#include <cstdint>     // for std::uint8_t
#include <cstring>     // for std::memset
#include <initializer_list> // for std::initializer_list

#include "myfunctional.h"
#include "mypair.h"

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class myhashset;

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class myhashset_iterator {
public:
    typedef Key                                                             value_type;
    typedef myhashset<Key, Hash, KeyEqual, Allocator>                       hashset_type;
    typedef myhashset_iterator<value_type, Hash, KeyEqual, Allocator>       iterator;
    typedef myhashset_iterator<const value_type, Hash, KeyEqual, Allocator> const_iterator;

    friend hashset_type;
    friend const_iterator;

    myhashset_iterator()
            : items(NULL), states(NULL), capacity(0), index(0) {}
    myhashset_iterator(value_type *items, std::uint8_t *states, std::size_t capacity, std::size_t index)
            : items(items), states(states), capacity(capacity), index(index) {}
    myhashset_iterator(const iterator &other)
            : items(other.items), states(other.states), capacity(other.capacity), index(other.index) {}
    myhashset_iterator &operator = (const iterator &other) {
        items = other.items;
        states = other.states;
        capacity = other.capacity;
        index = other.index;
        return *this;
    }
    void swap(iterator &other) {
        value_type *temp_items = other.items;
        std::uint8_t *temp_states = other.states;
        std::size_t temp_capacity = other.capacity, temp_index = other.index;
        other.items = items;
        other.states = states;
        other.capacity = capacity;
        other.index = index;
        items = temp_items;
        states = temp_states;
        capacity = temp_capacity;
        index = temp_index;
    }
    value_type &operator * () const {
        return items[index];
    }
    value_type *operator -> () const {
        return items + index;
    }
    bool operator == (const const_iterator &other) const {
        return index == other.index;
    }
    bool operator != (const const_iterator &other) const {
        return !(*this == other);
    }
    iterator &operator ++ () {
        while (index < capacity && states[++index] != 1);
        return *this;
    }
    const_iterator operator ++ (int) {
        const_iterator temp(*this);
        ++(*this);
        return temp;
    }
    iterator &operator -- () {
        while (index > 0 && states[--index] != 1);
        return *this;
    }
    const_iterator operator -- (int) {
        const_iterator temp(*this);
        --(*this);
        return temp;
    }
private:
    value_type *items;
    std::uint8_t *states;
    std::size_t capacity, index;
};

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class myhashset_iterator<const Key, Hash, KeyEqual, Allocator> {
public:
    typedef const Key                                                value_type;
    typedef myhashset<Key, Hash, KeyEqual, Allocator>                hashset_type;
    typedef myhashset_iterator<Key, Hash, KeyEqual, Allocator>       iterator;
    typedef myhashset_iterator<const Key, Hash, KeyEqual, Allocator> const_iterator;

    friend hashset_type;
    friend iterator;

    myhashset_iterator()
            : items(NULL), states(NULL), capacity(0), index(0) {}
    myhashset_iterator(value_type *items, std::uint8_t *states, std::size_t capacity, std::size_t index)
            : items(items), states(states), capacity(capacity), index(index) {}
    myhashset_iterator(const iterator &other)
            : items(other.items), states(other.states), capacity(other.capacity), index(other.index) {}
    myhashset_iterator(const const_iterator &other)
            : items(other.items), states(other.states), capacity(other.capacity), index(other.index) {}
    myhashset_iterator &operator = (const const_iterator &other) {
        items = other.items;
        states = other.states;
        capacity = other.capacity;
        index = other.index;
    }
    void swap(iterator &other) {
        value_type *temp_items = other.items;
        std::uint8_t *temp_states = other.states;
        std::size_t temp_capacity = other.capacity, temp_index = other.index;
        other.items = items;
        other.states = states;
        other.capacity = capacity;
        other.index = index;
        items = temp_items;
        states = temp_states;
        capacity = temp_capacity;
        index = temp_index;
    }
    value_type &operator * () const {
        return items[index];
    }
    value_type *operator -> () const {
        return items + index;
    }
    bool operator == (const const_iterator &other) const {
        return index == other.index;
    }
    bool operator != (const const_iterator &other) const {
        return !(*this == other);
    }
    const_iterator &operator ++ () {
        while (index < capacity && states[++index] != 1);
        return *this;
    }
    const_iterator operator ++ (int) {
        const_iterator temp(*this);
        ++(*this);
        return temp;
    }
    const_iterator &operator -- () {
        while (index > 0 && states[--index] != 1);
        return *this;
    }
    const_iterator operator -- (int) {
        const_iterator temp(*this);
        --(*this);
        return temp;
    }
private:
    value_type *items;
    std::uint8_t *states;
    std::size_t capacity, index;
};

template<typename Key, typename Hash = myhash<Key>, typename KeyEqual = myequal_to<Key>, typename Allocator = std::allocator<Key> >
class myhashset {
public:
    typedef Key                                                             value_type;
    typedef myhashset<value_type, Hash, KeyEqual, Allocator>                hashset_type;
    typedef myhashset_iterator<value_type, Hash, KeyEqual, Allocator>       iterator;
    typedef myhashset_iterator<const value_type, Hash, KeyEqual, Allocator> const_iterator;

    explicit myhashset(const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            : m_size(0), m_capacity(0), m_begin_index(0), m_items(NULL), m_states(NULL), hash(hash), equal(equal), alloc(alloc) {}
    myhashset(const hashset_type &other)
            :m_size(0), m_capacity(0), m_begin_index(0), m_items(NULL), m_states(NULL), hash(other.hash), equal(other.equal), alloc(other.alloc) {
        insert(other.begin(), other.end());
    }
    hashset_type &operator = (const hashset_type &other) {
        insert(other.begin(), other.end());
        return *this;
    }
    template <typename InputIterator>
    myhashset(InputIterator first, InputIterator last, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            :m_size(0), m_capacity(0), m_begin_index(0), m_items(NULL), m_states(NULL), hash(hash), equal(equal), alloc(alloc) {
        insert(first, last);
    }
    myhashset(std::initializer_list<value_type> list, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            :m_size(0), m_capacity(0), m_begin_index(0), m_items(NULL), m_states(NULL), hash(hash), equal(equal), alloc(alloc) {
        insert(list.begin(), list.end());
    }
    virtual ~myhashset() {
        clear();
    }
    mypair<iterator, bool> insert(const value_type &x) {
        maybe_rehash();
        iterator iter = add_member(x);
        return mymake_pair(iter, iter != end());
    }
    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        while (first != last) {
            maybe_rehash();
            add_member(*first);
            ++first;
        }
    }
    void clear() {
        for (std::size_t i = 0; i < m_capacity; i++) {
            if (m_states[i] == 1)
                alloc.destroy(m_states + i);
            m_states[i] = 0;
        }
    }
    iterator erase(const_iterator iter) {
        alloc.destroy(m_states + iter.index);
        m_states[iter.index] = 2;
        --m_size;
        iterator temp_iter(m_items, m_states, m_capacity, iter.index);
        ++temp_iter;
        if (iter.index == m_begin_index)
            m_begin_index = temp_iter.index;
        return temp_iter;
    }
    std::size_t size() const {
        return m_size;
    }
    std::size_t capacity() const {
        return m_capacity;
    }
    bool empty() const {
        return size() == 0;
    }

    iterator begin() {
        return iterator(m_items, m_states, m_capacity, m_begin_index);
    }
    iterator end() {
        return iterator(m_items, m_states, m_capacity, m_capacity);
    }
    const_iterator begin() const {
        return const_iterator(m_items, m_states, m_capacity, m_begin_index);
    }
    const_iterator end() const {
        return const_iterator(m_items, m_states, m_capacity, m_capacity);
    }
    const_iterator cbegin() const {
        return const_iterator(m_items, m_states, m_capacity, m_begin_index);
    }
    const_iterator cend() const {
        return const_iterator(m_items, m_states, m_capacity, m_capacity);
    }
    iterator find(const value_type &x) {
        if (m_capacity == 0)
            return end();
        std::size_t value = hash_first(hash(x)), orig_value = value;
        while (m_states[value] == 1 || m_states[value] == 2) {
            if (m_states[value] == 1 && equal(m_items[value], x))
                return iterator(m_items, m_states, m_capacity, value);
            else {
                value = hash_next(value);
                if (value == orig_value)
                    break;
            }
        }
        return end();
    }
    void reserve(size_t new_capacity) {
        value_type *old_items = m_items;
        std::uint8_t *old_states = m_states;
        std::size_t old_capacity = m_capacity;
        m_size = 0;
        m_capacity = new_capacity;
        m_begin_index = m_capacity;
        m_items = alloc.allocate(m_capacity);
        m_states = new std::uint8_t[m_capacity];
        std::memset(m_states, 0, sizeof(std::uint8_t) * m_capacity);
        for (std::size_t i = 0; i < old_capacity; i++) {
            if (old_states[i] == 1) {
                add_member(old_items[i]);
                alloc.destroy(old_items + i);
            }
        }
        alloc.deallocate(old_items, old_capacity);
        delete []old_states;
    }

    const_iterator find(const value_type &x) const {
        if (m_capacity == 0)
            return end();
        std::size_t value = hash_first(hash(x)), orig_value = value;
        while (m_states[value] == 1 || m_states[value] == 2) {
            if (m_states[value] == 1 && equal(m_items[value], x))
                return iterator(m_items, m_states, m_capacity, value);
            else {
                value = hash_next(value);
                if (value == orig_value)
                    break;
            }
        }
        return cend();
    }
protected:
    void maybe_rehash() {
        if (m_size > m_capacity * 0.3)
            force_rehash();
    }

    void force_rehash() {
        value_type *old_items = m_items;
        std::uint8_t *old_states = m_states;
        std::size_t old_capacity = m_capacity;
        m_size = 0;
        m_capacity *= 2;
        m_begin_index = m_capacity;
        m_items = alloc.allocate(m_capacity);
        m_states = new std::uint8_t[m_capacity];
        std::memset(m_states, 0, sizeof(std::uint8_t) * m_capacity);
        for (std::size_t i = 0; i < old_capacity; i++) {
            if (old_states[i] == 1) {
                add_member(old_items[i]);
                alloc.destroy(old_items + i);
            }
        }
        alloc.deallocate(old_items, old_capacity);
        delete []old_states;
    }

    iterator add_member(const value_type &x) {
        if (m_capacity == 0) {
            const std::size_t INITIAL_SIZE = 16;
            m_items = alloc.allocate(INITIAL_SIZE);
            m_states = new std::uint8_t[INITIAL_SIZE];
            std::memset(m_states, 0, sizeof(std::uint8_t) * INITIAL_SIZE);
            m_size = 0;
            m_capacity = INITIAL_SIZE;
            m_begin_index = INITIAL_SIZE;
        }
        std::size_t value = hash_first(hash(x)), orig_value = value;
        while (m_states[value] == 1) {
            if (equal(m_items[value], x))
                return iterator(m_items, m_states, m_capacity, value);
            else {
                value = hash_next(value);
                if (value == orig_value) {
                    force_rehash();
                    value = hash_first(hash(x));
                    orig_value = value;
                }
            }
        }
        m_states[value] = 1;
        alloc.construct(m_items + value, x);
        if (value < m_begin_index)
            m_begin_index = value;
        ++m_size;
        return iterator(m_items, m_states, m_capacity, value);
    }

    std::size_t hash_first(std::size_t value) const {
        const std::size_t PRIME1 = 73;
        return PRIME1 * value % m_capacity;
    }

    std::size_t hash_next(std::size_t value) const {
        const std::size_t PRIME2 = 5009;
        return (value + PRIME2) % m_capacity;
    }


private:
    std::size_t m_size, m_capacity, m_begin_index;
    value_type *m_items;
    std::uint8_t *m_states; // 0: not allocated, 1: allocated, 2: removed

    Hash hash;
    KeyEqual equal;
    Allocator alloc;
};


#endif //HTML_PARSER_MYHASHSET_H
