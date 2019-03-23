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
   bool operator<(const _Node& n) { return data < n.data; }
   bool operator>(const _Node& n) { return data > n.data; }
   bool operator<=(const _Node& n) { return data <= n.data; }
   bool operator>=(const _Node& n) { return data >= n.data; }
   bool operator==(const _Node& n) { return data == n.data; }
   bool operator!=(const _Node& n) { return data != n.data; }
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
class Vindex {
private:
   typedef _AVLState<_Node<T>> _AVLNode;
   typedef unique_ptr<_AVLNode> NodeP;
   typedef unordered_set<NodeP> NodeSet;
   typedef typename NodeSet::iterator NodeSetIter;

   NodeSet _nodes;
   _AVLNode *_head;

   NodeSetIter _find_if(const function<bool(const NodeP &)> &pred) {
      return find_if(_nodes.begin(), _nodes.end(), pred);
   }

   NodeSetIter _find(T val) {
      return _find_if(
         [&val](const NodeP &n) -> bool { return val == n->data; });
   }

   void _init_first_node(_AVLNode &n) {
      n.height = 1;
      _head = &n;
   }

   void _insert(_AVLNode &n, _AVLNode *subtree, _AVLNode *parent = nullptr) {
      if (n < *subtree) {
         if (subtree->left)
            _insert(n, subtree->left, subtree);
         else
            subtree->left = &n;
      }
      else {
         if (subtree->right)
            _insert(n, subtree->right, subtree);
         else
            subtree->right = &n;
      }
   }

public:
   Vindex(): _head(nullptr) {}

   bool insert(T val) {
      pair<NodeSetIter, bool> inserted = 
         _nodes.emplace(new _AVLNode(val));

      if (!get<1>(inserted)) 
         return false;

      const NodeP &n = *get<0>(inserted);
      _nodes.size() == 1 ? _init_first_node(*n) : _insert(*n, _head);
      return true;
   }

   void remove(int val) {
      NodeSetIter rm = _find(val);
      if (rm == _nodes.end())
         return;
      _nodes.erase(rm);
   }

   void dump() {
      for (const NodeP &n : _nodes)
         cout << 
            "("
            << "addr:" << n.get()
            << ", data:" << n->data
            << ", height:" << n->height
            << ", left:" << n->left
            << ", right:" << n->right
            << ")" << endl;
   }
};
