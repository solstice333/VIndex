#include <string>
#include <sstream>
#include <iostream>
#include "vindex.h"
#include <utility>

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

using namespace std;

struct BasicInt;
struct Int;

typedef Vindex<int, BasicInt> IntVindex;
typedef Vindex<int, Int> IntVindex2;

struct BasicInt {
   int val;

   BasicInt(): val(0) {}

   BasicInt(int i): val(i) {}

   string str() const {
      stringstream ss;
      ss << val;
      return ss.str();
   }

   bool operator<(const BasicInt& other) const { 
      return this->val < other.val; 
   }

   bool operator>(const BasicInt& other) const { 
      return this->val > other.val; 
   }

   bool operator<=(const BasicInt& other) const { 
      return this->val <= other.val; 
   }

   bool operator>=(const BasicInt& other) const { 
      return this->val >= other.val; 
   }

   bool operator==(const BasicInt& other) const { 
      return this->val == other.val; 
   }

   bool operator!=(const BasicInt& other) const { 
      return this->val != other.val; 
   }
};

struct Point {
   int x, y;

   Point(): x(0), y(0) {}

   Point(int x, int y): x(x), y(y) {}

   string str() const {
      stringstream ss;
      ss << "(" << x << "," << y << ")";
      return ss.str();
   }

   bool operator<(const Point& other) const { 
      return x < other.x; 
   }
};

ostream& operator<<(ostream& os, const BasicInt& i) {
   return os << i.str();
}

ostream& operator<<(ostream& os, const Point& p) {
   return os << p.str();
}

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
   os << "[";
   for (auto it = v.begin(); it != v.end(); ++it)
      os << *it << (it == v.end() - 1 ? "" : ", ");
   os << "]";
   return os;
}

struct YCmp: public IComparator<Point> {
   int foo, bar;

   YCmp(int foo = 0, int bar = 0) : foo(foo), bar(bar) {}

   bool operator==(const IComparator<Point>& other) const override {
      auto ycmp = dynamic_cast<const YCmp*>(&other);
      return ycmp && foo == ycmp->foo && bar == ycmp->bar;
   }

   bool lt(const Point& a, const Point& b) const override {
      return a.y < b.y;
   }
};

template<>
struct std::hash<YCmp> {
   size_t operator()(const YCmp& ycmp) {
      std::hash<int> h;
      return hash_helpers::combine({ h(ycmp.foo), h(ycmp.bar) });
   }
};

struct Int {
   int val;

   Int(): val(0) {}

   Int(int i): val(i) {}

   string str() const { 
      stringstream ss;
      ss << "i" << val;
      return ss.str();
   }

   bool operator<(const Int& other) const { return this->val < other.val; }

   bool operator>(const Int& other) const { return this->val > other.val; }

   bool operator<=(const Int& other) const { return this->val <= other.val; }

   bool operator>=(const Int& other) const { return this->val >= other.val; }

   bool operator==(const Int& other) const { return this->val == other.val; }

   bool operator!=(const Int& other) const { return this->val != other.val; }
};

ostream& operator<<(ostream& os, const Int& i) {
   return os << i.str();
}

struct Foo {
   BasicInt key;
   BasicInt val;

   Foo(int key, int val): key(key), val(val) {};

   bool operator<(const Int& other) const { return this->val < other.val; }

   bool operator>(const Int& other) const { return this->val > other.val; }

   bool operator<=(const Int& other) const { return this->val <= other.val; }

   bool operator>=(const Int& other) const { return this->val >= other.val; }

   bool operator==(const Int& other) const { return this->val == other.val; }

   bool operator!=(const Int& other) const { return this->val != other.val; }  
};

struct Terran {
   std::string name;
   std::string occupation;
   int hp;

   Terran() {}

   Terran(std::string name, std::string occupation, int hp):
      name(name), occupation(occupation), hp(hp) {}

   bool operator<(const Terran& other) const {
      return name < other.name;
   }

   std::string str() const {
      std::stringstream ss;
      ss << "(" << name << "," << occupation << "," << hp << ")";
      return ss.str();
   }
};

struct OccupationCmp: public IComparator<Terran> {
   bool operator==(const IComparator& other) const override {
      return dynamic_cast<const OccupationCmp*>(&other) != nullptr;
   }

   bool lt(const Terran& a, const Terran& b) const override {
      return a.occupation < b.occupation;
   } 
};

struct HpCmp: public IComparator<Terran> {
   bool operator==(const IComparator& other) const override {
      return dynamic_cast<const HpCmp*>(&other) != nullptr;
   }

   bool lt(const Terran& a, const Terran& b) const override {
      return a.hp < b.hp;
   } 
};

ostream& operator<<(ostream& os, const Terran& t) {
   return os << t.str();
}

class TestIntVindex {
private:
   IntVindex _vin;
   IntVindex2 _vin2;

public:
   TestIntVindex(): 
      _vin(make_extractor(BasicInt, val)),
      _vin2([](const Int& i) -> int { return i.val; })
      {}

   operator IntVindex&() {
      return _vin;
   }

   operator IntVindex2&() {
      return _vin2;
   }

   void vin_loud_insert(int val) {
      cout << "inserting " << val << endl;
      _vin.insert(val);
   }

   template <typename V>
   void bfs_dump(const V& vin) {
      cout << vin._bfs_str("\n") << endl << endl;
   }

   template <typename V>
   void bfs_dump_one_line(const V& vin) {
      cout << vin._bfs_str() << endl << endl;
   }

   void vin_bfs_dump() {
      bfs_dump(_vin);
   }

   void vin2_bfs_dump() {
      bfs_dump(_vin2);
   }

   void vin_bfs_dump_one_line() {
      bfs_dump_one_line(_vin);
   }

   void vin2_bfs_dump_one_line() {
      bfs_dump_one_line(_vin2);
   }

   void test_insert_left_left() {
      _vin.clear();
      _vin.insert(24);
      _vin.insert(20);
      _vin.insert(16);
      assert(_vin._bfs_str() == "(data: 20, height: 2, left: 16, right: 24, parent: null)|(data: 16, height: 1, left: null, right: null, parent: 20) (data: 24, height: 1, left: null, right: null, parent: 20)");
   }

   void test_insert_left_left_right_right() {
      _vin.clear();
      _vin.insert(24);
      _vin.insert(20);
      _vin.insert(16);
      _vin.insert(12);
      _vin.insert(30);
      _vin.insert(35);
      assert(_vin._bfs_str() == "(data: 20, height: 3, left: 16, right: 30, parent: null)|(data: 16, height: 2, left: 12, right: null, parent: 20) (data: 30, height: 2, left: 24, right: 35, parent: 20)|(data: 12, height: 1, left: null, right: null, parent: 16) (null) (data: 24, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");
   }

