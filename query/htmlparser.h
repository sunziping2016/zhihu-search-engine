//
// Created by sun on 11/13/16.
//

#ifndef HTML_PARSER_HTMLPARSER_H
#define HTML_PARSER_HTMLPARSER_H

#include "myhashmap.h"
#include "myhashset.h"
#include "mystring.h"

class html_node;

class html_selector {
public:
    html_selector(myu32string tag = "") : m_tag(tag) {}
    template<typename CharT>
    html_selector(const CharT * tag) : m_tag(tag) {}
    html_selector(const html_selector &other)
            : m_tag(other.m_tag), m_classes(other.m_classes), m_attrs(other.m_attrs) {}
    html_selector &operator = (const html_selector &other) {
        m_tag = other.m_tag;
        m_classes = other.m_classes;
        m_attrs = other.m_attrs;
        return *this;
    }
    html_selector &id(myu32string x) {
        m_attrs.push_back(mymake_pair("id", x));
        return *this;
    }
    html_selector &class_(myu32string x) {
        m_classes.push_back(x);
        return *this;
    }
    html_selector &attr(myu32string k, myu32string v) {
        m_attrs.push_back(mymake_pair(k, v));
        return *this;
    }

    bool match(const html_node *node) const;
private:
    myu32string m_tag;
    myvector<myu32string> m_classes;
    myvector<mypair<myu32string, myu32string> > m_attrs;
};

template<typename PNode>
class basic_html_result: public myvector<PNode> {
public:
    basic_html_result() {}
    basic_html_result(const basic_html_result &other) : myvector<PNode>(other) {}
    basic_html_result find_all(const html_selector &selector) const;
    myvector<myu32string> text() const;
    myvector<myu32string> html() const;
};

class html_node {
    friend class html_selector;
    friend std::size_t parse_node(const myu32string &html, std::size_t index, html_node &parent, html_node &node, int &open_num);
    friend std::ostream &operator << (std::ostream &out, const html_node &node);

public:
    enum html_node_type {DOCUMENT, TAG, TEXT, COMMENT};

    html_node() : m_type(DOCUMENT), m_parent(NULL) {}
    html_node(const myu32string &html) : m_type(DOCUMENT), m_parent(NULL) {
        parse(html);
    }
    html_node(const html_node &other)
            : m_type(other.m_type), m_name(other.m_name), m_attrs(other.m_attrs),
              m_parent(other.m_parent), m_children(other.m_children),
              m_text(other.m_text) {}
    html_node &operator = (const html_node &other) {
        m_type     = other.m_type;
        m_name     = other.m_name;
        m_attrs    = other.m_attrs;
        m_parent   = other.m_parent;
        m_children = other.m_children;
        m_text   = other.m_text;
        return *this;
    }
    ~html_node() {
        clear();
    }
    void clear();

    void parse(const myu32string &html);
    myu32string html() const;
    myu32string text() const;
    const html_node *find(const html_selector &selector) const;
    html_node *find(const html_selector &selector);
    basic_html_result<const html_node *> find_all(const html_selector &selector) const;
    basic_html_result<html_node *> find_all(const html_selector &selector);

    html_node_type type() const {
        return m_type;
    }
    const myu32string &name() const {
        return m_name;
    }
    const myhashmap<myu32string, myu32string> &attrs() const {
        return m_attrs;
    };
    const myhashset<myu32string> &classes() const {
        return m_classes;
    }
    html_node *parent() const {
        return m_parent;
    }
    myvector<html_node *>::const_iterator begin() const {
        return m_children.cbegin();
    };
    myvector<html_node *>::const_iterator end() const {
        return m_children.cend();
    };


private:
    html_node_type m_type;
    // Tag
    myu32string m_name;
    myhashmap<myu32string, myu32string> m_attrs;
    myhashset<myu32string> m_classes;
    html_node *m_parent;
    myvector<html_node *> m_children;
    // Text and comment
    myu32string m_text;
};

template<typename PNode>
basic_html_result<PNode> basic_html_result<PNode>::find_all(const html_selector &selector) const {
    basic_html_result result;
    for (typename myvector<PNode>::const_iterator iter = this->cbegin(); iter != this->cend(); ++iter)
        result += (*iter)->find_all(selector);
    return result;
}

template<typename PNode>
myvector<myu32string> basic_html_result<PNode>::text() const {
    myvector<myu32string> result;
    for (typename myvector<PNode>::const_iterator iter = this->cbegin(); iter != this->cend(); ++iter)
        result.push_back((*iter)->text());
    return result;
}

template<typename PNode>
myvector<myu32string> basic_html_result<PNode>::html() const {
    myvector<myu32string> result;
    for (typename myvector<PNode>::const_iterator iter = this->cbegin(); iter != this->cend(); ++iter)
        result.push_back((*iter)->html());
    return result;
}

// Debug
std::ostream &operator << (std::ostream &out, const html_node &node);

typedef html_node                            html_dom;
typedef basic_html_result<html_node *>       html_result;
typedef basic_html_result<const html_node *> const_html_result;

#endif //HTML_PARSER_HTMLPARSER_H
