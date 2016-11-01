//
// Created by sun on 10/24/16.
//

#ifndef HTML_PARSER_MYHASHSET_H
#define HTML_PARSER_MYHASHSET_H

#include <memory>      // for std::allocator
#include <functional>  // for std::equal_to

template<typename T, typename Hash, typename Allocator>
class hashset;

template<typename T, typename Hash, typename Allocator>
class hashset_iter
{
public:
    typedef T                                value_type;
    typedef hashset_iter<T, Hash, Allocator> iterator;
    typedef hashset<T, Hash, Allocator> hashset_type;

    hashset_iter(hashset<T, Hash, Allocator> *set, std::size_t index = 0)
            : set(set),
              index(index)
    {
        assert(set->items[index] == 0 || set->items[index] == 1)
    }
    hashset_iter(const iterator &other): set(other.set), index(other.index) {}
    hashset_iter &operator = (const iterator &other)
    {
        index = other.index;
        return *this;
    }

    const void *value() const
    {
        return (const void *)set.items[index];
    }
    bool has_next() const
    {
        if (set.nitems == 0 || index == set.capacity())
            return false;
        size_t ii = index;
        while (ii < set.capacity()) {
            if (set.items[ii] != 0 && set.items[ii] != 1)
                return true;
            ++ii;
        }
        return false;
    }

    iterator &operator ++ ()
    {
        if (index == set->capacity())
            return;
        do {
            ++index;
        } while ((set.items[index] == 0 || set.items[index] == 1) && index < set.capacity());
        return *this;
    }
private:
    hashset<T, Hash, Allocator> *set;
    std::size_t index;
};

constexpr unsigned int hashset::PRIME1 = 73;
constexpr unsigned int hashset::PRIME2 = 5009;

class hashset
{
    friend class hashset_iter;
public:
    hashset()
            : nbits(3),
              items((uintptr_t *)calloc(capacity(), sizeof(uintptr_t))),
              nitems(0)
    {}
    ~hashset()
    {
        free(items);
    }

    void add(const void *item)
    {
        add_member(item);
        maybe_rehash();
    }
    void remove(const void *item)
    {
        uintptr_t value = (uintptr_t)item;
        size_t ii = hash(value);
        while (items[ii] != 0) {
            if (items[ii] == value) {
                items[ii] = 1;
                nitems--;
                return;
            }
            else
                ii = next_hash(ii);
        }
    }

    void clear()
    {
        free(items);
        nbits = 3;
        items = (uintptr_t *)calloc(capacity(), sizeof(uintptr_t));
        nitems = 0;
    }

    bool contains(const void *item) const
    {
        size_t value = (size_t)item;
        if (value == 0 || value == 1)
            return false;
        size_t ii = hash(value);
        while (items[ii] != 0) {
            if (items[ii] == value)
                return true;
            else
                ii = next_hash(ii);
        }
        return false;
    }

    hashset_iter iter() const
    {
        return hashset_iter(*this);
    }
protected:
    std::size_t capacity() const
    {
        return (std::size_t) (1u << nbits);
    }
    std::size_t mask() const
    {
        return (std::size_t) ((1u << nbits) - 1);
    }
    std::size_t hash(std::uintptr_t value) const
    {
        return mask() & (PRIME1 * value);
    }
    std::size_t next_hash(std::size_t value) const
    {
        return mask() & (value + PRIME2);
    }
    void maybe_rehash()
    {
        if (nitems > (size_t)(capacity() * 0.6)) {
            uintptr_t *old_items;
            size_t old_capacity;
            old_items = items;
            old_capacity = capacity();
            nbits++;
            items = (uintptr_t *)calloc(capacity(), sizeof(uintptr_t));
            nitems = 0;
            for (size_t ii = 0; ii < old_capacity; ii++)
                add_member((const void *)old_items[ii]);
            free(old_items);
        }
    }

    void add_member(const void *item)
    {
        uintptr_t value = (uintptr_t)item;
        if (value == 0 || value == 1)
            return;
        size_t ii = hash(value);
        while (items[ii] != 0 && items[ii] != 1) {
            if (items[ii] == value)
                return;
            else
                ii = next_hash(ii);
        }
        nitems++;
        items[ii] = value;
    }

private:
    static const unsigned int PRIME1;
    static const unsigned int PRIME2;

    std::size_t nbits;
    std::uintptr_t *items;
    std::size_t nitems;
};

#endif //HTML_PARSER_MYHASHSET_H
