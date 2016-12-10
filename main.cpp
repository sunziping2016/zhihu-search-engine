#include <iostream>
#include <fstream>
#include <iomanip>

#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"
#include "threadpool.h"
#include "trie.h"

using namespace std;

mystring percent_to_string(double x) {
    mystring number;
    int num = static_cast<int>(x * 100 + 0.5);
    char hundreds = static_cast<char>(num / 100) + '0',
            tens = static_cast<char>(num / 10 % 10) + '0',
            ones = static_cast<char>(num % 10) + '0';
    number.push_back(hundreds == '0' ? ' ' :  hundreds);
    number.push_back(hundreds == '0' && tens == '0' ? ' ' : tens);
    number.push_back(ones);
    return number;
}

struct zhihu_content {
    myu32string headline, question, author, content;
};

int main() {
    thread_pool threads;

    trie_tree dictionary;
    std::future<bool> dictionary_loaded = threads.enqueue([&dictionary] {
        // Load dictionary
        mystring line;
        ifstream dict_file("dictionary.dic");
        if (!dict_file) {
            cerr << "Error opening \"dictionary.dic\"\n";
            return false;
        }
        while (getline(dict_file, line)) {
            if (line.empty())
                continue;
            dictionary.insert(utf8_to_utf32(line));
        }
        return true;
    });

    myvector<mystring> filenames(mydir("input"));

    myvector<std::future<zhihu_content *> > infos;
    for (auto &filename: filenames) {
        infos.push_back(threads.enqueue([&filename]() -> zhihu_content * {
            mystring input_filename = "input" PATH_SEPARATOR + filename,
                    basename = filename.substr(0, filename.find(".")),
                    info_filename = "output" PATH_SEPARATOR + basename + ".info";
            ifstream html_file(input_filename.c_str());
            if (!html_file) {
                cerr << ("Error opening \"" + input_filename + "\"\n");
                return nullptr;
            }
            ofstream info_file(info_filename.c_str());
            if (!info_file) {
                cerr << ("Error creating \"" + info_filename + "\"\n");
                return nullptr;
            }
            // Build dom tree
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
            html_node *headline = dom.find("h1"), *question = dom.find("h2"), *author = dom.find(html_selector("span").class_("author"));
            if (headline)
                result->headline = headline->text();
            if (question)
                result->question = question->text();
            if (author)
                result->author = author->text();
            if (!result->author.empty() && result->author.back() == L'\uff0c') // Extra comma
                result->author.pop_back();
            html_result contents = dom.find_all(html_selector("div").class_("content"));
            contents.pop_back();
            for (auto j: contents) {
                for (auto k: *j) {
                    result->content += k->text();
                    result->content.push_back('\n');
                }
            }
            // Output information
            info_file << utf32_to_utf8(result->headline) << '\n'
                      << utf32_to_utf8(result->question) << '\n'
                      << utf32_to_utf8(result->author) << '\n'
                      << utf32_to_utf8(result->content);
            return result;
        }));
    }
    if (!dictionary_loaded.get())
        return 1;
    myvector<std::future<bool> > results;
    for (size_t i = 0; i < filenames.size(); ++i) {
        zhihu_content *result = infos[i].get();
        if (!result)
            continue;
        results.push_back(threads.enqueue([result, &filenames, i, &dictionary] {
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
                size_t length = dictionary.match(result->content, start);
                if (length == 0)
                    ++length;
                word_file << utf32_to_utf8(result->content.substr(start, length)) << '\n';
                start += length;
            }
            delete result;
            return true;
        }));
    }
    for (auto &i: results)
        i.get();
    return 0;
}
