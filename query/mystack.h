#ifndef HTML_PARSER_MYSTACK_H
#define HTML_PARSER_MYSTACK_H

#include "myvector.h"

template<class T, class Container = myvector<T> >
class mystack {
public:
    typedef typename Container::reference       reference;
    typedef typename Container::const_reference const_reference;

    mystack() = default;
    mystack(const mystack &other) = default;
    mystack &operator = (const mystack &other) = default;
    mystack(mystack &&other) = default;
    mystack &operator = (mystack &&other) = default;

    explicit mystack(const Container &container)
            : m_container(container) {}
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
    void push(T &&value) {
        m_container.push_back(std::move(value));
    }
    void pop() {
        m_container.pop_back();
    }

private:
    Container m_container;
};


#endif //HTML_PARSER_MYSTACK_H
