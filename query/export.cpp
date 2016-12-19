//
// Created by sun on 12/17/16.
//
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

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
};

myu32string isubstr(const myu32string &str, size_t pos, size_t len) {
    myu32string result;
    for (size_t i = pos, end = pos + len; i < str.size() && i < end; ++i)
        result.push_back(tolower32(str[i]));
    return result;
};

struct app {
    app() {
        words.reserve(100000);
    }
    app(const app &other) = delete;
    app &operator = (const app &other) = delete;
    app(app &&other) = default;
    app &operator = (app &&other) = default;
    void clear_infos() {
        for (myvector<zhihu_content *>::const_iterator iter = infos.cbegin(); iter < infos.cend(); ++iter)
            if (*iter)
                delete *iter;
        infos.clear();
    }
    ~app() {
        // Fixme
        //clear_infos();
    }
    thread_pool threads;
    trie_tree dictionary, stopwords;
    myvector<mystring> files;
    myvector<zhihu_content *> infos;
    myhashmap<myu32string, size_t *> words;
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

DllExport const char *load(app *ins) {
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
    if (!error.empty())
        return cvt_result("{\"okay\":false,\"result\":null,\"message\":\"" + escape(error) + "\"}");

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
            } catch (mystring err) {
                error_lock.lock();
                if (!first)
                    error.push_back('\n');
                else
                    first = false;
                error += "Error parsing \"" + input_filename + "\":" + err;
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

    ins->clear_infos();
    myvector<std::future<myvector<mypair<myu32string, size_t> > *> > results;
    for (size_t i = 0; i < ins->files.size(); ++i) {
        zhihu_content *result = infos[i].get();
        ins->infos.push_back(result);
        results.push_back(ins->threads.enqueue([result, ins] () -> myvector<mypair<myu32string, size_t> > * {
            if (!result)
                return nullptr;
            myvector<mypair<myu32string, size_t> > *words = new myvector<mypair<myu32string, size_t> >;
            size_t start = 0;
            while (start != result->headline.size()) {
                size_t length = ins->stopwords.match(result->headline, start);
                if (!length) {
                    while (start + length != result->headline.size() && isgraph32(result->headline[start + length]))
                        ++length;
                    if (!length) {
                        length = ins->dictionary.match(result->headline, start);
                        if (!length)
                            ++length;
                    }
                    words->push_back(mymake_pair(isubstr(result->headline, start, length), 50));
                }
                start += length;
            }
            start = 0;
            while (start != result->content.size()) {
                size_t length = ins->stopwords.match(result->content, start);
                if (!length) {
                    while (start + length != result->content.size() && isgraph32(result->content[start + length]))
                        ++length;
                    if (!length) {
                        length = ins->dictionary.match(result->content, start);
                        if (!length)
                            ++length;
                    }
                    words->push_back(mymake_pair(isubstr(result->content, start, length), 1));
                }
                start += length;
            }
            return words;
        }));
    }

    ins->words.clear();
    size_t *temp = new size_t[ins->files.size()];
    memset(temp, 0, sizeof(size_t) * ins->files.size());
    for (size_t i = 0; i < ins->files.size(); ++i) {
        myvector<mypair<myu32string, size_t> > *result = results[i].get();
        if (!result)
            continue;
        for (myvector<mypair<myu32string, size_t> >::const_iterator iter = result->cbegin(); iter != result->cend(); ++iter) {
            mypair<myhashmap<myu32string, size_t *>::iterator, bool> ret = ins->words.insert(mymake_pair(iter->first, temp));
            if (ret.second) {
                temp = new size_t[ins->files.size()];
                memset(temp, 0, sizeof(size_t) * ins->files.size());
            }
            ret.first->second[i] += iter->second;
        }
        delete result;
    }

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
            result += "\"" + escape(isubstr(content, start, length)) + "\"";
        }
        start += length;
    }
    return cvt_result("{\"okay\":true,\"result\":[" + utf32_to_utf8(result) + "]}");
}

DllExport const char *query(app *ins, const char *str) {
    size_t *result = new size_t[ins->files.size()], *index = new size_t[ins->files.size()];
    memset(result, 0, sizeof(size_t) * ins->files.size());
    for (size_t k = 0; k < ins->files.size(); ++k)
        index[k] = k;
    myu32string query = utf8_to_utf32(str);
    size_t start = 0;
    while (start != query.size()) {
        size_t length = ins->stopwords.match(query, start);
        if (!length) {
            while (start + length != query.size() && isgraph32(query[start + length]))
                ++length;
            if (!length) {
                length = ins->dictionary.match(query, start);
                if (!length)
                    ++length;
            }
            myhashmap<myu32string, size_t *>::const_iterator res = ins->words.find(isubstr(query, start, length));
            if (res != ins->words.cend()) {
                for (size_t k = 0; k < ins->files.size(); ++k)
                    result[k] += res->second[k];
            }
        }
        start += length;
    }
    sort(index, index + ins->files.size(), [result] (size_t a, size_t b) {
        return result[a] > result[b];
    });
    mystring ret;
    bool first = true;
    for (size_t k = 0; k < ins->files.size() && result[index[k]]; ++k) {
        if (first)
            first = false;
        else
            ret.push_back(',');
        ret += "{\"id\":" + myto_string(index[k]) + ",\"rank\":" + myto_string(result[index[k]]) + "}";
    }
    delete []result;
    delete []index;
    return cvt_result("{\"okay\":true,\"result\":[" + ret + "]}");
}

DllExport const char *get_doc(app *ins, std::size_t id) {
    return cvt_result("{\"okay\":true,\"result\":{"
            "\"url\":\""    + utf32_to_utf8(escape(ins->infos[id]->url)) + "\","
            "\"headline\":\"" + utf32_to_utf8(escape(ins->infos[id]->headline)) + "\","
            "\"question\":\"" + utf32_to_utf8(escape(ins->infos[id]->question)) + "\","
            "\"author\":\"" + utf32_to_utf8(escape(ins->infos[id]->author)) + "\","
            "\"content\":\"" + utf32_to_utf8(escape(ins->infos[id]->content)) + "\""
    "}}");
}

DllExport const char *get_str(const char *str) {
    return str;
}

DllExport void free_str(const char *str) {
    delete []str;
}

}
