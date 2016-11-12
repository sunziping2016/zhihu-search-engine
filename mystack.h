#ifndef HTML_PARSER_MYSTACK_H
#define HTML_PARSER_MYSTACK_H

#include "myvector.h"

template<class T, class Container = myvector<T> >
class mystack {
public:
    typedef typename Container::reference       reference;
    typedef typename Container::const_reference const_reference;

    explicit mystack(const Container &container = Container())
            : m_container(container) {}
    mystack(const mystack &other)
            : m_container(other.m_container) {}
    mystack &operator = (const mystack &other) {
        m_container = other.m_container;
        return *this;
    }
    reference top() {
        return m_container.back();
    }
    const_reference top() const {
        return m_container.back();
    }
    bool empty() const {
        return m_container.empty();
    }
    std::size_t size() const {
        return m_container.size();
    }
    void push(const T &value) {
        m_container.push_back(value);
    }
    void pop() {
        m_container.pop_back();
    }

private:
    Container m_container;
};


#endif //HTML_PARSER_MYSTACK_H
