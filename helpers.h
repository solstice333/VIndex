#include <sstream>

int dtoi(double val) {
   using namespace std;
   stringstream ss;
   ss << val;
   int val_i;
   ss >> val_i;
   return val_i;
}

int max(int a, int b) {
   return a > b ? a : b;
}

std::string itos(int x) {
   using namespace std;
   stringstream ss;
   ss << x;
   return ss.str();
}
