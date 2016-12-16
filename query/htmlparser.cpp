//
// Created by sun on 11/13/16.
//
#include <stdexcept>

#include "htmlparser.h"
#include "mystack.h"
#include "mycodecvt.h"
#include "htmldata.h"

using namespace std;

static inline size_t skip_whitespace(const myu32string &html, size_t index) {
	while (index != html.size() && isspace32(html[index]))
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
	for (index2 = 0; index < html.size() && index2 < text.size() && tolower32(html[index]) == tolower32(text[index2]); ++index, ++index2);
	return index2 == text.size();
}


static inline bool is_void_element(const myu32string &name) {
	return void_elements.find(name) != void_elements.end();
}

static inline bool no_escape_element(const myu32string &name) {
	return name == "script" || name == "style";
}

static myu32string unescape_html(const myu32string &html, size_t begin, size_t end) {
	myu32string result;
	while (begin != end) {
		if (starts_with(html, begin, "&#")) {
			size_t orig_begin = begin;
			begin += 2;
			if (begin == end || !isdigit32(html[begin])) {
				result.push_back(html[orig_begin]);
				begin = orig_begin + 1;
				continue;
			}
			char32_t ch = 0;
			while (begin != end && isdigit32(html[begin])) {
				ch = ch * 10 + (html[begin] - '\0');
				++begin;
			}
			if (begin == end || html[begin] != ';') {
				result.push_back(html[orig_begin]);
				begin = orig_begin + 1;
				continue;
			}
			++begin;
		}
		else if (html[begin] == '&') {
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
			myhashmap<myu32string, char32_t>::const_iterator x = escape_string.find(html.substr(begin, new_end - begin));
			if (x == escape_string.cend()) {
				result.push_back(html[orig_begin]);
				begin = orig_begin + 1;
				continue;
			}
			result.push_back(x->second);
			begin = new_end + 1;
		}
		else {
			result.push_back(html[begin]);
			++begin;
		}
	}
	return result;
}


