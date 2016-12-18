#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"
#include "threadpool.h"
#include "trie.h"

using namespace std;

int main() {
    thread_pool threads;

    trie_tree dictionary, stopwords;

    std::future<bool> dictionary_loaded = threads.enqueue([&dictionary] {
        // Load dictionary
        ifstream dict_file("dictionary.dic");
        if (!dict_file) {
            cerr << "Error opening \"dictionary.dic\"\n";
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
                dictionary.insert(content, i, j - i);
            ++j;
        }
        return true;
    }), stopwords_loaded = threads.enqueue([&stopwords] {
        // Load stopwords
        ifstream dict_file("stop-words.dic");
        if (!dict_file) {
            cerr << "Error opening \"stop-words.dic\"\n";
            return false;
        }
        myu32string content = input_utf8_to_utf32(dict_file);
        size_t i, j = 0;
        stopwords.insert("\n");
        if (!content.empty() && content.front() == 0xfeff) // BOM
            ++j;
        while ((i = j) < content.size()) {
            while (j < content.size() && content[j] != '\n')
                ++j;
            if (j > i)
                stopwords.insert(content, i, j - i);
            ++j;
        }
        return true;
    });

    myvector<mystring> filenames(mydir("input"));
    sort(filenames.begin(), filenames.end());

    struct zhihu_content {
        myu32string url, headline, question, author, content;
    };

    myvector<std::future<zhihu_content *> > infos;
    for (auto &filename: filenames) {
        infos.push_back(threads.enqueue([&filename]() -> zhihu_content * {
            mystring input_filename = "input" PATH_SEPARATOR + filename;
            ifstream html_file(input_filename.c_str());
            if (!html_file) {
                cerr << ("Error opening \"" + input_filename + "\"\n");
                return nullptr;
            }
            html_dom dom;
            try {
                dom.parse(input_utf8_to_utf32(html_file));
            } catch (mystring error) {
                cerr << ("Error parsing \"" + filename + "\":" + error + "\n");
                return nullptr;
            } catch (runtime_error error) {
                cerr << error.what() << endl;
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

    if (!stopwords_loaded.get())
        return 1;
    if (!dictionary_loaded.get())
        return 1;

    myvector<std::future<myvector<myu32string> *> > results;
    for (size_t i = 0; i < filenames.size(); ++i) {
        results.push_back(threads.enqueue([i, &infos, &dictionary, &stopwords] () -> myvector<myu32string> * {
            zhihu_content *result = infos[i].get();
            if (!result)
                return nullptr;
            myvector<myu32string> *words = new myvector<myu32string>;
            size_t start = 0;
            while (start != result->content.size()) {
                size_t length = stopwords.match(result->content, start);
                if (!length) {
                    while (start + length != result->content.size() && isgraph32(result->content[start + length]))
                        ++length;
                    if (!length) {
                        length = dictionary.match(result->content, start);
                        if (!length)
                            ++length;
                    }
                    words->push_back(result->content.substr(start, length));
                }
                start += length;
            }
            delete result;
            return words;
        }));
    }

    myhashmap<myu32string, size_t *> words;
    words.reserve(100000);
    size_t *temp = new size_t[filenames.size()];
    memset(temp, 0, sizeof(size_t) * filenames.size());
    for (size_t i = 0; i < filenames.size(); ++i) {
        myvector<myu32string> *result = results[i].get();
        for (myvector<myu32string>::const_iterator iter = result->cbegin(); iter != result->cend(); ++iter) {
            mypair<myhashmap<myu32string, size_t *>::iterator, bool> ret = words.insert(mymake_pair(*iter, temp));
            if (ret.second) {
                temp = new size_t[filenames.size()];
                memset(temp, 0, sizeof(size_t) * filenames.size());
            }
            ++ret.first->second[i];
        }
        delete result;
    }

    ifstream query_file("query.txt");
    ofstream result_file("result.txt");
    if (!query_file) {
        cerr << "Error opening \"query.txt\"\n";
        return 1;
    }
    if (!result_file) {
        cerr << "Error opening \"result.txt\"\n";
        return 1;
    }
    myu32string query = input_utf8_to_utf32(query_file);
    size_t i, j = 0;
    if (!query.empty() && query.front() == 0xfeff) // BOM
        ++j;
    while ((i = j) < query.size()) {
        while (j < query.size() && query[j] != '\n')
            ++j;
        if (j > i) {
            size_t *result = new size_t[filenames.size()], *index = new size_t[filenames.size()];
            memset(result, 0, sizeof(size_t) * filenames.size());
            for (size_t k = 0; k < filenames.size(); ++k)
                index[k] = k;
            myu32string line = query.substr(i, j - i);
            size_t start = 0;
            while (start != line.size()) {
                size_t length = stopwords.match(line, start);
                if (!length) {
                    while (start + length != line.size() && isgraph32(line[start + length]))
                        ++length;
                    if (!length) {
                        length = dictionary.match(line, start);
                        if (!length)
                            ++length;
                    }
                    myhashmap<myu32string, size_t *>::const_iterator res = words.find(line.substr(start, length));
                    if (res != words.cend()) {
                        for (size_t k = 0; k < filenames.size(); ++k)
                            result[k] += res->second[k];
                    }
                }
                start += length;
            }
            sort(index, index + filenames.size(), [result] (size_t a, size_t b) {
                return result[a] > result[b];
            });
            bool first = true;
            for (size_t k = 0; k < filenames.size() && result[index[k]]; ++k) {
                if (first)
                    first = false;
                else
                    result_file << " ";
                result_file << "(" << (index[k] + 1) << "," << result[index[k]] << ")";
            }
            delete []result;
            delete []index;
            result_file << "\n";
        }
        ++j;
    }
    // Fixme
    // destruct
    return 0;
}
