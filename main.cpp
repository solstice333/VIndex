#include "vindex.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;

class BasicInt;
class Int;

typedef Vindex<int, BasicInt> IntVindex;
typedef Vindex<int, Int> IntVindex2;

class BasicInt {
public:
   int val;

   BasicInt(): val(0) {}

   BasicInt(int i): val(i) {}

   string str() const {
      stringstream ss;
      ss << val;
      return ss.str();
   }

   bool operator<(const BasicInt &other) const { 
      return this->val < other.val; 
   }

   bool operator>(const BasicInt &other) const { 
      return this->val > other.val; 
   }

   bool operator<=(const BasicInt &other) const { 
      return this->val <= other.val; 
   }

   bool operator>=(const BasicInt &other) const { 
      return this->val >= other.val; 
   }

   bool operator==(const BasicInt &other) const { 
      return this->val == other.val; 
   }

   bool operator!=(const BasicInt &other) const { 
      return this->val != other.val; 
   }
};

ostream& operator<<(ostream &os, const BasicInt &i) {
   return os << i.str();
}

class Int {
public:
   int val;

   Int(): val(0) {}

   Int(int i): val(i) {}

   string str() const { 
      stringstream ss;
      ss << "i" << val;
      return ss.str();
   }

   bool operator<(const Int &other) const { return this->val < other.val; }

   bool operator>(const Int &other) const { return this->val > other.val; }

   bool operator<=(const Int &other) const { return this->val <= other.val; }

   bool operator>=(const Int &other) const { return this->val >= other.val; }

   bool operator==(const Int &other) const { return this->val == other.val; }

   bool operator!=(const Int &other) const { return this->val != other.val; }
};

ostream& operator<<(ostream &os, const Int &i) {
   return os << i.str();
}

class TestIntVindex {
private:
   IntVindex _vin;
   IntVindex2 _vin2;

public:
   TestIntVindex(): 
      _vin(member_offset(BasicInt, val)),
      _vin2(member_offset(Int, val)) 
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
      _vin.remove(40);
      assert(_vin._bfs_str() == "");
      _vin.insert(30);
      _vin.remove(40);
      assert(_vin._bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");
   }

