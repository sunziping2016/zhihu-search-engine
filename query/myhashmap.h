//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYHASHMAP_H
#define HTML_PARSER_MYHASHMAP_H

#include "myhashset.h"

template<typename Key, typename T, typename Hash>
class myhashmap_hash {
public:
    myhashmap_hash(const Hash &hash) : hash(hash) {}
    size_t operator () (const mypair<const Key, T> &x) const {
        return hash(x.first);
    }
private:
    Hash hash;
};

template<typename Key, typename T, typename KeyEqual>
class myhashmap_equal {
public:
    myhashmap_equal(const KeyEqual &equal) : equal(equal) {}
    bool operator () (const mypair<const Key, T> &lhs, const mypair<const Key, T> &rhs) const {
        return equal(lhs.first, rhs.first);
    }
private:
    KeyEqual equal;
};

template<typename Key, typename T, typename Hash = myhash<Key>, typename KeyEqual = myequal_to<Key>,
        typename Allocator = std::allocator<mypair<const Key, T> > >
class myhashmap: public myhashset<mypair<const Key, T>, myhashmap_hash<Key, T, Hash>,
        myhashmap_equal<Key, T, KeyEqual>, Allocator>  {
public:
    typedef myhashset<mypair<const Key, T>, myhashmap_hash<Key, T, Hash>,
            myhashmap_equal<Key, T, KeyEqual>, Allocator> hashset_type;
    typedef typename hashset_type::iterator       iterator;
    typedef typename hashset_type::const_iterator const_iterator;

    explicit myhashmap(const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            : hashset_type(myhashmap_hash<Key, T, Hash>(hash), myhashmap_equal<Key, T, KeyEqual>(equal), alloc) {}
    template <typename InputIterator>
    myhashmap(InputIterator first, InputIterator last, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            : hashset_type(first, last, myhashmap_hash<Key, T, Hash>(hash), myhashmap_equal<Key, T, KeyEqual>(equal), alloc) {}
    myhashmap(std::initializer_list<typename hashset_type::value_type> list, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
            : hashset_type(list.begin(), list.end(), myhashmap_hash<Key, T, Hash>(hash), myhashmap_equal<Key, T, KeyEqual>(equal), alloc) {}
    myhashmap(const myhashmap &other) = default;
    myhashmap &operator = (const myhashmap &other) = default;
    myhashmap(myhashmap &&other) = default;
    myhashmap &operator = (myhashmap &&other) = default;


    iterator find(const Key &key) {
        return hashset_type::find(mypair<const Key, T>(key, T()));
    }
    const_iterator find(const Key& key) const {
        return hashset_type::find(mypair<const Key, T>(key, T()));
    }

    T &operator[](const Key& key) {
        return find(key)->second;
    }
};

#endif //HTML_PARSER_MYHASHMAP_H
