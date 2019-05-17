#include "vindex.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;

class Int;

typedef Vindex<int> IntVindex;
typedef Vindex<Int> IntVindex2;

class Int {
private:
   int _val;
public:
   Int(): _val(0) {}
   Int(int i): _val(i) {}
   int val() const { return _val; }
   void val(int i) { _val = i; }
   string str() const { 
      stringstream ss;
      ss << "i" << _val;
      return ss.str();
   }
   bool operator<(const Int &other) const { return this->_val < other._val; }
   bool operator>(const Int &other) const { return this->_val > other._val; }
   bool operator<=(const Int &other) const { return this->_val <= other._val; }
   bool operator>=(const Int &other) const { return this->_val >= other._val; }
   bool operator==(const Int &other) const { return this->_val == other._val; }
   bool operator!=(const Int &other) const { return this->_val != other._val; }
};

ostream& operator<<(ostream &os, const Int &i) {
   return os << i.str();
}

class TestIntVindex {
private:
   IntVindex _vin;
   IntVindex2 _vin2;

public:
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
   void bfs_dump(V &vin) {
      cout << vin.bfs_str("\n") << endl << endl;
   }

   template <typename V>
   void bfs_dump_one_line(V &vin) {
      cout << vin.bfs_str() << endl << endl;
   }

   void vin_bfs_dump() {
      bfs_dump(_vin);
   }

   void avin_bfs_dump() {
      bfs_dump(_vin2);
   }

   void vin_bfs_dump_one_line() {
      bfs_dump_one_line(_vin);
   }

   void avin_bfs_dump_one_line() {
      bfs_dump_one_line(_vin2);
   }

   void test_insert_left_left_right_right() {
      _vin.clear();
      _vin.insert(24);
      _vin.insert(20);
      _vin.insert(16);
      _vin.insert(12);
      _vin.insert(30);
      _vin.insert(35);
      assert(_vin.bfs_str() == "(data: 20, height: 3, left: 16, right: 30, parent: null)|(data: 16, height: 2, left: 12, right: null, parent: 20) (data: 30, height: 2, left: 24, right: 35, parent: 20)|(data: 12, height: 1, left: null, right: null, parent: 16) (null) (data: 24, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");
   }

   void test_insert_left_right_right_left() {
      _vin.clear();
      _vin.insert(30);
      _vin.insert(20);
      _vin.insert(25);
      _vin.insert(40);
      _vin.insert(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 35, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 35, height: 2, left: 30, right: 40, parent: 25)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");
      _vin.insert(42);
      _vin.insert(18);
      _vin.insert(22);
      _vin.insert(16);
      assert(_vin.bfs_str() == "(data: 35, height: 4, left: 20, right: 40, parent: null)|(data: 20, height: 3, left: 18, right: 25, parent: 35) (data: 40, height: 2, left: null, right: 42, parent: 35)|(data: 18, height: 2, left: 16, right: null, parent: 20) (data: 25, height: 2, left: 22, right: 30, parent: 20) (null) (data: 42, height: 1, left: null, right: null, parent: 40)|(data: 16, height: 1, left: null, right: null, parent: 18) (null) (data: 22, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25) (null) (null) (null) (null)");
   }

   void test_dne_removal() {
      _vin.clear();
      _vin.remove(40);
      assert(_vin.bfs_str() == "");
      _vin.insert(30);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");
   }

