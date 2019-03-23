#include "vindex.h"

using namespace std;

int main () {
   AVL<int> avl;
   avl.insert(24);
   avl.insert(54);
   avl.insert(2);
   avl.dump();
   avl.remove(54);
}