size_t html_node::parse_node(const myu32string &html, size_t index, html_node &parent, html_node &node, int &open_num) {
	size_t end_index;
	bool no_escape = no_escape_element(parent.m_name);
	if (starts_with(html, index, "<!--") && !no_escape) { // comment
		node.m_type = html_node::COMMENT;
		end_index = index += 4;
		while (end_index != html.size() && !starts_with(html, end_index, "-->"))
			++end_index;
		if (end_index == html.size())
			throw mystring("Unexpected ending in a comment");
		node.m_text = unescape_html(html, end_index, index);
		open_num = 0;
		return skip_whitespace(html, end_index + 3);
	}
	else if (starts_with(html, index, "</")) { // close tag
		node.m_type = html_node::TAG;
		end_index = index = skip_whitespace(html, index + 2);
		while (end_index != html.size() && html[end_index] != '>' && !isspace32(html[end_index]))
			++end_index;
		if (index == end_index)
			throw mystring("Unnamed close tag");
		if (end_index == html.size())
			throw mystring("Unexpected ending in a close tag");
		node.m_name = html.substr(index, end_index - index);
		index = skip_whitespace(html, end_index);
		if (index == html.size())
			throw "Unexpected ending in close tag \"" + utf32_to_utf8(node.m_name) + "\"";
		if (html[index] != '>')
			throw "Unexpected character in close tag \"" + utf32_to_utf8(node.m_name) + "\", expected \'>\'";
		open_num = -1;
		return skip_whitespace(html, index + 1);
	}
	else if (no_escape) { // text
		node.m_type = html_node::TEXT;
		end_index = index;
		myu32string close_tag = "</" + parent.m_name + ">";
		while (end_index != html.size() && !starts_with(html, end_index, close_tag))
			++end_index;
		size_t temp_index = end_index;
		while (isspace32(html[temp_index - 1]))
			--temp_index;
		node.m_text = html.substr(index, temp_index - index);
		open_num = 0;
		return end_index;

	}
	else if (html[index] == '<') { // open tag
		node.m_type = html_node::TAG;
		end_index = index = skip_whitespace(html, index + 1);
		while (end_index != html.size() && html[end_index] != '>' && !isspace32(html[end_index]) && !starts_with(html, end_index, "/>"))
			++end_index;
		if (end_index == html.size())
			throw mystring("Unexpected ending in an open tag");
		if (index == end_index)
			throw mystring("Unnamed open tag");
		node.m_name = html.substr(index, end_index - index);
		index = skip_whitespace(html, end_index);
		myu32string name, value;
		while (index != html.size() && html[index] != '>' && !starts_with(html, index, "/>")) {
			end_index = index;
			while (end_index != html.size() && html[end_index] != '>' && html[end_index] != '=' && !isspace32(html[end_index]) && !starts_with(html, end_index, "/>"))
				++end_index;
			if (end_index == html.size())
				throw "Unexpected ending in open tag \"" + utf32_to_utf8(node.m_name) + "\"";
			if (index == end_index)
				throw "Unnamed tag's attribute in open tag \"" + utf32_to_utf8(node.m_name) + "\"";
			name = html.substr(index, end_index - index);
			if (html[end_index] == '=') {
				index = skip_whitespace(html, end_index + 1);
				if (index == html.size())
					throw "Unexpected ending in atrribute \"" + utf32_to_utf8(name) + "\" in open tag \"" + utf32_to_utf8(node.m_name) + "\"";;
				if (html[index] == '\"') {
					end_index = index + 1;
					while (end_index != html.size() && html[end_index] != '\"')
						++end_index;
					if (end_index == html.size())
						throw "Unexpected ending in atrribute \"" + utf32_to_utf8(name) + "\" in open tag \"" + utf32_to_utf8(node.m_name) + "\"";;
					value = unescape_html(html, index + 1, end_index);
					index = end_index + 1;
				}
				else {
					end_index = index;
					while (end_index != html.size() && html[end_index] != '>' && !isspace32(html[end_index]) && !starts_with(html, end_index, "/>"))
						++end_index;
					if (end_index == html.size())
						throw "Unexpected ending in open tag \"" + utf32_to_utf8(node.m_name) + "\"";
					value = unescape_html(html, index, end_index);
					index = end_index;
				}
			}
			else {
				value = "";
				index = end_index;
			}
			if (name == "class") {
				size_t p = 0, q;
				while (p != value.size()) {
					q = p;
					while (q != value.size() && !isspace32(value[q]))
						++q;
					node.m_classes.insert(value.substr(p, q - p));
					p = q;
					while (p != value.size() && isspace32(value[p]))
						++p;
				}
			}
			node.m_attrs.insert(mymake_pair(name, value));
			index = skip_whitespace(html, index);
		}
		if (index == html.size())
			throw "Unexpected ending in open tag \"" + utf32_to_utf8(node.m_name) + "\"";
		if (html[index] == '>') {
			if (is_void_element(node.m_name))
				open_num = 0;
			else
				open_num = 1;
			return skip_whitespace(html, index + 1);
		}
		else {
			open_num = 0;
			return skip_whitespace(html, index + 2);
		}
	}
	else {
		node.m_type = html_node::TEXT;
		end_index = index;
		while (end_index != html.size() && html[end_index] != '<')
			++end_index;
		size_t temp_index = end_index;
		while (isspace32(html[temp_index - 1]))
			--temp_index;
		node.m_text = unescape_html(html, index, temp_index);
		open_num = 0;
		return end_index;
	}
}

static mystring index_to_mystring(size_t index) {
	mystack<char> stack;
	while (index) {
		stack.push((char)(index % 10 + '0'));
		index /= 10;
	}
	if (stack.empty())
		return "0";
	else {
		mystring str;
		while (!stack.empty()) {
			str.push_back(stack.top());
			stack.pop();
		}
		return str;
	}
}

void html_node::parse(const myu32string &html) {
	size_t index = 0;

	mystack<html_node *> nodes_stack;
	myvector<html_node *> nodes;
	html_node *node = NULL;

	try {
		try {
			nodes_stack.push(this);
			index = skip_whitespace(html, index);
			while (index != html.size()) {
				node = new html_node;
				int open_num = 0;
				index = parse_node(html, index, *nodes_stack.top(), *node, open_num);
				if (open_num == 1 || open_num == 0) {
					node->m_parent = nodes_stack.top();
					node->m_parent->m_children.push_back(node);
					if (open_num == 1)
						nodes_stack.push(node);
					nodes.push_back(node);
					node = NULL;
				}
				else {
					// open_num == -1
					if (nodes_stack.size() < 2)
						throw "Redundant close tag \"" + utf32_to_utf8(node->m_name) + "\"";
					if (nodes_stack.top()->m_name != node->m_name)
						throw "Unmatched close tag \"" + utf32_to_utf8(nodes_stack.top()->m_name) + "\" vs \"" + utf32_to_utf8(node->m_name) + "\"";
					nodes_stack.pop();
					delete node;
					node = NULL;
				}
			}
			if (nodes_stack.size() != 1)
				throw "Unclosed tag \"" + utf32_to_utf8(nodes_stack.top()->name()) + "\"";
		}
		catch (mystring error) {
			size_t line = 0, last_index = 0;
			for (size_t i = 0; i < index && i < html.size(); ++i)
				if (html[i] == '\n') {
					last_index = i;
					++line;
				}
			throw "(line: " + index_to_mystring(line + 1) + ", col: " + index_to_mystring(index < html.size() ? index - last_index + 1 : index - last_index) + ") " + error;
		}
	}
	catch (...) {
		for (myvector<html_node *>::const_iterator iter = nodes.cbegin(); iter != nodes.cend(); ++iter) {
			(*iter)->m_children.clear();
			delete *iter;
		}
		if (node) {
			node->m_children.clear();
			delete node;
		}
		m_children.clear();
		throw;
	}
}