   void test_insert_left_right_right_left() {
      _vin.clear();
      _vin.insert(30);
      _vin.insert(20);
      _vin.insert(25);
      _vin.insert(40);
      _vin.insert(35);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 35, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 35, height: 2, left: 30, right: 40, parent: 25)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");
      _vin.insert(42);
      _vin.insert(18);
      _vin.insert(22);
      _vin.insert(16);
      assert(_vin._bfs_str() == "(data: 35, height: 4, left: 20, right: 40, parent: null)|(data: 20, height: 3, left: 18, right: 25, parent: 35) (data: 40, height: 2, left: null, right: 42, parent: 35)|(data: 18, height: 2, left: 16, right: null, parent: 20) (data: 25, height: 2, left: 22, right: 30, parent: 20) (null) (data: 42, height: 1, left: null, right: null, parent: 40)|(data: 16, height: 1, left: null, right: null, parent: 18) (null) (data: 22, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25) (null) (null) (null) (null)");
   }

   void test_dne_removal() {
      _vin.clear();
      Result<BasicInt> res = _vin.remove(40);
      auto resf = dynamic_cast<ResultFailure<BasicInt> *>(res.get());
      assert(resf);
      assert(_vin._bfs_str() == "");

      _vin.insert(30);
      assert(_vin._bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");

      res = _vin.remove(40);
      assert(_vin._bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");

      resf = dynamic_cast<ResultFailure<BasicInt> *>(res.get());
      assert(resf);

      assert(_vin._bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");
   }

   void test_leaf_removal() {
      _vin.clear();
      _vin.insert(30);
      Result<BasicInt> res = _vin.remove(30);
      auto ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 30);
      assert(_vin._bfs_str() == "");

      _vin.insert(30);
      _vin.insert(20);
      _vin.insert(25);
      _vin.insert(40);
      _vin.insert(35);
      _vin.remove(30);

      res = _vin.remove(20);
      ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 20);
      assert(_vin._bfs_str() == "(data: 35, height: 2, left: 25, right: 40, parent: null)|(data: 25, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(33);
      _vin.remove(40);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 35, height: 1, left: null, right: null, parent: 33)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(27);
      _vin.remove(40);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(22);
      _vin.insert(40);
      _vin.insert(24);
      _vin.remove(15);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 22, right: 35, parent: null)|(data: 22, height: 2, left: 20, right: 24, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 22) (data: 24, height: 1, left: null, right: null, parent: 22) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(22);
      _vin.insert(40);
      _vin.insert(21);
      _vin.remove(15);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 21, right: 35, parent: null)|(data: 21, height: 2, left: 20, right: 22, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 21) (data: 22, height: 1, left: null, right: null, parent: 21) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");
  }

   void test_one_child_removal() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(30);
      _vin.insert(40);
      _vin.remove(30);
      Result<BasicInt> res = _vin.remove(35);
      auto ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 35);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      res = _vin.remove(20);
      ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 20);

      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 40, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 20, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(30);
      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(40);
      _vin.remove(35);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(30);
      _vin.remove(35);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(18);
      _vin.remove(20);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 18, right: 35, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(23);
      _vin.remove(20);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 23, right: 35, parent: null)|(data: 23, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");
   }

   void test_two_children_removal() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      Result<BasicInt> res = _vin.remove(25);
      auto ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 25);
      assert(_vin._bfs_str() == "(data: 35, height: 2, left: 20, right: null, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 35) (null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      res = _vin.remove(35);
      ress = dynamic_cast<ResultSuccess<BasicInt> *>(res.get());
      assert(ress);
      assert(ress->data().val == 35);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 40, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 40, height: 2, left: 30, right: null, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 40) (null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(30);
      _vin.insert(23);
      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 23, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 23) (data: 30, height: 1, left: null, right: null, parent: 23)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(30);
      _vin.insert(18);
      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 20, height: 2, left: 18, right: 30, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 20) (data: 30, height: 1, left: null, right: null, parent: 20)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(33);
      _vin.remove(35);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 40, height: 1, left: null, right: null, parent: 33)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(27);
      _vin.remove(35);
      assert(_vin._bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 40, height: 1, left: null, right: null, parent: 30)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(24);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(27);
      _vin.remove(25);
      assert(_vin._bfs_str("|") == "(data: 27, height: 3, left: 20, right: 35, parent: null)|(data: 20, height: 2, left: 15, right: 24, parent: 27) (data: 35, height: 2, left: 30, right: 40, parent: 27)|(data: 15, height: 1, left: null, right: null, parent: 20) (data: 24, height: 1, left: null, right: null, parent: 20) (data: 30, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");
   }

   void test_removal_by_key() {
      auto vin = make_vindex(Point, x);

      vin.emplace(1, 2);
      assert(vin._bfs_str() == "(data: (1,2), height: 1, left: null, right: null, parent: null)");

      vin.emplace(10, 12);
      assert(vin._bfs_str() == "(data: (1,2), height: 2, left: null, right: (10,12), parent: null)|(null) (data: (10,12), height: 1, left: null, right: null, parent: (1,2))");

      vin.emplace(14, 4);
      assert(vin._bfs_str() == "(data: (10,12), height: 2, left: (1,2), right: (14,4), parent: null)|(data: (1,2), height: 1, left: null, right: null, parent: (10,12)) (data: (14,4), height: 1, left: null, right: null, parent: (10,12))");

      vin.remove(1);
      assert(vin._bfs_str() == "(data: (10,12), height: 2, left: null, right: (14,4), parent: null)|(null) (data: (14,4), height: 1, left: null, right: null, parent: (10,12))");

      Result<Point> rm_res = vin.remove(Point(14, 4));
      assert(vin._bfs_str() == "(data: (10,12), height: 1, left: null, right: null, parent: null)");
      auto rm = dynamic_cast<ResultSuccess<Point>*>(rm_res.get());
      assert(rm);
      assert(rm->data().str() == "(14,4)");

      rm_res = vin.remove(12);
      assert(vin._bfs_str() == "(data: (10,12), height: 1, left: null, right: null, parent: null)");
      bool rm_failed = false;
      if (dynamic_cast<ResultFailure<Point>*>(rm_res.get()))
         rm_failed = true;
      assert(rm_failed);

      rm_res = vin.remove(10);
      assert(vin._bfs_str() == "");
      if (auto p = dynamic_cast<ResultSuccess<Point>*>(rm_res.get()))
         assert(p->data().str() == "(10,12)");
      else
         assert(0);
   }

   void test_iterative_remove() {
      auto vin = make_vindex(Point, x);
      vin.emplace(1, 4);
      vin.emplace(2, 4);
      vin.emplace(3, 4);
      vin.push_comparator(YCmp());

      vin.remove(2);

      assert(vin._bfs_str("|") == "(data: (3,4), height: 2, left: (1,4), right: null, parent: null)|(data: (1,4), height: 1, left: null, right: null, parent: (3,4)) (null)");
      assert(vin._bfs_str("|", YCmp()) == "(data: (3,4), height: 2, left: (1,4), right: null, parent: null)|(data: (1,4), height: 1, left: null, right: null, parent: (3,4)) (null)");

      vin.remove(3);

      assert(vin._bfs_str("|") == "(data: (1,4), height: 1, left: null, right: null, parent: null)");
      assert(vin._bfs_str("|", YCmp()) == "(data: (1,4), height: 1, left: null, right: null, parent: null)");

      vin.remove(1);

      assert(vin._bfs_str("|") == "");
      assert(vin._bfs_str("|", YCmp()) == "");
   }

   void test_in_order_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(33);

      stringstream ss;
      IntVindex::const_iterator it;
      for (it = _vin.cbegin(OrderType::INORDER); it != _vin.cend(); ++it) {
         ss << *it;
      }

      assert(ss.str() == "1516202530333540");

      assert(*it == 0);
      assert(*++it == 0);

      it = prev(it);
      assert(*it == 40);

      it = prev(it);
      assert(*it == 35);

      ss.str("");
      ss.clear();
      for (it = prev(it); it != _vin.cbegin(OrderType::INORDER); it--)
         ss << *it;
      ss << *it;

      assert(ss.str() == "333025201615");

      --it;
      assert(*it == 0);
      --it;
      assert(*it == 0);

      ++it;
      assert(*it == 15);

      stringstream ss2;
      auto it2 = _vin.cbegin(OrderType::INORDER);
      ss2 << *it2;
      ss2 << *++it2;
      ss2 << *++it2;
      ss2 << *--it2;
      ss2 << *it2--;
      ss2 << *it++;
      assert(ss2.str() == "151620161615");

      const IntVindex::const_iterator const_it = 
         _vin.cbegin(OrderType::INORDER);
      const IntVindex::const_iterator const_end = 
         _vin.cend();
      assert(*const_it == 15);
      assert(const_it->val == 15);
      assert(const_end == const_end);
      assert(const_it != const_end);
   }

   void test_in_order_iter_arrow_data() {
      _vin2.clear();
      _vin2.insert(Int(25));
      _vin2.insert(Int(20));
      _vin2.insert(Int(35));
      _vin2.insert(Int(15));
      _vin2.insert(Int(30));
      _vin2.insert(Int(40));
      _vin2.insert(Int(33));

      stringstream ss;
      stringstream ss2;
      for (IntVindex2::const_iterator it = _vin2.cbegin(OrderType::INORDER);
         it != _vin2.cend(); ++it) {
         ss << it->val;
         ss2 << *it;
      }

      assert(ss.str() == "15202530333540");
      assert(ss2.str() == "i15i20i25i30i33i35i40");
   }

   void test_pre_order_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_iterator it = _vin.cbegin(OrderType::PREORDER);
      assert(*it == 25);
      assert(it != _vin.cend());
      advance(it, 1);
      assert(*it == 16);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 15);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 20);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 35);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 26);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 33);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 40);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.cend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.cend());

      stringstream ss;
      for (auto it2 = _vin.cbegin(OrderType::PREORDER); 
         it2 != _vin.cend(); ++it2)
         ss << *it2;

      assert(ss.str() == "251615203530263340");

      ss.str("");
      ss.clear();

      assert(*--it == 40);
      assert(it != _vin.cend());
      assert(*--it == 33);
      assert(it != _vin.cend());
      assert(*--it == 26);
      assert(it != _vin.cend());
      assert(*--it == 30);
      assert(it != _vin.cend());
      assert(*--it == 35);
      assert(it != _vin.cend());
      assert(*--it == 20);
      assert(it != _vin.cend());
      assert(*--it == 15);
      assert(it != _vin.cend());
      assert(*--it == 16);
      assert(it != _vin.cend());
      assert(*--it == 25);
      assert(it != _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());

      assert(*++it == 25);
      assert(it != _vin.cend());
      assert(*++it == 16);
      assert(it != _vin.cend());
   }

   void test_post_order_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_iterator it = _vin.cbegin(OrderType::POSTORDER);
      assert(*it == 15);
      assert(it != _vin.cend());
      advance(it, 1);
      assert(*it == 20);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 16);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 26);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 33);
      assert(it != _vin.cend());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 40);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 35);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 25);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.cend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.cend());

      stringstream ss;
      for (auto it2 = _vin.cbegin(OrderType::POSTORDER); 
         it2 != _vin.cend(); ++it2)
         ss << *it2;

      assert(ss.str() == "152016263330403525");

      ss.str("");
      ss.clear();

      advance(it, -1);
      assert(*it == 25);
      assert(it != _vin.cend());
      it = prev(it);
      assert(*it == 35);
      assert(it != _vin.cend());
      --it;
      assert(*it == 40);
      assert(it != _vin.cend());
      it--;
      assert(*it == 30);
      assert(it != _vin.cend());
      it--;
      assert(*it == 33);
      assert(it != _vin.cend());
      it--;
      assert(*it == 26);
      assert(it != _vin.cend());
      it--;
      assert(*it == 16);
      assert(it != _vin.cend());
      it--;
      assert(*it == 20);
      assert(it != _vin.cend());
      it--;
      assert(*it == 15);
      assert(it != _vin.cend());
      --it;
      assert(*it == 0);
      assert(it == _vin.cend());
      --it;
      assert(*it == 0);
      assert(it == _vin.cend());

      ++it;
      assert(*it == 15);
      assert(it != _vin.cend());
      ++it;
      assert(*it == 20);
      assert(it != _vin.cend());
   }

   void test_breadth_first_order_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_iterator it = _vin.cbegin(OrderType::BREADTHFIRST);
      assert(*it == 25);
      assert(it != _vin.cend());
      assert(it.curr_level() == 1);
      assert(*++it == 16);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);
      assert(*++it == 35);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);
      assert(*++it == 15);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*++it == 20);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*++it == 30);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*++it == 40);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*++it == 26);
      assert(it != _vin.cend());
      assert(it.curr_level() == 4);
      assert(*++it == 33);
      assert(it != _vin.cend());
      assert(it.curr_level() == 4);
      assert(*++it == 0);
      assert(it == _vin.cend());
      assert(it.curr_level() == 4);
      assert(*++it == 0);
      assert(it == _vin.cend());
      assert(it.curr_level() == 4);

      assert(*--it == 33);
      assert(it != _vin.cend());
      assert(it.curr_level() == 4);
      assert(*--it == 26);
      assert(it != _vin.cend());
      assert(it.curr_level() == 4);
      assert(*--it == 40);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*--it == 30);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*--it == 20);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*--it == 15);
      assert(it != _vin.cend());
      assert(it.curr_level() == 3);
      assert(*--it == 35);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);
      assert(*--it == 16);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);
      assert(*--it == 25);
      assert(it != _vin.cend());
      assert(it == _vin.cbegin(OrderType::BREADTHFIRST));
      assert(it.curr_level() == 1);
      assert(*--it == 0);
      assert(it == _vin.cend());
      assert(it.curr_level() == 1);
      assert(*--it == 0);
      assert(it == _vin.cend());
      assert(it.curr_level() == 1);

      assert(*++it == 25);
      assert(it != _vin.cend());
      assert(it.curr_level() == 1);
      assert(*++it == 16);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);
      assert(*++it == 35);
      assert(it != _vin.cend());
      assert(it.curr_level() == 2);

      assert(*++it == 15);
      assert(it.curr_level() == 3);
      assert(*--it == 35);
      assert(it.curr_level() == 2);
      assert(*++it == 15);
      assert(it.curr_level() == 3);
      assert(*--it == 35);
      assert(it.curr_level() == 2);
   }

   void test_insertion_order_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_iterator it = _vin.cbegin(OrderType::INSERTION);
      assert(*it == 25);
      assert(it == _vin.cbegin(OrderType::INSERTION));
      assert(it != _vin.cend());
      assert(*++it == 20);
      assert(it != _vin.cend());
      assert(*++it == 35);
      assert(it != _vin.cend());
      assert(*++it == 15);
      assert(it != _vin.cend());
      assert(*++it == 30);
      assert(it != _vin.cend());
      assert(*++it == 40);
      assert(it != _vin.cend());
      assert(*++it == 16);
      assert(it != _vin.cend());
      assert(*++it == 26);
      assert(it != _vin.cend());
      assert(*++it == 33);
      assert(it != _vin.cend());
      assert(*++it == 0);
      assert(it == _vin.cend());
      assert(*++it == 0);
      assert(it == _vin.cend());

      assert(*--it == 33);
      assert(it != _vin.cend());
      assert(*--it == 26);
      assert(it != _vin.cend());
      assert(*--it == 16);
      assert(it != _vin.cend());
      assert(*--it == 40);
      assert(it != _vin.cend());
      assert(*--it == 30);
      assert(it != _vin.cend());
      assert(*--it == 15);
      assert(it != _vin.cend());
      assert(*--it == 35);
      assert(it != _vin.cend());
      assert(*--it == 20);
      assert(it != _vin.cend());
      assert(*--it == 25);
      assert(it == _vin.cbegin(OrderType::INSERTION));
      assert(it != _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());

      assert(*++it == 25);
      assert(it == _vin.cbegin(OrderType::INSERTION));
      assert(it != _vin.cend());
      assert(*++it == 20);
      assert(it != _vin.cend());

      _vin.remove(15);

      it = _vin.cbegin(OrderType::INSERTION);

      assert(*it == 25);
      assert(it == _vin.cbegin(OrderType::INSERTION));
      assert(it != _vin.cend());
      assert(*++it == 20);
      assert(it != _vin.cend());
      assert(*++it == 35);
      assert(it != _vin.cend());
      assert(*++it == 30);
      assert(it != _vin.cend());

      Vindex<int, BasicInt>::const_iterator it2 = it;
      assert(*it2 == 30);

      assert(*++it == 40);
      assert(it != _vin.cend());

      it2 = it;
      assert(*it2 == 40);
      assert(*++it2 == 16);

      assert(*++it == 16);
      assert(it != _vin.cend());
      assert(*++it == 26);
      assert(it != _vin.cend());
      assert(*++it == 33);
      assert(it != _vin.cend());
      assert(*++it == 0);
      assert(it == _vin.cend());
      assert(*++it == 0);
      assert(it == _vin.cend());
   }

   void test_in_order_rev_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(33);

      stringstream ss;
      IntVindex::const_reverse_iterator it;
      for (it = _vin.crbegin(OrderType::INORDER); it != _vin.crend(); ++it) {
         ss << *it;
      }

      assert(ss.str() == "4035333025201615");

      assert(*it == 0);
      assert(*++it == 0);

      it = prev(it);
      assert(*it == 15);

      it = prev(it);
      assert(*it == 16);

      ss.str("");
      ss.clear();
      for (it = prev(it); it != _vin.crbegin(OrderType::INORDER); it--)
         ss << *it;
      ss << *it;

      assert(ss.str() == "202530333540");

      --it;
      assert(*it == 0);
      --it;
      assert(*it == 0);

      ++it;
      assert(*it == 40);

      stringstream ss2;
      auto it2 = _vin.crbegin(OrderType::INORDER);
      ss2 << *it2;
      ss2 << *++it2;
      ss2 << *++it2;
      ss2 << *--it2;
      ss2 << *it2--;
      ss2 << *it2++;
      assert(ss2.str() == "403533353540");

      const IntVindex::const_reverse_iterator const_it = 
         _vin.crbegin(OrderType::INORDER);
      const IntVindex::const_reverse_iterator const_end = 
         _vin.crend();
      assert(*const_it == 40);
   }

   void test_pre_order_rev_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_reverse_iterator it = _vin.crbegin(OrderType::PREORDER);
      assert(*it == 40);
      assert(it != _vin.crend());
      advance(it, 1);
      assert(*it == 33);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 26);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 35);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 20);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 15);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 16);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 25);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.crend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.crend());

      stringstream ss;
      for (auto it2 = _vin.crbegin(OrderType::PREORDER); 
         it2 != _vin.crend(); ++it2)
         ss << *it2;

      assert(ss.str() == "403326303520151625");

      ss.str("");
      ss.clear();

      assert(*--it == 25);
      assert(it != _vin.crend());
      assert(*--it == 16);
      assert(it != _vin.crend());
      assert(*--it == 15);
      assert(it != _vin.crend());
      assert(*--it == 20);
      assert(it != _vin.crend());
      assert(*--it == 35);
      assert(it != _vin.crend());
      assert(*--it == 30);
      assert(it != _vin.crend());
      assert(*--it == 26);
      assert(it != _vin.crend());
      assert(*--it == 33);
      assert(it != _vin.crend());
      assert(*--it == 40);
      assert(it != _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());

      assert(*++it == 40);
      assert(it != _vin.crend());
      assert(*++it == 33);
      assert(it != _vin.crend());
   }

   void test_post_order_rev_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_reverse_iterator it = 
         _vin.crbegin(OrderType::POSTORDER);     
      assert(*it == 25);
      assert(it != _vin.crend());
      advance(it, 1);
      assert(*it == 35);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 40);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 33);
      assert(it != _vin.crend());
      it = next(it);
      assert(*it == 26);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 16);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 20);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 15);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.crend());
      ++it;
      assert(*it == 0);
      assert(it == _vin.crend());

      stringstream ss;
      for (auto it2 = _vin.crbegin(OrderType::POSTORDER); 
         it2 != _vin.crend(); ++it2)
         ss << *it2;

      assert(ss.str() == "253540303326162015");

      ss.str("");
      ss.clear();

      advance(it, -1);
      assert(*it == 15);
      assert(it != _vin.crend());
      it = prev(it);
      assert(*it == 20);
      assert(it != _vin.crend());
      --it;
      assert(*it == 16);
      assert(it != _vin.crend());
      it--;
      assert(*it == 26);
      assert(it != _vin.crend());
      it--;
      assert(*it == 33);
      assert(it != _vin.crend());
      it--;
      assert(*it == 30);
      assert(it != _vin.crend());
      it--;
      assert(*it == 40);
      assert(it != _vin.crend());
      it--;
      assert(*it == 35);
      assert(it != _vin.crend());
      it--;
      assert(*it == 25);
      assert(it != _vin.crend());
      --it;
      assert(*it == 0);
      assert(it == _vin.crend());
      --it;
      assert(*it == 0);
      assert(it == _vin.crend());

      ++it;
      assert(*it == 25);
      assert(it != _vin.crend());
      ++it;
      assert(*it == 35);
      assert(it != _vin.crend());
   }

   void test_breadth_first_order_rev_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_reverse_iterator it = 
         _vin.crbegin(OrderType::BREADTHFIRST);
      assert(*it == 33);
      assert(it != _vin.crend());
      assert(it.curr_level() == 4);
      assert(*++it == 26);
      assert(it != _vin.crend());
      assert(it.curr_level() == 4);
      assert(*++it == 40);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*++it == 30);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*++it == 20);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*++it == 15);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*++it == 35);
      assert(it != _vin.crend());
      assert(it.curr_level() == 2);
      assert(*++it == 16);
      assert(it != _vin.crend());
      assert(it.curr_level() == 2);
      assert(*++it == 25);
      assert(it != _vin.crend());
      assert(it.curr_level() == 1);
      assert(*++it == 0);
      assert(it == _vin.crend());
      assert(it.curr_level() == 1);
      assert(*++it == 0);
      assert(it == _vin.crend());
      assert(it.curr_level() == 1);

      assert(*--it == 25);
      assert(it != _vin.crend());
      assert(it.curr_level() == 1);
      assert(*--it == 16);
      assert(it != _vin.crend());
      assert(it.curr_level() == 2);
      assert(*--it == 35);
      assert(it != _vin.crend());
      assert(it.curr_level() == 2);
      assert(*--it == 15);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*--it == 20);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*--it == 30);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*--it == 40);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);
      assert(*--it == 26);
      assert(it != _vin.crend());
      assert(it.curr_level() == 4);
      assert(*--it == 33);
      assert(it != _vin.crend());
      assert(it == _vin.crbegin(OrderType::BREADTHFIRST));
      assert(it.curr_level() == 4);
      assert(*--it == 0);
      assert(it == _vin.crend());
      assert(it.curr_level() == 4);
      assert(*--it == 0);
      assert(it == _vin.crend());
      assert(it.curr_level() == 4);

      assert(*++it == 33);
      assert(it != _vin.crend());
      assert(it.curr_level() == 4);
      assert(*++it == 26);
      assert(it != _vin.crend());
      assert(it.curr_level() == 4);
      assert(*++it == 40);
      assert(it != _vin.crend());
      assert(it.curr_level() == 3);

      assert(*--it == 26);
      assert(it.curr_level() == 4);
      assert(*++it == 40);
      assert(it.curr_level() == 3);
      assert(*--it == 26);
      assert(it.curr_level() == 4);
      assert(*++it == 40);
      assert(it.curr_level() == 3);
   }

   void test_insertion_order_rev_iter() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(26);
      _vin.insert(33);

      IntVindex::const_reverse_iterator it = 
         _vin.crbegin(OrderType::INSERTION);
      assert(*it == 33);
      assert(it == _vin.crbegin(OrderType::INSERTION));
      assert(it != _vin.crend());
      assert(*++it == 26);
      assert(it != _vin.crend());
      assert(*++it == 16);
      assert(it != _vin.crend());
      assert(*++it == 40);
      assert(it != _vin.crend());
      assert(*++it == 30);
      assert(it != _vin.crend());
      assert(*++it == 15);
      assert(it != _vin.crend());
      assert(*++it == 35);
      assert(it != _vin.crend());
      assert(*++it == 20);
      assert(it != _vin.crend());
      assert(*++it == 25);
      assert(it != _vin.crend());
      assert(*++it == 0);
      assert(it == _vin.crend());
      assert(*++it == 0);
      assert(it == _vin.crend());

      assert(*--it == 25);
      assert(it != _vin.crend());
      assert(*--it == 20);
      assert(it != _vin.crend());
      assert(*--it == 35);
      assert(it != _vin.crend());
      assert(*--it == 15);
      assert(it != _vin.crend());
      assert(*--it == 30);
      assert(it != _vin.crend());
      assert(*--it == 40);
      assert(it != _vin.crend());
      assert(*--it == 16);
      assert(it != _vin.crend());
      assert(*--it == 26);
      assert(it != _vin.crend());
      assert(*--it == 33);
      assert(it == _vin.crbegin(OrderType::INSERTION));
      assert(it != _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());

      assert(*++it == 33);
      assert(it == _vin.crbegin(OrderType::INSERTION));
      assert(it != _vin.crend());
      assert(*++it == 26);
      assert(it != _vin.crend());

      _vin.remove(15);

      it = _vin.crbegin(OrderType::INSERTION);

      assert(*it == 33);
      assert(it == _vin.crbegin(OrderType::INSERTION));
      assert(it != _vin.crend());
      assert(*++it == 26);
      assert(it != _vin.crend());
      assert(*++it == 16);
      assert(it != _vin.crend());
      assert(*++it == 40);
      assert(it != _vin.crend());
      assert(*++it == 30);
      assert(it != _vin.crend());
      assert(*++it == 35);
      assert(it != _vin.crend());
      assert(*++it == 20);
      assert(it != _vin.crend());
      assert(*++it == 25);
      assert(it != _vin.crend());
      assert(*++it == 0);
      assert(it == _vin.crend());
      assert(*++it == 0);
      assert(it == _vin.crend());
   }

   void test_emplace() {
      _vin2.clear();
         
      _vin2.emplace(25);
      _vin2.emplace(20);
      _vin2.emplace(35);
      _vin2.emplace(15);
      _vin2.emplace(30);
      _vin2.emplace(40);
      _vin2.emplace(16);
      _vin2.emplace(26);
      _vin2.emplace(33);

      assert(_vin2._bfs_str() == "(data: i25, height: 4, left: i16, right: i35, parent: null)|(data: i16, height: 2, left: i15, right: i20, parent: i25) (data: i35, height: 3, left: i30, right: i40, parent: i25)|(data: i15, height: 1, left: null, right: null, parent: i16) (data: i20, height: 1, left: null, right: null, parent: i16) (data: i30, height: 2, left: i26, right: i33, parent: i35) (data: i40, height: 1, left: null, right: null, parent: i35)|(null) (null) (null) (null) (data: i26, height: 1, left: null, right: null, parent: i30) (data: i33, height: 1, left: null, right: null, parent: i30) (null) (null)");
   }

   void test_find() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(33);

      auto it = _vin.find(16, OrderType::INORDER);
      assert(*it == 16);
      assert(*++it == 20);

      it = _vin.find(100, OrderType::INORDER);
      assert(it == _vin.cend());
      assert(*it == 0);
   }

   void test_index_insert_removal() {
      _vin2.clear();
      _vin2.insert(25);
      _vin2.insert(20);
      _vin2.insert(35);
      _vin2.insert(15);
      _vin2.insert(30);
      _vin2.insert(40);
      _vin2.insert(16);
      _vin2.insert(33);     

      assert(_vin2._index.size() == 8);
      assert(_vin2.at(33).val == 33);
      assert(_vin2.at(16).val == 16);
      assert(_vin2.at(40).val == 40);
      assert(_vin2.at(30).val == 30);
      assert(_vin2.at(15).val == 15);
      assert(_vin2.at(35).val == 35);
      assert(_vin2.at(20).val == 20);
      assert(_vin2.at(25).val == 25);

      bool caught = false;
      try { _vin2.at(100); }
      catch (const out_of_range&) { caught = true; }
      assert(caught);

      _vin2.remove(15);
      assert(_vin2._index.size() == 7);
      assert(_vin2.at(33).val == 33);
      assert(_vin2.at(16).val == 16);
      assert(_vin2.at(40).val == 40);
      assert(_vin2.at(30).val == 30);
      assert(_vin2.at(35).val == 35);
      assert(_vin2.at(20).val == 20);
      assert(_vin2.at(25).val == 25);

      caught = false;
      try { _vin2.at(15); }
      catch (const out_of_range&) { caught = true; }
      assert(caught);
   }

   void test_insert_return() {
      _vin.clear();

      int cnt = 0;

      auto assert_successful_results = 
         [&cnt](ConstResult<BasicInt&>& r, int exp) {
            if (auto res = 
               dynamic_cast<ConstResultSuccess<BasicInt&> *>(r.get())) {
               assert(res->data() == exp);
               ++cnt;
            }
            else {
               assert(false);
               --cnt;
            }
         };

      auto assert_failing_results = 
         [&cnt](ConstResult<BasicInt&>& r) {
            if (auto res = 
               dynamic_cast<ConstResultFailure<BasicInt&> *>(r.get())) {
               ++cnt;
            }
            else {
               assert(false);
               --cnt;
            }
         };

      ConstResult<BasicInt&> r = _vin.insert(25);
      assert_successful_results(r, 25);
      r = _vin.insert(20);
      assert_successful_results(r, 20);
      r = _vin.insert(35);
      assert_successful_results(r, 35);
      r = _vin.insert(15);
      assert_successful_results(r, 15);
      r = _vin.insert(30);
      assert_successful_results(r, 30);
      r = _vin.insert(40);
      assert_successful_results(r, 40);
      r = _vin.insert(16);
      assert_successful_results(r, 16);
      r = _vin.insert(33);     
      assert_successful_results(r, 33);

      r = _vin.insert(15);     
      assert_failing_results(r);
      assert(cnt == 9);
   }

   void test_make_vindex() {
      Vindex<int, Int> myvin([](const Int& i) -> int { return i.val; });

      myvin.insert(30);
      myvin.insert(20);
      myvin.insert(25);
      myvin.insert(40);
      myvin.insert(35);
      myvin.insert(42);
      myvin.insert(18);
      myvin.insert(22);
      myvin.insert(16);

      assert(myvin._bfs_str() == "(data: i35, height: 4, left: i20, right: i40, parent: null)|(data: i20, height: 3, left: i18, right: i25, parent: i35) (data: i40, height: 2, left: null, right: i42, parent: i35)|(data: i18, height: 2, left: i16, right: null, parent: i20) (data: i25, height: 2, left: i22, right: i30, parent: i20) (null) (data: i42, height: 1, left: null, right: null, parent: i40)|(data: i16, height: 1, left: null, right: null, parent: i18) (null) (data: i22, height: 1, left: null, right: null, parent: i25) (data: i30, height: 1, left: null, right: null, parent: i25) (null) (null) (null) (null)");

      auto myvin2 = make_vindex(Int, val);   

      myvin2.insert(30);
      myvin2.insert(20);
      myvin2.insert(25);
      myvin2.insert(40);
      myvin2.insert(35);
      myvin2.insert(42);
      myvin2.insert(18);
      myvin2.insert(22);
      myvin2.insert(16);

      assert(myvin2._bfs_str() == "(data: i35, height: 4, left: i20, right: i40, parent: null)|(data: i20, height: 3, left: i18, right: i25, parent: i35) (data: i40, height: 2, left: null, right: i42, parent: i35)|(data: i18, height: 2, left: i16, right: null, parent: i20) (data: i25, height: 2, left: i22, right: i30, parent: i20) (null) (data: i42, height: 1, left: null, right: null, parent: i40)|(data: i16, height: 1, left: null, right: null, parent: i18) (null) (data: i22, height: 1, left: null, right: null, parent: i25) (data: i30, height: 1, left: null, right: null, parent: i25) (null) (null) (null) (null)");

      auto myvin3 = std::move(myvin2);
      assert(myvin3._bfs_str() == "(data: i35, height: 4, left: i20, right: i40, parent: null)|(data: i20, height: 3, left: i18, right: i25, parent: i35) (data: i40, height: 2, left: null, right: i42, parent: i35)|(data: i18, height: 2, left: i16, right: null, parent: i20) (data: i25, height: 2, left: i22, right: i30, parent: i20) (null) (data: i42, height: 1, left: null, right: null, parent: i40)|(data: i16, height: 1, left: null, right: null, parent: i18) (null) (data: i22, height: 1, left: null, right: null, parent: i25) (data: i30, height: 1, left: null, right: null, parent: i25) (null) (null) (null) (null)");
   }

   void test_size() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(16);
      _vin.insert(33);
      assert(_vin.size() == 8);
      _vin.remove(16);
      assert(_vin.size() == 7);
      _vin.clear();
      assert(_vin.size() == 0);
   }

   void test_iter_on_empty_vin() {
      _vin.clear();
      auto it = _vin.cbegin(OrderType::INORDER);
      assert(it == _vin.cend());
      it = _vin.cbegin(OrderType::INSERTION);
      assert(it == _vin.cend());
   }

   void test_multi_comparators() {
      Vindex<int, Point> vin(make_extractor(Point, x));
      vin.emplace(3, 3);
      vin.emplace(2, 4);
      vin.emplace(4, 2);

      assert(vin._bfs_str() == "(data: (3,3), height: 2, left: (2,4), right: (4,2), parent: null)|(data: (2,4), height: 1, left: null, right: null, parent: (3,3)) (data: (4,2), height: 1, left: null, right: null, parent: (3,3))");

      auto ycmp = YCmp(12, 14);
      vin.push_comparator(ycmp);

      assert(vin._bfs_str("|", ycmp) == "(data: (3,3), height: 2, left: (4,2), right: (2,4), parent: null)|(data: (4,2), height: 1, left: null, right: null, parent: (3,3)) (data: (2,4), height: 1, left: null, right: null, parent: (3,3))");

      vin.emplace(1, 5);

      assert(vin._bfs_str() == "(data: (3,3), height: 3, left: (2,4), right: (4,2), parent: null)|(data: (2,4), height: 2, left: (1,5), right: null, parent: (3,3)) (data: (4,2), height: 1, left: null, right: null, parent: (3,3))|(data: (1,5), height: 1, left: null, right: null, parent: (2,4)) (null) (null) (null)");

      assert(vin._bfs_str("|", ycmp) == "(data: (3,3), height: 3, left: (4,2), right: (2,4), parent: null)|(data: (4,2), height: 1, left: null, right: null, parent: (3,3)) (data: (2,4), height: 2, left: null, right: (1,5), parent: (3,3))|(null) (null) (null) (data: (1,5), height: 1, left: null, right: null, parent: (2,4))");
   }

   void test_multi_comparators_iter() {
      auto reset = 
         [](vector<string>& v, stringstream& ss) {
         v.clear();
         ss.str("");
         ss.clear();
      };

      stringstream ss;

      Vindex<int, Point> vin(make_extractor(Point, x));
      vin.emplace(3, 3);
      vin.emplace(2, 4);
      vin.emplace(4, 2);
      vin.emplace(1, 5);

      auto ycmp = YCmp(30);

      vector<string> v;
      for (auto it = vin.cbegin(OrderType::INORDER); it != vin.cend(); ++it) {
         stringstream ss;
         ss << *it;
         v.emplace_back(ss.str());
      }

      ss << v;
      assert(ss.str() == "[(1,5), (2,4), (3,3), (4,2)]");
      reset(v, ss);

      for (auto it = vin.cbegin(OrderType::INORDER, ycmp); 
         it != vin.cend(); ++it) {
         stringstream ss;
         ss << *it;
         v.emplace_back(ss.str());
      }

      ss << v;
      assert(ss.str() == "[(4,2), (3,3), (2,4), (1,5)]");
      reset(v, ss);

      for (auto it = vin.crbegin(OrderType::INSERTION, ycmp); 
         it != vin.crend(); ++it) {
         stringstream ss;
         ss << *it;
         v.emplace_back(ss.str());
      }

      ss << v;
      assert(ss.str() == "[(1,5), (4,2), (2,4), (3,3)]");
      reset(v, ss);

      vin.push_comparator(ycmp);
      for (auto it = vin.crbegin(OrderType::PREORDER, ycmp); 
         it != vin.crend(); ++it) {
         stringstream ss;
         ss << *it;
         v.emplace_back(ss.str());
      }

      ss << v;
      assert(ss.str() == "[(1,5), (2,4), (4,2), (3,3)]");
      reset(v, ss);
   }

   void doc_example() {
      // Vindex<std::string, Terran> vin(
      //    [](const Terran& t) -> std::string { return t.name; });
      // Vindex<std::string, Terran> vin(make_extractor(Terran, name));
      auto vin = make_vindex(Terran, name);
      vin.emplace("Jim Raynor", "Marine", 100);
      vin.insert(Terran("Matt Horner", "Battlecruiser", 500));
      vin.emplace("Tychus Findlay", "Marine", 100);
      vin.insert(Terran("Rory Swann", "Marauder", 100));
      vin.emplace("Gabriel Tosh", "Ghost", 90);
      vin.emplace("Lily Preston", "Medic", 75);

      auto jim_raynor = vin.at("Jim Raynor");

      std::cout << jim_raynor.name << ", " << jim_raynor.occupation
         << ", " << jim_raynor.hp << std::endl;
         
      std::cout << std::endl;

      for (auto it = vin.cbegin(OrderType::INORDER); it != vin.cend(); ++it)
         std::cout << it->name << std::endl;

      std::cout << std::endl;

      for (auto it = vin.cbegin(OrderType::INSERTION); it != vin.cend(); ++it)
         std::cout << it->name << std::endl;

      std::cout << std::endl;

      for (
         auto it = vin.cbegin(OrderType::INORDER, OccupationCmp()); 
         it != vin.cend(); 
         ++it)
            std::cout << it->occupation << ", " << it->name << std::endl;

      std::cout << std::endl;

      for (
         auto it = vin.cbegin(OrderType::INORDER, HpCmp()); 
         it != vin.cend(); 
         ++it)
         std::cout << it->hp << ", " << it->name << std::endl;

      std::cout << std::endl;

      assert(vin._bfs_str("|") == "(data: (Matt Horner,Battlecruiser,500), height: 3, left: (Jim Raynor,Marine,100), right: (Tychus Findlay,Marine,100), parent: null)|(data: (Jim Raynor,Marine,100), height: 2, left: (Gabriel Tosh,Ghost,90), right: (Lily Preston,Medic,75), parent: (Matt Horner,Battlecruiser,500)) (data: (Tychus Findlay,Marine,100), height: 2, left: (Rory Swann,Marauder,100), right: null, parent: (Matt Horner,Battlecruiser,500))|(data: (Gabriel Tosh,Ghost,90), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Tychus Findlay,Marine,100)) (null)");
      assert(vin._bfs_str("|", HpCmp()) == "(data: (Tychus Findlay,Marine,100), height: 3, left: (Gabriel Tosh,Ghost,90), right: (Matt Horner,Battlecruiser,500), parent: null)|(data: (Gabriel Tosh,Ghost,90), height: 2, left: (Lily Preston,Medic,75), right: (Jim Raynor,Marine,100), parent: (Tychus Findlay,Marine,100)) (data: (Matt Horner,Battlecruiser,500), height: 2, left: (Rory Swann,Marauder,100), right: null, parent: (Tychus Findlay,Marine,100))|(data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Jim Raynor,Marine,100), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Matt Horner,Battlecruiser,500)) (null)");
      assert(vin._bfs_str("|", OccupationCmp()) == "(data: (Jim Raynor,Marine,100), height: 3, left: (Gabriel Tosh,Ghost,90), right: (Tychus Findlay,Marine,100), parent: null)|(data: (Gabriel Tosh,Ghost,90), height: 2, left: (Matt Horner,Battlecruiser,500), right: (Rory Swann,Marauder,100), parent: (Jim Raynor,Marine,100)) (data: (Tychus Findlay,Marine,100), height: 2, left: null, right: (Lily Preston,Medic,75), parent: (Jim Raynor,Marine,100))|(data: (Matt Horner,Battlecruiser,500), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (null) (data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Tychus Findlay,Marine,100))");

      Result<Terran> res = vin.remove("Tychus Findlay");
      auto success = dynamic_cast<ResultSuccess<Terran>*>(res.get());
      assert(success);
      assert(success->data().name == "Tychus Findlay");

      assert(vin._bfs_str("|") == "(data: (Matt Horner,Battlecruiser,500), height: 3, left: (Jim Raynor,Marine,100), right: (Rory Swann,Marauder,100), parent: null)|(data: (Jim Raynor,Marine,100), height: 2, left: (Gabriel Tosh,Ghost,90), right: (Lily Preston,Medic,75), parent: (Matt Horner,Battlecruiser,500)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Matt Horner,Battlecruiser,500))|(data: (Gabriel Tosh,Ghost,90), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (null) (null)");
      assert(vin._bfs_str("|", HpCmp()) == "(data: (Rory Swann,Marauder,100), height: 3, left: (Gabriel Tosh,Ghost,90), right: (Matt Horner,Battlecruiser,500), parent: null)|(data: (Gabriel Tosh,Ghost,90), height: 2, left: (Lily Preston,Medic,75), right: (Jim Raynor,Marine,100), parent: (Rory Swann,Marauder,100)) (data: (Matt Horner,Battlecruiser,500), height: 1, left: null, right: null, parent: (Rory Swann,Marauder,100))|(data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Jim Raynor,Marine,100), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (null) (null)");
      assert(vin._bfs_str("|", OccupationCmp()) == "(data: (Jim Raynor,Marine,100), height: 3, left: (Gabriel Tosh,Ghost,90), right: (Lily Preston,Medic,75), parent: null)|(data: (Gabriel Tosh,Ghost,90), height: 2, left: (Matt Horner,Battlecruiser,500), right: (Rory Swann,Marauder,100), parent: (Jim Raynor,Marine,100)) (data: (Lily Preston,Medic,75), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100))|(data: (Matt Horner,Battlecruiser,500), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (null) (null)");

      vin.remove("Lily Preston"); 

      assert(vin._bfs_str("|") == "(data: (Matt Horner,Battlecruiser,500), height: 3, left: (Jim Raynor,Marine,100), right: (Rory Swann,Marauder,100), parent: null)|(data: (Jim Raynor,Marine,100), height: 2, left: (Gabriel Tosh,Ghost,90), right: null, parent: (Matt Horner,Battlecruiser,500)) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Matt Horner,Battlecruiser,500))|(data: (Gabriel Tosh,Ghost,90), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (null) (null) (null)");
      assert(vin._bfs_str("|", HpCmp()) == "(data: (Rory Swann,Marauder,100), height: 3, left: (Gabriel Tosh,Ghost,90), right: (Matt Horner,Battlecruiser,500), parent: null)|(data: (Gabriel Tosh,Ghost,90), height: 2, left: null, right: (Jim Raynor,Marine,100), parent: (Rory Swann,Marauder,100)) (data: (Matt Horner,Battlecruiser,500), height: 1, left: null, right: null, parent: (Rory Swann,Marauder,100))|(null) (data: (Jim Raynor,Marine,100), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (null) (null)");
      assert(vin._bfs_str("|", OccupationCmp()) == "(data: (Gabriel Tosh,Ghost,90), height: 3, left: (Matt Horner,Battlecruiser,500), right: (Jim Raynor,Marine,100), parent: null)|(data: (Matt Horner,Battlecruiser,500), height: 1, left: null, right: null, parent: (Gabriel Tosh,Ghost,90)) (data: (Jim Raynor,Marine,100), height: 2, left: (Rory Swann,Marauder,100), right: null, parent: (Gabriel Tosh,Ghost,90))|(null) (null) (data: (Rory Swann,Marauder,100), height: 1, left: null, right: null, parent: (Jim Raynor,Marine,100)) (null)");

      res = vin.remove("Lily Preston");
      auto failure = dynamic_cast<ResultFailure<Terran>*>(res.get());
      assert(failure);

      std::vector<std::string> actual;
      std::vector<std::string> exp(
         {"Gabriel Tosh", "Jim Raynor", "Matt Horner", "Rory Swann"});

      for (auto it = vin.cbegin(OrderType::INORDER); it != vin.cend(); ++it)
         actual.emplace_back(it->name);

      assert(actual == exp); 

      actual.clear();
      exp = std::vector<std::string>({
         "Battlecruiser,Matt Horner",
         "Ghost,Gabriel Tosh",
         "Marauder,Rory Swann",
         "Marine,Jim Raynor"
      });

      for (
         auto it = vin.cbegin(OrderType::INORDER, OccupationCmp()); 
         it != vin.cend(); 
         ++it) {
         std::stringstream ss;
         ss << it->occupation << "," << it->name;
         actual.emplace_back(ss.str());
      }

      assert(actual == exp);

      actual.clear();
      exp = std::vector<std::string>({
         "90,Gabriel Tosh",
         "100,Jim Raynor",
         "100,Rory Swann",
         "500,Matt Horner"
      });

      for (
         auto it = vin.cbegin(OrderType::INORDER, HpCmp()); 
         it != vin.cend(); 
         ++it) {
         std::stringstream ss;
         ss << it->hp << "," << it->name;
         actual.emplace_back(ss.str());
      }

      assert(actual == exp);

      assert(
         dynamic_cast<ResultSuccess<Terran>*>(
            vin.remove("Gabriel Tosh").get()
         )
      );

      assert(
         dynamic_cast<ResultSuccess<Terran>*>(
            vin.remove("Rory Swann").get()
         )
      );

      assert(
         dynamic_cast<ResultSuccess<Terran>*>(
            vin.remove("Matt Horner").get()
         )
      );

      actual.clear();
      exp = std::vector<std::string>({
         "Jim Raynor",
         "Marine,Jim Raynor",
         "100,Jim Raynor"
      });

      auto it = vin.cbegin(OrderType::INORDER);
      ++it;
      it = vin.cbegin(OrderType::POSTORDER);
      ++it;
      it = vin.cbegin(OrderType::PREORDER);
      ++it;
      it = vin.cbegin(OrderType::INSERTION);
      ++it;
      it = vin.cbegin(OrderType::BREADTHFIRST);
      ++it;

      for (auto it = vin.cbegin(OrderType::INORDER); it != vin.cend(); ++it)
         actual.emplace_back(it->name);

      for (
         auto it = vin.cbegin(OrderType::INORDER, OccupationCmp()); 
         it != vin.cend(); 
         ++it) {
         std::stringstream ss;
         ss << it->occupation << "," << it->name;
         actual.emplace_back(ss.str());
      }

      for (
         auto it = vin.cbegin(OrderType::INORDER, HpCmp()); 
         it != vin.cend(); 
         ++it) {
         std::stringstream ss;
         ss << it->hp << "," << it->name;
         actual.emplace_back(ss.str());
      }

      assert(actual == exp);
   }
};

int main () {
   TestIntVindex vin;

   vin.test_insert_left_left();
   vin.test_insert_left_left_right_right();
   vin.test_insert_left_right_right_left();
   vin.test_dne_removal();
   vin.test_leaf_removal();
   vin.test_one_child_removal();
   vin.test_two_children_removal();
   vin.test_removal_by_key();
   vin.test_iterative_remove();

   vin.test_iter_on_empty_vin();

   vin.test_in_order_iter();
   vin.test_in_order_iter_arrow_data();
   vin.test_pre_order_iter();
   vin.test_post_order_iter();
   vin.test_breadth_first_order_iter();
   vin.test_insertion_order_iter();

   vin.test_in_order_rev_iter();
   vin.test_pre_order_rev_iter();
   vin.test_post_order_rev_iter();
   vin.test_breadth_first_order_rev_iter();
   vin.test_insertion_order_rev_iter();

   vin.test_emplace();
   vin.test_find();
   vin.test_index_insert_removal();
   vin.test_insert_return();
   vin.test_make_vindex();
   vin.test_size();

   vin.test_multi_comparators();
   vin.test_multi_comparators_iter();
   vin.doc_example();
}
