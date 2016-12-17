//
// Created by sun on 12/17/16.
//
#include <iostream>
#include <fstream>
#include <cstring>

#include "threadpool.h"
#include "trie.h"
#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"

#if defined(_WIN32)
#define DllExport   __declspec(dllexport)
#else
#define DllExport
#endif
using namespace std;

struct zhihu_content {
    myu32string url, headline, question, author, content;
    myhashset<myu32string, int> words;
};

struct app {
    app() = default;
    app(const app &other) = delete;
    app &operator = (const app &other) = delete;
    app(app &&other) = default;
    app &operator = (app &&other) = default;
    ~app() {
        for (myvector<zhihu_content *>::const_iterator iter = infos.cbegin(); iter < infos.cend(); ++iter)
            if (*iter)
                delete *iter;
    }
    thread_pool threads;
    trie_tree dictionary, stopwords;
    myvector<mystring> files;
    myvector<zhihu_content *> infos;
};

inline const char *cvt_result(const mystring &result) {
    char *str = new char[result.size() + 1];
    memcpy(str, result.begin(), sizeof(char) * result.size());
    str[result.size()] = '\0';
    return str;
}

template<typename CharT>
mybasic_string<CharT> escape(const mybasic_string<CharT> &str) {
    mybasic_string<CharT> result;
    result.reserve(str.size());
    for (typename mybasic_string<CharT>::const_iterator iter = str.cbegin(); iter != str.cend(); ++iter) {
        if (*iter == '\t')
            result += "\\t";
        else if (*iter == '\n')
            result += "\\n";
        else if (*iter == '\\')
            result += "\\\\";
        else if (*iter == '\"')
            result += "\\\"";
        else
            result.push_back(*iter);
    }
    return result;
}

extern "C" {

DllExport app *init() {
    return new app;
}

DllExport void destroy(app *ins) {
    if (ins)
        delete ins;
}

DllExport const char *load_dict(app *ins) {
    mutex error_lock;
    mystring error;
    atomic_bool first(true);

    std::future<bool> dictionary_loaded = ins->threads.enqueue([ins, &error_lock, &error, &first] {
        // Load dictionary
        ins->dictionary.clear();
        ifstream dict_file("dictionary.dic");
        if (!dict_file) {
            error_lock.lock();
            if (!first)
                error.push_back('\n');
            else
                first = false;
            error += "Error opening \"dictionary.dic\"";
            error_lock.unlock();
            return false;
        }
        myu32string content = input_utf8_to_utf32(dict_file);
        size_t i, j = 0;
        if (!content.empty() && content.front() == 0xfeff) // BOM
            ++j;
        while ((i = j) < content.size()) {
            while (j < content.size() && content[j] != '\n')
                ++j;
            if (j > i)
                ins->dictionary.insert(content, i, j - i);
            ++j;
        }
        return true;
    }), stopwords_loaded = ins->threads.enqueue([ins, &error_lock, &error, &first] {
        // Load stopwords
        ins->stopwords.clear();
        ifstream stopwords_file("stop-words.dic");
        if (!stopwords_file) {
            error_lock.lock();
            if (!first)
                error.push_back('\n');
            else
                first = false;
            error += "Error opening \"stop-words.dic\"";
            error_lock.unlock();
            return false;
        }
        myu32string content = input_utf8_to_utf32(stopwords_file);
        size_t i, j = 0;
        ins->stopwords.insert("\n");
        if (!content.empty() && content.front() == 0xfeff) // BOM
            ++j;
        while ((i = j) < content.size()) {
            while (j < content.size() && content[j] != '\n')
                ++j;
            if (j > i)
                ins->stopwords.insert(content, i, j - i);
            ++j;
        }
        return true;
    });

    stopwords_loaded.get();
    dictionary_loaded.get();

    if (error.empty())
        return cvt_result("{\"okay\":true,\"result\":null}");
    return cvt_result("{\"okay\":false,\"result\":null,\"message\":\"" + escape(error) + "\"}");
}

DllExport const char *split_words(app *ins, const char *words) {
    myu32string content(utf8_to_utf32(words)), result;
    size_t start = 0;
    bool first = true;

    while (start != content.size()) {
        size_t length = ins->stopwords.match(content, start);
        if (!length) {
            while (start + length != content.size() && isgraph32(content[start + length]))
                ++length;
            if (!length) {
                length = ins->dictionary.match(content, start);
                if (!length)
                    ++length;
            }
            if (first)
                first = false;
            else
                result.push_back(',');
            result += "\"" + escape(content.substr(start, length)) + "\"";
        }
        start += length;
    }
    return cvt_result("{\"okay\":true,\"result\":[" + utf32_to_utf8(result) + "]}");
}

DllExport const char *load_htmls(app *ins) {
    mutex error_lock;
    mystring error;
    atomic_bool first(true);

    ins->files = mydir("input");
    myvector<std::future<zhihu_content *> > infos;
    for (auto &filename: ins->files) {
        infos.push_back(ins->threads.enqueue([&filename, &first, &error_lock, &error]() -> zhihu_content * {
            mystring input_filename = "input" PATH_SEPARATOR + filename;
            ifstream html_file(input_filename.c_str());
            if (!html_file) {
                error_lock.lock();
                if (!first)
                    error.push_back('\n');
                else
                    first = false;
                error += "Error opening \"" + input_filename + "\"";
                error_lock.unlock();
                return nullptr;
            }
            html_dom dom;
            try {
                dom.parse(input_utf8_to_utf32(html_file));
            } catch (mystring error) {
                error_lock.lock();
                if (!first)
                    error.push_back('\n');
                else
                    first = false;
                error += "Error opening \"" + input_filename + "\"";
                error_lock.unlock();
                return nullptr;
            }
            // Parse information
            zhihu_content *result = new zhihu_content;
            html_node *url = dom.find(html_selector("link").attr("rel", "canonical")), *headline = dom.find("h1"),
                    *question = dom.find("h2"), *author = dom.find(html_selector("span").class_("author"));
            if (url) {
                myhashmap<myu32string, myu32string>::const_iterator iter = url->attrs().find("href");
                if (iter != url->attrs().end())
                    result->url = iter->second;
            }
            if (headline)
                result->headline = headline->text();
            if (question)
                result->question = question->text();
            if (author)
                result->author = author->text();
            if (!result->author.empty() && result->author.back() == L'\uff0c') // Extra comma
                result->author.pop_back();
            html_result contents = dom.find_all(html_selector("div").class_("content"));
            for (auto j: contents) {
                for (auto k: j->children()) {
                    result->content += k->text();
                    result->content.push_back('\n');
                }
            }
            return result;
        }));
    }

    ins->infos.clear();
    myvector<std::future<bool> > results;
    for (size_t i = 0; i < ins->files.size(); ++i) {
        zhihu_content *result = infos[i].get();
        ins->infos.push_back(result);
    }
    return cvt_result("{\"okay\":true,\"result\":null}");
}

DllExport const char *load_words(app **ins) {
    return "";
}

DllExport const char *get_str(const char *str) {
    return str;
}

DllExport void free_str(const char *str) {
    delete []str;
}

}