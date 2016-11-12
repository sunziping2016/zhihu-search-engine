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
    explicit myvector(const allocator_type &alloc = allocator_type())
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL), m_alloc(alloc) {}
    template <class InputIterator>
    myvector(InputIterator first, InputIterator last, const allocator_type &alloc = allocator_type())
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL), m_alloc(alloc) {
        assign(first, last);
    }
    myvector(const vector_type &x)
            : m_begin(NULL), m_end(NULL), m_end_cap(NULL), m_alloc(x.m_alloc) {
        assign(x.begin(), x.end());
    }
    myvector &operator = (const vector_type &x) {
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
        for (pointer temp = m_begin; temp != m_end; ++temp)
            m_alloc.destroy(temp);
        m_alloc.deallocate(m_begin, m_end_cap - m_begin);
        m_begin = m_end = m_end_cap = NULL;
    }
    iterator       begin()  { return m_begin; }
    const_iterator begin()  const { return m_begin; }
    iterator       end()    { return m_end; }
    const_iterator end()    const { return m_end; }
    const_iterator cbegin() const { return m_begin; }
    const_iterator cend()   const { return m_end; }

    std::size_t size() const {
        if (m_begin == NULL)
            return 0;
        return m_end - m_begin;
    }
    std::size_t capacity() const {
        if (m_begin == NULL)
            return 0;
        return m_end_cap - m_begin;
    }
    bool empty() const {
        return size() == 0;
    }

    reference front() {
        return m_begin[0];
    }
    const_reference front() const {
        return m_begin[0];
    }
    reference back() {
        return m_end[-1];
    }
    const_reference back() const {
        return m_begin[-1];
    }

    reference operator [] (std::size_t n) {
        return m_begin[n];
    }
    const_reference operator [] (std::size_t n) const {
        return m_begin[n];
    }
    vector_type &operator += (const vector_type &x) {
        insert(end(), x.begin(), x.end());
        return *this;
    }
    bool operator == (const vector_type &x) const {
        const_iterator iter1, iter2;
        for (iter1 = cbegin(), iter2 = x.cbegin(); iter1 != cend() && iter2 != x.cend() && *iter1 == *iter2; ++iter1, ++iter2);
        if (iter1 == cend() && iter2 == x.cend())
            return true;
        return false;
    }
    bool operator != (const vector_type &x) const {
        return (*this == x);
    }

    vector_type operator + (const vector_type &x) const {
        vector_type temp(*this);
        temp.insert(temp.end(), x.begin(), x.end());
        return temp;
    }

    iterator insert(const_iterator position, const value_type &x) {
        const std::size_t INITIAL_SIZE = 16;
        std::size_t pos = position - m_begin;
        if (m_begin == NULL) {
            m_end = m_begin = m_alloc.allocate(INITIAL_SIZE);
            m_end_cap = m_begin + INITIAL_SIZE;
        }
        if (m_end == m_end_cap) {
            std::size_t length = size();
            pointer new_begin = m_alloc.allocate(length * 2);
            for (std::size_t i = 0; i < length; ++i) {
                m_alloc.construct(new_begin + i, m_begin[i]);
                m_alloc.destroy(m_begin + i);
            }
            m_alloc.deallocate(m_begin, length);
            m_begin = new_begin;
            m_end = m_begin + length;
            m_end_cap = m_begin + length * 2;
        }
        for (pointer temp = m_end; temp != m_begin + pos; --temp) {
            m_alloc.construct(temp, *(temp - 1));
            m_alloc.destroy(temp - 1);
        }
        m_alloc.construct(m_begin + pos, x);
        ++m_end;
        return m_begin + pos;
    }
    template <class InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last) {
        std::size_t pos = position - m_begin, i = pos;
        for(; first != last; ++first, ++i)
            insert(m_begin + i, *first);
        return m_begin + pos;
    }

    void push_back(const value_type &u) {
        insert(end(), u);
    }
    void pop_back() {
        erase(end() - 1);
    }

    iterator erase(const_iterator position) {
        m_alloc.destroy(position);
        --m_end;
        for (pointer temp = m_begin + (position - m_begin); temp != m_end; ++temp) {
            m_alloc.construct(temp, *(temp + 1));
            m_alloc.destroy(temp + 1);
        }
        return m_begin + (position - m_begin);
    }
    iterator erase(const_iterator first, const_iterator last) {
        std::size_t n = last - first;
        for (std::size_t i = 0; i < n; ++i)
            erase(first);
        return m_begin + (first - m_begin);
    }

private:
    pointer m_begin, m_end, m_end_cap;
    allocator_type m_alloc;
};


template<typename CharT, typename T, typename Allocator>
std::basic_ostream<CharT> &operator << (std::basic_ostream<CharT> &out, const myvector<T, Allocator> &x)
{
    bool first = true;
    out << out.widen('[');
    for (typename myvector<T, Allocator>::const_iterator iter = x.cbegin(); iter != x.cend(); ++iter) {
        if (first) {
            out << *iter;
            first = false;
        }
        else
            out << out.widen(',') << out.widen(' ') << *iter;
    }
    return out << out.widen(']');
};



#endif //MYVECTOR_MYVECTOR_H