   void test_leaf_removal() {
      _vin.clear();
      _vin.insert(30);
      _vin.remove(30);
      assert(_vin.bfs_str() == "");

      _vin.insert(30);
      _vin.insert(20);
      _vin.insert(25);
      _vin.insert(40);
      _vin.insert(35);
      _vin.remove(30);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 35, height: 2, left: 25, right: 40, parent: null)|(data: 25, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(33);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 35, height: 1, left: null, right: null, parent: 33)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(27);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(22);
      _vin.insert(40);
      _vin.insert(24);
      _vin.remove(15);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 22, right: 35, parent: null)|(data: 22, height: 2, left: 20, right: 24, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 22) (data: 24, height: 1, left: null, right: null, parent: 22) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(22);
      _vin.insert(40);
      _vin.insert(21);
      _vin.remove(15);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 21, right: 35, parent: null)|(data: 21, height: 2, left: 20, right: 22, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 21) (data: 22, height: 1, left: null, right: null, parent: 21) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");
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
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      _vin.remove(20);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 40, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 20, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(30);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(40);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(30);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(18);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 18, right: 35, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(23);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 23, right: 35, parent: null)|(data: 23, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");
   }

   void test_two_children_removal() {
      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 35, height: 2, left: 20, right: null, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 35) (null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 40, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 40, height: 2, left: 30, right: null, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 40) (null)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(30);
      _vin.insert(23);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 23, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 23) (data: 30, height: 1, left: null, right: null, parent: 23)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(30);
      _vin.insert(18);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 20, height: 2, left: 18, right: 30, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 20) (data: 30, height: 1, left: null, right: null, parent: 20)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(33);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 40, height: 1, left: null, right: null, parent: 33)");

      _vin.clear();
      _vin.insert(25);
      _vin.insert(20);
      _vin.insert(35);
      _vin.insert(15);
      _vin.insert(30);
      _vin.insert(40);
      _vin.insert(27);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 40, height: 1, left: null, right: null, parent: 30)");
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
      for (it = _vin.begin(); it != _vin.end(); ++it) {
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
      for (it = prev(it); it != _vin.begin(); it--)
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
      auto it2 = _vin.begin();
      ss2 << *it2;
      ss2 << *++it2;
      ss2 << *++it2;
      ss2 << *--it2;
      assert(ss2.str() == "15162016");

      const IntVindex::const_iterator const_it = _vin.begin();
      const IntVindex::const_iterator const_end = _vin.end();
      assert(*const_it == 15);
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
      for (IntVindex2::const_iterator it = _vin2.begin();
         it != _vin2.end(); ++it) {
         ss << it->val();
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

      IntVindex::const_iterator it = _vin.begin();
      assert(*it == 25);
      assert(it != _vin.end());
      advance(it, 1);
      assert(*it == 16);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 15);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 20);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 35);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.end());
      ++it;
      assert(*it == 26);
      assert(it != _vin.end());
      ++it;
      assert(*it == 33);
      assert(it != _vin.end());
      ++it;
      assert(*it == 40);
      assert(it != _vin.end());
      ++it;
      assert(*it == 0);
      assert(it == _vin.end());
      ++it;
      assert(*it == 0);
      assert(it == _vin.end());

      stringstream ss;
      for (auto it2 = _vin.begin(); it2 != _vin.end(); ++it2)
         ss << *it2;

      assert(ss.str() == "251615203530263340");

      ss.str("");
      ss.clear();
      for (auto &data : _vin)
         ss << data;

      assert(ss.str() == "251615203530263340");

      assert(*--it == 40);
      assert(it != _vin.end());
      assert(*--it == 33);
      assert(it != _vin.end());
      assert(*--it == 26);
      assert(it != _vin.end());
      assert(*--it == 30);
      assert(it != _vin.end());
      assert(*--it == 35);
      assert(it != _vin.end());
      assert(*--it == 20);
      assert(it != _vin.end());
      assert(*--it == 15);
      assert(it != _vin.end());
      assert(*--it == 16);
      assert(it != _vin.end());
      assert(*--it == 25);
      assert(it != _vin.end());
      assert(*--it == 0);
      assert(it == _vin.end());
      assert(*--it == 0);
      assert(it == _vin.end());

      assert(*++it == 25);
      assert(it != _vin.end());
      assert(*++it == 16);
      assert(it != _vin.end());
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

      IntVindex::const_iterator it = _vin.begin();     
      assert(*it == 15);
      assert(it != _vin.end());
      advance(it, 1);
      assert(*it == 20);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 16);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 26);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 33);
      assert(it != _vin.end());
      it = next(it);
      assert(*it == 30);
      assert(it != _vin.end());
      ++it;
      assert(*it == 40);
      assert(it != _vin.end());
      ++it;
      assert(*it == 35);
      assert(it != _vin.end());
      ++it;
      assert(*it == 25);
      assert(it != _vin.end());
      ++it;
      assert(*it == 0);
      assert(it == _vin.end());
      ++it;
      assert(*it == 0);
      assert(it == _vin.end());

      stringstream ss;
      for (auto it2 = _vin.begin(); it2 != _vin.end(); ++it2)
         ss << *it2;

      assert(ss.str() == "152016263330403525");

      ss.str("");
      ss.clear();
      for (auto &data : _vin)
         ss << data;

      assert(ss.str() == "152016263330403525");

      advance(it, -1);
      assert(*it == 25);
      assert(it != _vin.end());
      it = prev(it);
      assert(*it == 35);
      assert(it != _vin.end());
      --it;
      assert(*it == 40);
      assert(it != _vin.end());
      it--;
      assert(*it == 30);
      assert(it != _vin.end());
      it--;
      assert(*it == 33);
      assert(it != _vin.end());
      it--;
      assert(*it == 26);
      assert(it != _vin.end());
      it--;
      assert(*it == 16);
      assert(it != _vin.end());
      it--;
      assert(*it == 20);
      assert(it != _vin.end());
      it--;
      assert(*it == 15);
      assert(it != _vin.end());
      --it;
      assert(*it == 0);
      assert(it == _vin.end());
      --it;
      assert(*it == 0);
      assert(it == _vin.end());

      ++it;
      assert(*it == 15);
      assert(it != _vin.end());
      ++it;
      assert(*it == 20);
      assert(it != _vin.end());
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

      IntVindex::const_iterator it = _vin.begin();     
      assert(*it == 25);
      assert(it != _vin.end());
      assert(*++it == 16);
      assert(it != _vin.end());
      assert(*++it == 35);
      assert(it != _vin.end());
      assert(*++it == 15);
      assert(it != _vin.end());
      assert(*++it == 20);
      assert(it != _vin.end());
      assert(*++it == 30);
      assert(it != _vin.end());
      assert(*++it == 40);
      assert(it != _vin.end());
      assert(*++it == 26);
      assert(it != _vin.end());
      assert(*++it == 33);
      assert(it != _vin.end());
      assert(*++it == 0);
      assert(it == _vin.end());
      assert(*++it == 0);
      assert(it == _vin.end());

      assert(*--it == 33);
      assert(it != _vin.end());
      assert(*--it == 26);
      assert(it != _vin.end());
      assert(*--it == 40);
      assert(it != _vin.end());
      assert(*--it == 30);
      assert(it != _vin.end());
      assert(*--it == 20);
      assert(it != _vin.end());
      assert(*--it == 15);
      assert(it != _vin.end());
      assert(*--it == 35);
      assert(it != _vin.end());
      assert(*--it == 16);
      assert(it != _vin.end());
      assert(*--it == 25);
      assert(it != _vin.end());
      assert(it == _vin.begin());
      assert(*--it == 0);
      assert(it == _vin.end());
      assert(*--it == 0);
      assert(it == _vin.end());

      assert(*++it == 25);
      assert(it != _vin.end());
      assert(*++it == 16);
      assert(it != _vin.end());
   }

   // TODO add iterator tests for empty v-index
   // TODO reverse iterator tests
   // TODO add insertion order iterator tests
};

int main () {
   TestIntVindex vin;

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
}
