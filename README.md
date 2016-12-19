# About

This project is meant to build a small search engine for [zhihu.com](https://zhihu.com) as my *Data Structure
and Algorithm* homework. It consists of following two parts.

## First Part - Parse HTML and Perform Word Splitting
Yeah, this project may best explain what reinventing the wheel means. :) As it is my homework for
*Data Structure and Algorithm*, I'm not allowed to use any STL or `std::string`-liked standard components.
However, I've really missing them a lot, so I decide to reinvent them.

I've implemented these following things:
* `myvector`: dynamic array similar to `std::vector`
* `mylist`: doubly linked circular list similar to `std::list`
* `myhashset`: hash set similar to `std::unordered_set`
* `myhashmap`: hash map inherited from `myhashset` and similar to `std::unordered_map` 
* `mystring`: inherited from `myvector` and similar to `std::string`
* `mystack`: stack adaptor similar to `std::stack`
* `mypair`: simple pair similar to `std::pair`

Every container has its corresponding iterator as well as const iterator. And everything is compatible with STL.
However, due to the low version of teaching assistant's c++ compiling environment, I'm afraid that I
couldn't use C++ 11 or any version later.

Well, finally I've used these to implement a html parser providing some interface similar to
`BeautifulSoap` and a quite simple word separation algorithm.

## Second Part - Create Inverted Index for Web Search
I use trie tree to store the dictionary, and I also use multi-thread to boost peroformance.

As for GUI, I use `electron` with `node-ffi` to power JavaScript with the ability of calling C++ API.
Most designer of GUI is copied from Google.

