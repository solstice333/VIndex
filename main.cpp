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

   void bfs_dump() {
      cout << _vin.bfs_str("\n") << endl << endl;
   }

   void bfs_dump_one_line() {
      cout << _vin.bfs_str() << endl << endl;
   }

   void test_insert_left_left_right_right() {
      insert(24);
      insert(20);
      insert(16);
      insert(12);
      insert(30);
      insert(35);
      assert(_vin.bfs_str() == "(data: 20, height: 3, left: 16, right: 30, parent: null)|(data: 16, height: 2, left: 12, right: null, parent: 20) (data: 30, height: 2, left: 24, right: 35, parent: 20)|(data: 12, height: 1, left: null, right: null, parent: 16) (null) (data: 24, height: 1, left: null, right: null, parent: 30) (data: 35, height: 1, left: null, right: null, parent: 30)");
   }

   void test_insert_left_right_right_left() {
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

   void test_leaf_removal() {
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 54) (data: 60, height: 2, left: 57, right: null, parent: 54)|(null) (null) (null) (null) (null) (null) (data: 57, height: 1, left: null, right: null, parent: 60) (null)");
      _vin.remove(57);
      assert(_vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 2, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 54) (data: 60, height: 1, left: null, right: null, parent: 54)");
      _vin.remove(30);
      assert(_vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 2, left: null, right: 60, parent: 24)|(null) (null) (null) (data: 60, height: 1, left: null, right: null, parent: 54)");
      _vin.remove(60);
      assert(_vin.bfs_str() == "(data: 24, height: 2, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 1, left: null, right: null, parent: 24)");
      _vin.remove(54);
      assert(_vin.bfs_str() == "(data: 24, height: 2, left: 2, right: null, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (null)");
      _vin.remove(2);
      assert(_vin.bfs_str() == "(data: 24, height: 1, left: null, right: null, parent: null)");
      _vin.remove(24);
      assert(_vin.bfs_str() == "");
   }

   void test_one_child_removal() {
      _vin.remove(60);
      assert(_vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: 57, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 54) (data: 57, height: 1, left: null, right: null, parent: 54)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
      _vin.remove(57);
      assert(_vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: null, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 54) (null)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
      _vin.remove(54);
      assert(_vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 30, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 30, height: 2, left: null, right: 40, parent: 24)|(null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30)");
      _vin.remove(2);
      assert(_vin.bfs_str() == "(data: 24, height: 3, left: null, right: 30, parent: null)|(null) (data: 30, height: 2, left: null, right: 40, parent: 24)|(null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30)");
      _vin.remove(24);
      assert(_vin.bfs_str() == "(data: 30, height: 2, left: null, right: 40, parent: null)|(null) (data: 40, height: 1, left: null, right: null, parent: 30)");
      _vin.remove(30);
      assert(_vin.bfs_str() == "(data: 40, height: 1, left: null, right: null, parent: null)");
      _vin.remove(40);
      assert(_vin.bfs_str() == "");
   }

   void test_clear() {
      _vin.clear();
   }

   void test_two_children_removal() {
      _vin.remove(54);
      assert(_vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 57, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 57, height: 3, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 57) (data: 60, height: 1, left: null, right: null, parent: 57)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
      _vin.remove(57);
      assert(_vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 60, height: 3, left: 30, right: null, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 60) (null)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
      _vin.remove(24);
      assert(_vin.bfs_str() == "(data: 30, height: 3, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 30) (data: 60, height: 2, left: 40, right: null, parent: 30)|(null) (null) (data: 40, height: 1, left: null, right: null, parent: 60) (null)");
      _vin.remove(30);
      assert(_vin.bfs_str() == "(data: 40, height: 2, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 40) (data: 60, height: 1, left: null, right: null, parent: 40)");
      _vin.remove(40);
      assert(_vin.bfs_str() == "(data: 60, height: 2, left: 2, right: null, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 60) (null)");
   }
};

int main () {
   TestIntVindex vin;

   vin.test_insert_left_left_right_right();
   vin.test_clear();
   vin.test_insert_left_right_right_left();
   // vin.test_leaf_removal();
   // vin.test_insert();
   // vin.test_one_child_removal();
   // vin.test_clear();
   // vin.test_insert();
   // vin.test_two_children_removal();
}
