#include <iostream>
#include <fstream>
#include <iomanip>

#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"
#include "threadpool.h"
#include "trie.h"

using namespace std;

struct zhihu_content {
    myu32string url, headline, question, author, content;
    myhashset<myu32string, int> words;
};

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

    myvector<std::future<bool> > results;
    for (size_t i = 0; i < filenames.size(); ++i) {
        zhihu_content *result = infos[i].get();
        if (!result)
            continue;
        results.push_back(threads.enqueue([result, &filenames, i, &dictionary, &stopwords] {
            mystring basename = filenames[i].substr(0, filenames[i].find(".")),
                    word_filename = "output" PATH_SEPARATOR + basename + ".txt";
            ofstream word_file(word_filename.c_str());
            if (!word_file) {
                cerr << ("Error creating \"" + word_filename + "\"\n");
                delete result;
                return false;
            }
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
                    word_file << utf32_to_utf8(result->content.substr(start, length)) << '\n';
                }
                start += length;
            }
            delete result;
            return true;
        }));
    }
    for (auto &i: results)
        i.get();
    //cin.get();
    return 0;
}
