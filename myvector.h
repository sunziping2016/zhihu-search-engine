//
// Created by sun on 8/20/16.
//

#ifndef MYVECTOR_MYVECTOR_H
#define MYVECTOR_MYVECTOR_H

#include <memory>  // for std::allocator

template<class T, class Allocator = std::allocator<T> >
class myvector {
public:
    typedef T                                        value_type;
    typedef Allocator                                allocator_type;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef pointer                                  iterator;
    typedef const_pointer                            const_iterator;
    typedef myvector<T, Allocator>                   vector_type;
    myvector()
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL) {}
    template <class InputIterator>
    myvector(InputIterator first, InputIterator last)
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL) {
        assign(first, last);
    }
    myvector(const vector_type &x)
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL) {
        assign(x.begin(), x.end());
    }
    myvector& operator = (const myvector &x) {
        assign(x.begin(), x.end());
    }
    ~myvector() {
        clear();
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last) {
        clear();
        for(; first != last; ++first)
            push_back(*first);
    }
    void clear() {
        Allocator a;
        for (pointer temp = m_begin; temp != m_end; ++temp)
            a.destroy(temp);
        a.deallocate(m_begin, m_end_cap - m_begin);
        m_begin = m_end = m_end_cap = NULL;
    }
    iterator       begin()  { return m_begin; }
    const_iterator begin()  const { return m_begin; }
    iterator       end()    { return m_end; }
    const_iterator end()    const { return m_end; }
    const_iterator cbegin() const { return m_begin; }
    const_iterator cend()   const { return m_end; }

    size_t size() const {
        if (m_begin == NULL)
            return 0;
        return m_end - m_begin;
    }
    size_t capacity() const {
        if (m_begin == NULL)
            return 0;
        return m_end_cap - m_begin;
    }
    bool empty() const {
        return size() == 0;
    }

    reference operator[](size_t n) {
        return m_begin[n];
    }
    const_reference operator[](size_t n) const {
        return m_begin[n];
    }

    iterator insert(const_iterator position, const value_type &x) {
        const size_t INITIAL_SIZE = 16;
        size_t pos = position - m_begin;
        Allocator a;
        if (m_begin == NULL) {
            m_end = m_begin = a.allocate(INITIAL_SIZE);
            m_end_cap = m_begin + INITIAL_SIZE;
        }
        if (m_end == m_end_cap) {
            size_t length = size();
            pointer new_begin = a.allocate(length * 2);
            for (size_t i = 0; i < length; ++i) {
                a.construct(new_begin + i, m_begin[i]);
                a.destroy(m_begin + i);
            }
            a.deallocate(m_begin, length);
            m_begin = new_begin;
            m_end = m_begin + length;
            m_end_cap = m_begin + length * 2;
        }
        for (pointer temp = m_end; temp != m_begin + pos; --temp) {
            a.construct(temp, *(temp - 1));
            a.destroy(temp - 1);
        }
        a.construct(m_begin + pos, x);
        ++m_end;
        return m_begin + pos;
    }
    template <class InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last) {
        size_t pos = position - m_begin, i = pos;
        for(; first != last; ++first, ++i)
            insert(m_begin + i, *first);
        return m_begin + pos;
    }

    void push_back(const value_type &u) {
        insert(end(), u);
    }

    iterator erase(const_iterator position) {
        Allocator a;
        a.destroy(position);
        --m_end;
        for (pointer temp = m_begin + (position - m_begin); temp != m_end; ++temp) {
            a.construct(temp, *(temp + 1));
            a.destroy(temp + 1);
        }
    }
    iterator erase(const_iterator first, const_iterator last) {
        size_t n = last - first;
        for (size_t i = 0; i < n; ++i)
            erase(first);
    }

private:
    pointer m_begin, m_end, m_end_cap;
};

#endif //MYVECTOR_MYVECTOR_H
