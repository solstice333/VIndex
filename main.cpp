#include "bst.h"
#include <iostream>
#include <functional>

struct Foo {
   int x;
   int y;

   Foo(): x(0), y(0) {}

   Foo(int x, int y): x(x), y(y) {}

   bool operator<(const Foo& other) const {
      return x < other.x;
   }
};

template <>
struct std::hash<Foo> {
   size_t operator()(const Foo& foo) {
      std::hash<int> int_hasher;
      std::vector<size_t> v;
      v.emplace_back(int_hasher(foo.x));
      v.emplace_back(int_hasher(foo.y));
      return hash_helpers::combine(v);
   }
};

struct FooYComparator: public IComparator<Foo> {
   bool operator==(const IComparator& other) const override {
      return dynamic_cast<const FooYComparator *>(&other);
   }

   bool lt(const Foo& a, const Foo& b) const override {
      return a.y < b.y;
   }
};

std::ostream& operator<<(std::ostream& os, const Foo& foo) {
   return os << "(" << foo.x << "," << foo.y << ")";
}

int main() {
   BST<Foo> bst;
   bst.insert(Foo(3, 3));
   bst.insert(Foo(2, 4));
   bst.insert(Foo(4, 2));

   std::cout << bst.str("\n") << std::endl;
   std::cout << std::endl;

   bst.push_comparator(FooYComparator());

   std::cout << bst.str("\n", FooYComparator()) << std::endl;
   std::cout << std::endl;

   std::cout << "-------------" << std::endl;
   
   bst.insert(Foo(1, 5));

   std::cout << bst.str("\n") << std::endl;
   std::cout << std::endl;

   std::cout << bst.str("\n", FooYComparator()) << std::endl;
   std::cout << std::endl;
}
