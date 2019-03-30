#include "vindex.h"
#include <cassert>

using namespace std;

typedef Vindex<int> IntVindex;

static void bfs_dump(IntVindex &vin) {
   cout << vin.bfs_str("\n") << endl << endl;
}

static void bfs_dump_one_line(IntVindex &vin) {
   cout << vin.bfs_str() << endl << endl;
}

static void test_insert(IntVindex &vin) {
   vin.insert(24);
   vin.insert(54);
   vin.insert(2);
   vin.insert(30);
   vin.insert(40);
   vin.insert(60);
   vin.insert(57);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 54) (data: 60, height: 2, left: 57, right: null, parent: 54)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (data: 57, height: 1, left: null, right: null, parent: 60) (null)");
}

static void test_leaf_removal(IntVindex &vin) {
   vin.remove(40);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 54) (data: 60, height: 2, left: 57, right: null, parent: 54)|(null) (null) (null) (null) (null) (null) (data: 57, height: 1, left: null, right: null, parent: 60) (null)");
   vin.remove(57);
   assert(vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 2, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 1, left: null, right: null, parent: 54) (data: 60, height: 1, left: null, right: null, parent: 54)");
   vin.remove(30);
   assert(vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 2, left: null, right: 60, parent: 24)|(null) (null) (null) (data: 60, height: 1, left: null, right: null, parent: 54)");
   vin.remove(60);
   assert(vin.bfs_str() == "(data: 24, height: 2, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 1, left: null, right: null, parent: 24)");
   vin.remove(54);
   assert(vin.bfs_str() == "(data: 24, height: 2, left: 2, right: null, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (null)");
   vin.remove(2);
   assert(vin.bfs_str() == "(data: 24, height: 1, left: null, right: null, parent: null)");
   vin.remove(24);
   assert(vin.bfs_str() == "");
}

static void test_one_child_removal(IntVindex &vin) {
   vin.remove(60);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: 57, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 54) (data: 57, height: 1, left: null, right: null, parent: 54)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
   vin.remove(57);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 54, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 54, height: 3, left: 30, right: null, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 54) (null)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
   vin.remove(54);
   assert(vin.bfs_str() == "(data: 24, height: 3, left: 2, right: 30, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 30, height: 2, left: null, right: 40, parent: 24)|(null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30)");
   vin.remove(2);
   assert(vin.bfs_str() == "(data: 24, height: 3, left: null, right: 30, parent: null)|(null) (data: 30, height: 2, left: null, right: 40, parent: 24)|(null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30)");
   vin.remove(24);
   assert(vin.bfs_str() == "(data: 30, height: 2, left: null, right: 40, parent: null)|(null) (data: 40, height: 1, left: null, right: null, parent: 30)");
   vin.remove(30);
   assert(vin.bfs_str() == "(data: 40, height: 1, left: null, right: null, parent: null)");
   vin.remove(40);
   assert(vin.bfs_str() == "");
}

static void test_clear(IntVindex &vin) {
   vin.clear();
}

static void test_two_children_removal(IntVindex &vin) {
   vin.remove(54);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 57, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 57, height: 3, left: 30, right: 60, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 57) (data: 60, height: 1, left: null, right: null, parent: 57)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
   vin.remove(57);
   assert(vin.bfs_str() == "(data: 24, height: 4, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 24) (data: 60, height: 3, left: 30, right: null, parent: 24)|(null) (null) (data: 30, height: 2, left: null, right: 40, parent: 60) (null)|(null) (null) (null) (null) (null) (data: 40, height: 1, left: null, right: null, parent: 30) (null) (null)");
   vin.remove(24);
   assert(vin.bfs_str() == "(data: 30, height: 3, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 30) (data: 60, height: 2, left: 40, right: null, parent: 30)|(null) (null) (data: 40, height: 1, left: null, right: null, parent: 60) (null)");
   vin.remove(30);
   assert(vin.bfs_str() == "(data: 40, height: 2, left: 2, right: 60, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 40) (data: 60, height: 1, left: null, right: null, parent: 40)");
   vin.remove(40);
   assert(vin.bfs_str() == "(data: 60, height: 2, left: 2, right: null, parent: null)|(data: 2, height: 1, left: null, right: null, parent: 60) (null)");
}

int main () {
   IntVindex vin;

   test_insert(vin);
   test_leaf_removal(vin);
   // test_insert(vin);
   // test_one_child_removal(vin);
   // test_clear(vin);
   // test_insert(vin);
   // test_two_children_removal(vin);
}
