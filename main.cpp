#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>

#include "mycodecvt.h"
#include "mydir.h"
#include "htmlparser.h"
#include "threadpool.h"

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
    thread_pool threads(2 * std::thread::hardware_concurrency());

    myhashset<myu32string> dictionary;
    size_t max_key_length = 0;

    std::future<bool> dictionary_loaded = threads.enqueue([&dictionary, &max_key_length] {
        // Load dictionary
        dictionary.reserve(1000000);
        mystring line;
        ifstream dict_file("dictionary.dic");
        if (!dict_file) {
            cerr << "Error opening \"dictionary.dic\"\n";
            return false;
        }
        while (getline(dict_file, line)) {
            if (line.empty())
                continue;
            myu32string data = utf8_to_utf32(line);
            if (data.size() > max_key_length)
                max_key_length = data.size();
            dictionary.insert(data);
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
        results.push_back(threads.enqueue([result, &filenames, i, &dictionary, max_key_length] {
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
                size_t length = result->content.size() - start;
                if (length > max_key_length)
                    length = max_key_length;
                while (length > 1 && dictionary.find(result->content.substr(start, length)) == dictionary.end())
                    --length;
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