   void test_leaf_removal() {
      _vin.clear();
      _vin.insert(30);
      _vin.remove(30);
      assert(_vin._bfs_str() == "");

      _vin.insert(30);
      _vin.insert(20);
      _vin.insert(25);
      _vin.insert(40);
      _vin.insert(35);
      _vin.remove(30);
      _vin.remove(20);
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
      _vin.remove(35);
      assert(_vin._bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      _vin.remove(20);
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
      _vin.remove(25);
      assert(_vin._bfs_str() == "(data: 35, height: 2, left: 20, right: null, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 35) (null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.remove(35);
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

      _vin.order(OrderType::INORDER);

      stringstream ss;
      IntVindex::const_iterator it;
      for (it = _vin.cbegin(); it != _vin.cend(); ++it) {
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
      for (it = prev(it); it != _vin.cbegin(); it--)
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
      auto it2 = _vin.cbegin();
      ss2 << *it2;
      ss2 << *++it2;
      ss2 << *++it2;
      ss2 << *--it2;
      assert(ss2.str() == "15162016");

      const IntVindex::const_iterator const_it = _vin.cbegin();
      const IntVindex::const_iterator const_end = _vin.cend();
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

      _vin2.order(OrderType::INORDER);

      stringstream ss;
      stringstream ss2;
      for (IntVindex2::const_iterator it = _vin2.cbegin();
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

      _vin.order(OrderType::PREORDER);

      IntVindex::const_iterator it = _vin.cbegin();
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
      for (auto it2 = _vin.cbegin(); it2 != _vin.cend(); ++it2)
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

      _vin.order(OrderType::POSTORDER);

      IntVindex::const_iterator it = _vin.cbegin();     
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
      for (auto it2 = _vin.cbegin(); it2 != _vin.cend(); ++it2)
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

      _vin.order(OrderType::BREADTHFIRST);     

      IntVindex::const_iterator it = _vin.cbegin();     
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
      assert(it == _vin.cbegin());
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

      _vin.order(OrderType::INSERTION);

      IntVindex::const_iterator it = _vin.cbegin();
      assert(*it == 25);
      assert(it == _vin.cbegin());
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
      assert(it == _vin.cbegin());
      assert(it != _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());
      assert(*--it == 0);
      assert(it == _vin.cend());

      assert(*++it == 25);
      assert(it == _vin.cbegin());
      assert(it != _vin.cend());
      assert(*++it == 20);
      assert(it != _vin.cend());

      _vin.remove(15);

      it = _vin.cbegin();

      assert(*it == 25);
      assert(it == _vin.cbegin());
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

   // TODO add iterator tests for empty v-index

   // TODO reverse iterator tests
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

      _vin.order(OrderType::INORDER);

      stringstream ss;
      IntVindex::const_reverse_iterator it;
      for (it = _vin.crbegin(); it != _vin.crend(); ++it) {
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
      for (it = prev(it); it != _vin.crbegin(); it--)
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
      auto it2 = _vin.crbegin();
      ss2 << *it2;
      ss2 << *++it2;
      ss2 << *++it2;
      ss2 << *--it2;
      assert(ss2.str() == "40353335");

      const IntVindex::const_reverse_iterator const_it = _vin.crbegin();
      const IntVindex::const_reverse_iterator const_end = _vin.crend();
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

      _vin.order(OrderType::PREORDER);

      IntVindex::const_reverse_iterator it = _vin.crbegin();
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
      for (auto it2 = _vin.crbegin(); it2 != _vin.crend(); ++it2)
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

      _vin.order(OrderType::POSTORDER);

      IntVindex::const_reverse_iterator it = _vin.crbegin();     
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
      for (auto it2 = _vin.crbegin(); it2 != _vin.crend(); ++it2)
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

      _vin.order(OrderType::BREADTHFIRST);

      IntVindex::const_reverse_iterator it = _vin.crbegin();     
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
      assert(it == _vin.crbegin());
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

      _vin.order(OrderType::INSERTION);

      IntVindex::const_reverse_iterator it = _vin.crbegin();
      assert(*it == 33);
      assert(it == _vin.crbegin());
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
      assert(it == _vin.crbegin());
      assert(it != _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());
      assert(*--it == 0);
      assert(it == _vin.crend());

      assert(*++it == 33);
      assert(it == _vin.crbegin());
      assert(it != _vin.crend());
      assert(*++it == 26);
      assert(it != _vin.crend());

      _vin.remove(15);

      it = _vin.crbegin();

      assert(*it == 33);
      assert(it == _vin.crbegin());
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

   void test_emplace_back() {
      _vin2.clear();
         
      _vin2.emplace_back(25);
      _vin2.emplace_back(20);
      _vin2.emplace_back(35);
      _vin2.emplace_back(15);
      _vin2.emplace_back(30);
      _vin2.emplace_back(40);
      _vin2.emplace_back(16);
      _vin2.emplace_back(26);
      _vin2.emplace_back(33);

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

      _vin.order(OrderType::INORDER);

      auto it = _vin.find(16);
      assert(*it == 16);
      assert(*++it == 20);

      it = _vin.find(100);
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
         [&cnt](ConstResult<BasicInt> &r, int exp) {
            if (auto res = 
               dynamic_cast<ConstResultSuccess<BasicInt> *>(r.get())) {
               assert(res->data() == exp);
               ++cnt;
            }
            else {
               assert(false);
               --cnt;
            }
         };

      auto assert_failing_results = 
         [&cnt](ConstResult<BasicInt> &r, int exp) {
            if (auto res = 
               dynamic_cast<ConstResultFailure<BasicInt> *>(r.get())) {
               assert(res->data() == exp);
               ++cnt;
            }
            else {
               assert(false);
               --cnt;
            }
         };

      ConstResult<BasicInt> r = _vin.insert(25);
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
      assert_failing_results(r, 0);
      assert(cnt == 9);
   }

   // TODO add tests where the key type needs std::hash() specialized
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

   vin.test_emplace_back();
   vin.test_find();
   vin.test_index_insert_removal();
   vin.test_insert_return();
}
