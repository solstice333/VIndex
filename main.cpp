#include "vindex.h"
#include <cassert>

using namespace std;

typedef Vindex<int> IntVindex;

class TestIntVindex {
private:
   IntVindex _vin;

public:
   operator IntVindex&() {
      return _vin;
   }

   void insert(int val) {
      _vin.insert(val);
   }

   void loud_insert(int val) {
      cout << "inserting " << val << endl;
      insert(val);
   }

   void clear() {
      _vin.clear();
   }

   void bfs_dump() {
      cout << _vin.bfs_str("\n") << endl << endl;
   }

   void bfs_dump_one_line() {
      cout << _vin.bfs_str() << endl << endl;
   }

   void test_insert_left_left_right_right() {
      clear();
      insert(24);
      insert(20);
      insert(16);
      insert(12);
      insert(30);
      insert(35);
      assert(_vin.bfs_str() == "(data: 20, height: 3, left: 16, right: 30, parent: null)|(data: 16, height: 2, left: 12, right: null, parent: 20) (data: 30, height: 2, left: 24, right: 35, parent: 20)|(data: 12, height: 1, left: null, right: null, parent: 16) (null) (data: 24, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");
   }

   void test_insert_left_right_right_left() {
      clear();
      insert(30);
      insert(20);
      insert(25);
      insert(40);
      insert(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 35, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 35, height: 2, left: 30, right: 40, parent: 25)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");
      insert(42);
      insert(18);
      insert(22);
      insert(16);
      assert(_vin.bfs_str() == "(data: 35, height: 4, left: 20, right: 40, parent: null)|(data: 20, height: 3, left: 18, right: 25, parent: 35) (data: 40, height: 2, left: null, right: 42, parent: 35)|(data: 18, height: 2, left: 16, right: null, parent: 20) (data: 25, height: 2, left: 22, right: 30, parent: 20) (null) (data: 42, height: 1, left: null, right: null, parent: 40)|(data: 16, height: 1, left: null, right: null, parent: 18) (null) (data: 22, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25) (null) (null) (null) (null)");
   }

   void test_dne_removal() {
      clear();
      _vin.remove(40);
      assert(_vin.bfs_str() == "");
      insert(30);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");
   }

   void test_leaf_removal() {
      clear();
      insert(30);
      _vin.remove(30);
      assert(_vin.bfs_str() == "");

      insert(30);
      insert(20);
      insert(25);
      insert(40);
      insert(35);
      _vin.remove(30);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 35, height: 2, left: 25, right: 40, parent: null)|(data: 25, height: 1, left: null, right: null, parent: 35) (data: 40, height: 1, left: null, right: null, parent: 35)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(30);
      insert(40);
      insert(33);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 35, height: 1, left: null, right: null, parent: 33)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(30);
      insert(40);
      insert(27);
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 35, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(22);
      insert(40);
      insert(24);
      _vin.remove(15);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 22, right: 35, parent: null)|(data: 22, height: 2, left: 20, right: 24, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 22) (data: 24, height: 1, left: null, right: null, parent: 22) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(22);
      insert(40);
      insert(21);
      _vin.remove(15);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 21, right: 35, parent: null)|(data: 21, height: 2, left: 20, right: 22, parent: 25) (data: 35, height: 2, left: null, right: 40, parent: 25)|(data: 20, height: 1, left: null, right: null, parent: 21) (data: 22, height: 1, left: null, right: null, parent: 21) (null) (data: 40, height: 1, left: null, right: null, parent: 35)");
  }

   void test_one_child_removal() {
      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(30);
      insert(40);
      _vin.remove(30);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      _vin.remove(20);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 40, height: 1, left: null, right: null, parent: null)");

      clear();
      insert(25);
      insert(20);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 20, height: 1, left: null, right: null, parent: null)");

      clear();
      insert(25);
      insert(30);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 30, height: 1, left: null, right: null, parent: null)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(40);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 40, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 40, height: 1, left: null, right: null, parent: 25)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(30);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 25) (data: 30, height: 1, left: null, right: null, parent: 25)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(18);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 18, right: 35, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(23);
      _vin.remove(20);
      assert(_vin.bfs_str() == "(data: 25, height: 2, left: 23, right: 35, parent: null)|(data: 23, height: 1, left: null, right: null, parent: 25) (data: 35, height: 1, left: null, right: null, parent: 25)");
   }

   void test_two_children_removal() {
      clear();
      insert(25);
      insert(20);
      insert(35);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 35, height: 2, left: 20, right: null, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 35) (null)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(30);
      insert(40);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 40, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 40, height: 2, left: 30, right: null, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 40) (null)");

      clear();
      insert(25);
      insert(20);
      insert(30);
      insert(23);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 23, height: 2, left: 20, right: 30, parent: null)|(data: 20, height: 1, left: null, right: null, parent: 23) (data: 30, height: 1, left: null, right: null, parent: 23)");

      clear();
      insert(25);
      insert(20);
      insert(30);
      insert(18);
      _vin.remove(25);
      assert(_vin.bfs_str() == "(data: 20, height: 2, left: 18, right: 30, parent: null)|(data: 18, height: 1, left: null, right: null, parent: 20) (data: 30, height: 1, left: null, right: null, parent: 20)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(30);
      insert(40);
      insert(33);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 33, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 33, height: 2, left: 30, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 30, height: 1, left: null, right: null, parent: 33) (data: 40, height: 1, left: null, right: null, parent: 33)");

      clear();
      insert(25);
      insert(20);
      insert(35);
      insert(15);
      insert(30);
      insert(40);
      insert(27);
      _vin.remove(35);
      assert(_vin.bfs_str() == "(data: 25, height: 3, left: 20, right: 30, parent: null)|(data: 20, height: 2, left: 15, right: null, parent: 25) (data: 30, height: 2, left: 27, right: 40, parent: 25)|(data: 15, height: 1, left: null, right: null, parent: 20) (null) (data: 27, height: 1, left: null, right: null, parent: 30) (data: 40, height: 1, left: null, right: null, parent: 30)");
   }
};

int main () {
   TestIntVindex vin;

   vin.test_insert_left_left_right_right();
   vin.test_insert_left_right_right_left();
   vin.test_dne_removal();
   vin.test_leaf_removal();
   vin.test_one_child_removal();
   vin.test_two_children_removal();
}
