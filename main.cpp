#include "vindex.h"

using namespace std;

int main () {
   Vindex<int> vin;
   vin.insert(24);
   vin.insert(54);
   // vin.insert(2);
   // vin.insert(30);
   // vin.insert(60);
   // vin.insert(62);
   vin.dump();
   // vin.remove(54);
}
