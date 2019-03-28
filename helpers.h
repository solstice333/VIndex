#include <sstream>

using namespace std;

int dtoi(double val) {
   stringstream ss;
   ss << val;
   int val_i;
   ss >> val_i;
   return val_i;
}

int max(int a, int b) {
   return a > b ? a : b;
}

string itos(int x) {
   stringstream ss;
   ss << x;
   return ss.str();
}
