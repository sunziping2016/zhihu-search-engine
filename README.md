# About
Yeah, this project may best explain what reinventing the wheel means. :) As it is my homework for
*Data Structure and Algorithm*, I'm not allowed to use any STL or `std::string`-liked standard components.
However, I've really missing them a lot, so I decide to reinvent them.

I've implemented these following things:
* `myvector`: dynamic array similar to `std::vector`
* `mylist`: doubly linked circular list similar to `std::list`

And I am going to implement the following things:
* `mystring`: inherited from `myvector` and similar to `std::string`
* `myhashset`: hash set similar to `std::unordered_set`
* `myhashmap`: hash map inherited from `myhashset` and similar to `std::unordered_map` 
* `mystack`: stack adaptor similar to `std::stack`

Every container has its corresponding iterator. And everything is compatible with STL. However,
due to the low version of teaching assistant's c++ compiling environment, I'm afraid that I
couldn't use C++ 11 or any version later.

Well, finally I'll use these to implement a html parser providing some interface similar to
`BeautifulSoap` and a quite simple word separation algorithm.
