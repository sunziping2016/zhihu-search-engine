//
// Created by sun on 11/13/16.
//
#include <cwctype>
#include <stdexcept>

#include "htmlparser.h"
#include "mystack.h"
#include "mycodecvt.h"
#include "htmldata.h"

using namespace std;

static inline size_t skip_whitespace(const myu32string &html, size_t index) {
    while (index != html.size() && iswspace(html[index]))
        ++index;
    return index;
}

static inline bool starts_with(const myu32string &html, size_t index, const myu32string &text) {
    size_t index2;
    for (index2 = 0; index < html.size() && index2 < text.size() && html[index] == text[index2]; ++index, ++index2);
    return index2 == text.size();
}


static inline bool istarts_with(const myu32string &html, size_t index, const myu32string &text) {
    size_t index2;
    for (index2 = 0; index < html.size() && index2 < text.size() && towlower(html[index]) == towlower(text[index2]); ++index, ++index2);
    return index2 == text.size();
}


static bool is_void_element(const myu32string &name) {
    static myhashset<myu32string> elements;
    static bool init = false;
    if (!init) {
        elements.insert("!doctype");
        for (size_t i = 0; i < void_elements_n; ++i)
            elements.insert(void_elements[i]);
        init = true;
    }
    return elements.find(name) != elements.end();
}

static inline bool no_escape_element(const myu32string &name) {
    return name == "script" || name == "style";
}

static myu32string unescape_html(const myu32string &html, size_t begin, size_t end) {
    static myhashmap<myu32string, char32_t> elements;
    static bool init = false;
    static size_t max_length = 0;
    if (!init) {
        for (size_t i = 0; i < escape_n; ++i) {
            elements.insert(mymake_pair(escape_string[i], escape_char[i]));
            size_t length = 0;
            while (escape_string[i][length]) ++length;
            if (length > max_length)
                max_length = length;
        }
        init = true;
    }

    myu32string result;
    while (begin != end) {
        if (starts_with(html, begin, "&#")) {
            size_t orig_begin = begin;
            begin += 2;
            if (begin == end || !iswdigit(html[begin])) {
                result.push_back(html[orig_begin]);
                begin = orig_begin + 1;
                continue;
            }
            char32_t ch = 0;
            while (begin != end && iswdigit(html[begin])) {
                ch = ch * 10 + (html[begin] - '\0');
                ++begin;
            }
            if (begin == end || html[begin] != ';') {
                result.push_back(html[orig_begin]);
                begin = orig_begin + 1;
                continue;
            }
            ++begin;
        } else if (html[begin] == '&') {
            size_t orig_begin = begin;
            ++begin;
            size_t new_end = begin;
            while (new_end != end && html[new_end] != ';')
                ++new_end;
            if (new_end == end) {
                result.push_back(html[orig_begin]);
                begin = orig_begin + 1;
                continue;
            }
            myhashmap<myu32string, char32_t>::const_iterator x = elements.find(html.substr(begin, new_end - begin));
            if (x == elements.cend()) {
                result.push_back(html[orig_begin]);
                begin = orig_begin + 1;
                continue;
            }
            result.push_back(x->second);
            begin = new_end + 1;
        } else {
            result.push_back(html[begin]);
            ++begin;
        }
    }
    return result;
}