ostream &operator << (ostream &out, const html_node &node) {
	const char *type_name[4]{
		"DOCUMENT",
		"TAG",
		"TEXT",
		"COMMENT"
	};
	out << "{ m_type: " << type_name[node.type()];
	if (node.type() == html_node::TAG) {
		out << ", m_name: " << utf32_to_utf8(node.name());
		if (!node.attrs().empty()) {
			out << ", attr: { ";
			for (myhashmap<myu32string, myu32string>::const_iterator iter = node.attrs().cbegin();
				iter != node.attrs().cend(); ++iter) {
				if (iter != node.attrs().cbegin())
					out << ", ";
				out << utf32_to_utf8(iter->first) << " : " << utf32_to_utf8(iter->second);
			}
			out << " }";
		}
	}

	if ((node.type() == html_node::DOCUMENT || node.type() == html_node::TAG) && !node.children().empty()) {
		out << ", child: [ ";
		for (size_t i = 0; i < node.children().size(); ++i) {
			if (i != 0)
				out << ", ";
			out << *node.children()[i];
		}
		out << " ]";
	}
	if (node.type() == html_node::COMMENT || node.type() == html_node::TEXT)
		out << ", m_text: " << utf32_to_utf8(node.text());
	out << " }";
	return out;
}

myu32string html_node::html() const {
	if (m_type == COMMENT)
		return "<!--" + m_text + "-->";
	if (m_type == TEXT)
		return m_text;
	myu32string result;
	if (m_type == TAG) {
		result += "<";
		result += m_name;
		for (myhashmap<myu32string, myu32string>::const_iterator iter = m_attrs.cbegin(); iter != m_attrs.cend(); ++iter) {
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
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter)
		result += (*iter)->html();
	if (m_type == TAG && !is_void_element(m_name)) {
		result += "</";
		result += m_name;
		result += ">";
	}
	return result;
}

myu32string html_node::text() const {
	if (m_type == TEXT)
		return m_text;
	else {
		myu32string result;
		for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter)
			result += (*iter)->text();
		return result;
	}
}

const html_node *html_node::find(const html_selector &selector) const {
	if (selector.match(this))
		return this;
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter) {
		const html_node *result = (*iter)->find(selector);
		if (result)
			return result;
	}
	return NULL;
}

html_node *html_node::find(const html_selector &selector) {
	if (selector.match(this))
		return this;
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter) {
		html_node *result = (*iter)->find(selector);
		if (result)
			return result;
	}
	return NULL;
}

basic_html_result<const html_node *> html_node::find_all(const html_selector &selector) const {
	basic_html_result<const html_node *> result;
	if (selector.match(this))
		result.push_back(this);
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter) {
		const html_node * pnode = *iter;
		result += pnode->find_all(selector);
	}
	return result;
}

basic_html_result<html_node *> html_node::find_all(const html_selector &selector) {
	basic_html_result<html_node *> result;
	if (selector.match(this))
		result.push_back(this);
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter)
		result += (*iter)->find_all(selector);
	return result;
}

void html_node::clear() {
	for (myvector<html_node *>::const_iterator iter = m_children.cbegin(); iter < m_children.cend(); ++iter)
		delete (*iter);
}

bool html_selector::match(const html_node *node) const {
	if (node->type() != html_node::TAG)
		return false;
	if (!m_tag.empty() && m_tag != node->name())
		return false;
	for (myvector<myu32string>::const_iterator iter = m_classes.cbegin(); iter < m_classes.cend(); ++iter)
		if (node->classes().find(*iter) == node->classes().cend())
			return false;
	for (myvector<mypair<myu32string, myu32string> >::const_iterator iter = m_attrs.cbegin(); iter != m_attrs.cend(); ++iter) {
		myhashmap<myu32string, myu32string>::const_iterator result = node->attrs().find(iter->first);
		if (result == node->attrs().cend() || result->second != iter->second)
			return false;
	}
	return true;
}
