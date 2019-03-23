#include <iostream>
#include <memory>
#include <functional>
#include <unordered_set>
#include <algorithm>
#include <utility>

using namespace std;

template<typename T>
struct _Node {
   T data;
   _Node(int data = 0): data(data) {}
   ~_Node() { cout << "deleting node with " << data << endl; }
};

template<typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   int height;
   _AVLState *left;
   _AVLState *right;
   _AVLState *parent;

   _AVLState(BASE_TY data): 
      T(data), height(0), left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename T>
class AVL {
private:
   typedef _AVLState<_Node<T>> _AVLNode;
   typedef unique_ptr<_AVLNode> Node;
   typedef unordered_set<Node> NodeSet;
   typedef typename NodeSet::iterator NodeSetIter;

   NodeSet _nodes;

   NodeSetIter _find_if(const function<bool(const Node &)> &pred) {
      return find_if(_nodes.begin(), _nodes.end(), pred);
   }

   NodeSetIter _find(T val) {
      return _find_if(
         [&val](const Node &n) -> bool { return val == n->data; });
   }

   void _init_first_node(const Node &n) {
      n->height = 1;
   }

public:
   bool insert(T val) {
      pair<NodeSetIter, bool> inserted = 
         _nodes.emplace(new _AVLNode(val));

      if (!get<1>(inserted)) 
         return false;

      const Node &n = *get<0>(inserted);

      if (_nodes.size() == 1)
         _init_first_node(n);

      return true;
   }

   void remove(int val) {
      NodeSetIter rm = _find(val);
      if (rm == _nodes.end())
         return;
      _nodes.erase(rm);
   }

   void dump() {
      for (const Node &n : _nodes)
         cout << 
            "(data:" << n->data 
            << ", height:" << n->height 
            << ")" << endl;
   }
};