size_t parse_node(const myu32string &html, size_t index, html_node &parent, html_node &node, int &open_num) {
    size_t end_index;
    bool no_escape = no_escape_element(parent.name);
    if (starts_with(html, index, "<!--") && !no_escape) { // comment
        node.type = html_node::COMMENT;
        end_index = index += 4;
        while (end_index != html.size() && !starts_with(html, end_index, "-->"))
            ++end_index;
        if (end_index == html.size())
            throw runtime_error("Unexpected ending in a comment");
        node.m_text = html.substr(index, end_index - index);
        open_num = 0;
        return skip_whitespace(html, end_index + 3);
    } else if (starts_with(html, index, "</")) { // close tag
        node.type = html_node::TAG;
        end_index = index = skip_whitespace(html, index + 2);
        while (end_index != html.size() && html[end_index] != '>' && !iswspace(html[end_index]))
            ++end_index;
        if (index == end_index)
            throw runtime_error("Unnamed close tag");
        if (end_index == html.size())
            throw runtime_error("Unexpected ending in a close tag");
        node.name = html.substr(index, end_index - index);
        index = skip_whitespace(html, end_index);
        if (index == html.size())
            throw runtime_error("Unexpected ending in a close tag");
        if (html[index] != '>')
            throw runtime_error("Unexpected character in a close tag");
        open_num = -1;
        return skip_whitespace(html, index + 1);
    } else if (no_escape) { // text
        node.type = html_node::TEXT;
        end_index = index;
        myu32string close_tag = "</" + parent.name + ">";
        while (end_index != html.size() && !starts_with(html, end_index, close_tag))
            ++end_index;
        size_t temp_index = end_index;
        while (iswspace(html[temp_index - 1]))
            --temp_index;
        node.m_text = html.substr(index, temp_index - index);
        open_num = 0;
        return end_index;

    } else if (html[index] == '<') { // open tag
        node.type = html_node::TAG;
        end_index = index = skip_whitespace(html, index + 1);
        while (end_index != html.size() && html[end_index] != '>' && !iswspace(html[end_index]) && !starts_with(html, end_index, "/>"))
            ++end_index;
        if (end_index == html.size())
            throw runtime_error("Unexpected ending in an open tag");
        node.name = html.substr(index, end_index - index);
        index = skip_whitespace(html, end_index);
        myu32string name, value;
        while (index != html.size() && html[index] != '>' && !starts_with(html, index, "/>")) {
            end_index = index;
            while (end_index != html.size() && html[end_index] != '>' && html[end_index] != '=' && !iswspace(html[end_index]) && !starts_with(html, end_index, "/>"))
                ++end_index;
            if (end_index == html.size())
                throw runtime_error("Unexpected ending in an open tag");
            if (index == end_index)
                throw runtime_error("Unnamed tag's attribute");
            name = html.substr(index, end_index - index);
            if (html[end_index] == '=') {
                index = skip_whitespace(html, end_index + 1);
                if (index == html.size())
                    throw runtime_error("Unexpected ending in an open tag");
                if (html[index] == '\"') {
                    end_index = index + 1;
                    while(end_index != html.size() && html[end_index] != '\"')
                        ++end_index;
                    if (end_index == html.size())
                        throw runtime_error("Unexpected ending in an open tag's attribute value");
                    value = unescape_html(html, index + 1, end_index);
                    index = end_index + 1;
                } else {
                    end_index = index;
                    while(end_index != html.size() && html[end_index] != '>' && !iswspace(html[end_index]) && !starts_with(html, end_index, "/>"))
                        ++end_index;
                    if (end_index == html.size())
                        throw runtime_error("Unexpected ending in an open tag");
                    value = unescape_html(html, index, end_index);
                    index = end_index;
                }
            } else {
                value = "";
                index = end_index;
            }
            if (name == "class") {
                size_t p = 0, q;
                while (p != value.size()) {
                    q = p;
                    while (q != value.size() && !iswspace(value[q]))
                        ++q;
                    node.classes.insert(value.substr(p, q - p));
                    p = q;
                    while (p != value.size() && iswspace(value[p]))
                        ++p;
                }
            }
            node.attrs.insert(mymake_pair(name, value));
            index = skip_whitespace(html, index);
        }
        if (index == html.size())
            throw runtime_error("Unexpected ending in an open tag");
        if (html[index] == '>') {
            if (is_void_element(node.name))
                open_num = 0;
            else
                open_num = 1;
            return skip_whitespace(html, index + 1);
        } else {
            open_num = 0;
            return skip_whitespace(html, index + 2);
        }
    } else {
        node.type = html_node::TEXT;
        end_index = index;
        while (end_index != html.size() && html[end_index] != '<')
            ++end_index;
        size_t temp_index = end_index;
        while (iswspace(html[temp_index - 1]))
            --temp_index;
        node.m_text = unescape_html(html, index, temp_index);
        open_num = 0;
        return end_index;
    }
}


void html_node::parse(const myu32string &html) {
    size_t index = 0;

    mystack<html_node *> nodes_stack;
    myvector<html_node *> nodes;
    html_node *node = NULL;

    try {
        nodes_stack.push(this);
        index = skip_whitespace(html, index);
        while (index != html.size()) {
            node = new html_node;
            int open_num = 0;
            index = parse_node(html, index, *nodes_stack.top(), *node, open_num);
            if (open_num == 1 || open_num == 0) {
                node->parent = nodes_stack.top();
                node->parent->children.push_back(node);
                if (open_num == 1)
                    nodes_stack.push(node);
                nodes.push_back(node);
                node = NULL;
            } else {
                // open_num == -1
                if (nodes_stack.size() < 2)
                    throw runtime_error("Too many close tags");
                if (nodes_stack.top()->name != node->name)
                    throw runtime_error("Unmatched close tag");
                nodes_stack.pop();
                delete node;
                node = NULL;
            }
        }
        if (nodes_stack.size() != 1)
            throw runtime_error("Too many open tags");
    } catch (...) {
        for (myvector<html_node *>::const_iterator iter = nodes.cbegin(); iter != nodes.cend(); ++iter) {
            (*iter)->children.clear();
            delete *iter;
        }
        if (node) {
            node->children.clear();
            delete node;
        }
        throw;
    }
}

