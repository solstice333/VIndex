# VIndex

## Description

header-only lib of a generic VIndex data structure with STL-ish interface. See main.cpp for tests and examples, particularly `test_multi_comparators_iter()`

## Build

Compile for c++14

`$ make`

Or compile for c++11

`$ CPPSTD=11 make`

## Usage/Tests

run tests with

`$ ./a`

## TODO

- refactor NodeTracker typedef to be a POD struct
- remove() is under-optimized with linear time on removing an entry from the insertion list. When we append an entry to the insertion list, we need to save an iterator pointed to the last element and bind it to that entry's key, perhaps in the NodeTracker object. We can use the iterator returned by next(it.rbegin()).base() after appending to the insertion list.
