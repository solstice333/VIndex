#include "vindex.h"

using namespace std;

int main () {
   Vindex<int> vin;
   vin.insert(24);
   vin.insert(54);
   vin.insert(2);
   vin.dump();
   vin.remove(54);
}