ostream &operator << (ostream &out, const html_node &node) {
    const char *type_name[4] {
            "DOCUMENT",
            "TAG",
            "TEXT",
            "COMMENT"
    };
    out << "{ type: " << type_name[node.type];
    if (node.type == html_node::TAG) {
        out << ", name: " << utf32_to_utf8(node.name);
        if (!node.attrs.empty()) {
            out << ", attr: { ";
            for (myhashmap<myu32string, myu32string>::const_iterator iter = node.attrs.cbegin();
                 iter != node.attrs.cend(); ++iter) {
                if (iter != node.attrs.cbegin())
                    out << ", ";
                out << utf32_to_utf8(iter->first) << " : " << utf32_to_utf8(iter->second);
            }
            out << " }";
        }
    }

    if (node.type == html_node::DOCUMENT || node.type == html_node::TAG && !node.children.empty()) {
        out << ", child: [ ";
        for (size_t i = 0; i < node.children.size(); ++i) {
            if (i != 0)
                out << ", ";
            out << *node.children[i];
        }
        out << " ]";
    }
    if (node.type == html_node::COMMENT || node.type == html_node::TEXT)
        out << ", m_text: " << utf32_to_utf8(node.m_text);
    out << " }";
    return out;
}

myu32string html_node::html() const {
    if (type == COMMENT)
        return "<!--" + m_text  + "-->";
    if (type == TEXT)
        return m_text;
    myu32string result;
    if (type == TAG) {
        result += "<";
        result += name;
        for (myhashmap<myu32string, myu32string>::const_iterator iter = attrs.cbegin(); iter != attrs.cend(); ++iter) {
            result += " ";
            result += iter->first;
            if (!iter->second.empty()) {
                result += "=\"";
                result += iter->second;
                result += "\"";
            }
        }
        result += ">";
    }
    for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter)
        result += (*iter)->html();
    if (type == TAG && !is_void_element(name)) {
        result += "</";
        result += name;
        result += ">";
    }
    return result;
}

myu32string html_node::text() const {
    if (type == TEXT)
        return m_text;
    else {
        myu32string result;
        for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter)
            result += (*iter)->text();
        return result;
    }
}

const html_node *html_node::find(const html_selector &selector) const {
    if (selector.match(this))
        return this;
    for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter) {
        const html_node *result = (*iter)->find(selector);
        if (result)
            return result;
    }
    return NULL;
}

html_node *html_node::find(const html_selector &selector) {
    if (selector.match(this))
        return this;
    for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter) {
        html_node *result = (*iter)->find(selector);
        if (result)
            return result;
    }
    return NULL;
}

html_result<const html_node *> html_node::find_all(const html_selector &selector) const {
    html_result<const html_node *> result;
    if (selector.match(this))
        result.insert(this);
    for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter) {
        const html_node * pnode = *iter;
        html_result<const html_node *> temp_result = pnode->find_all(selector);
        result.insert(temp_result.cbegin(), temp_result.cend());
    }
    return result;
}

html_result<html_node *> html_node::find_all(const html_selector &selector) {
    html_result<html_node *> result;
    if (selector.match(this))
        result.insert(this);
    for (myvector<html_node *>::const_iterator iter = children.cbegin(); iter < children.cend(); ++iter) {
        html_result<html_node *> temp_result = (*iter)->find_all(selector);
        result.insert(temp_result.cbegin(), temp_result.cend());
    }
    return result;
}

bool html_selector::match(const html_node *node) const {
    if (node->type != html_node::TAG)
        return false;
    if (!m_tag.empty() && m_tag != node->name)
        return false;
    for (myvector<myu32string>::const_iterator iter = m_classes.cbegin(); iter < m_classes.cend(); ++iter)
        if (node->classes.find(*iter) == node->classes.cend())
            return false;
    for (myvector<mypair<myu32string, myu32string> >::const_iterator iter = m_attrs.cbegin(); iter != m_attrs.cend(); ++iter) {
        myhashmap<myu32string, myu32string>::const_iterator result = node->attrs.find(iter->first);
        if (result == node->attrs.cend())
            return false;
        if (result->second != iter->second)
            return false;
    }
    return true;
}
