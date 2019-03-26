#include <iostream>
#include <memory>
#include <functional>
#include <map>
#include <algorithm>
#include <utility>
#include <deque>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <cassert>

using namespace std;

static int _dtoi(double val) {
   stringstream ss;
   ss << val;
   int val_i;
   ss >> val_i;
   return val_i;
}

static int _max(int a, int b) {
   return a > b ? a : b;
}

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
};

template<typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   int height;
   int depth;
   _AVLState *left;
   _AVLState *right;
   _AVLState *parent;

   _AVLState(BASE_TY data): 
      T(data), depth(0), height(0), 
      left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename T>
class Vindex {
private:
   typedef _AVLState<_Node<T>> _AVLNode;
   typedef unique_ptr<_AVLNode> NodeP;
   typedef map<_AVLNode *, NodeP> NodeCache;
   typedef typename NodeCache::iterator NodeCacheIter;
   typedef deque<_AVLNode *> NodeDQ;

   NodeCache _nodes;
   _AVLNode *_head;

   void _init_first_node(_AVLNode &n) {
      _head = &n;
   }

   int _height(_AVLNode *tree) {
      if (!tree)
         return 0;
      return _max(_height(tree->left), _height(tree->right)) + 1;
   }

   void _insert(_AVLNode &n, _AVLNode *subtree, _AVLNode *parent = nullptr) {
      ++n.depth;
      n.parent = subtree;

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

      subtree->height = 
         _max(_height(subtree->left), _height(subtree->right)) + 1;
   }

   int _nodes_at_lv(int lv) {
      if (lv < 1)
         throw invalid_argument("Error: lv must be greater than 0");   
      double val = pow(2, lv - 1);
      return _dtoi(val);
   }

   bool _is_dq_all_nulls(NodeDQ &dq) {
      auto it = find_if(
         dq.begin(), dq.end(), [](_AVLNode *n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_printed_per_line(
      NodeDQ &dq, int &curr_depth, int &nodes_printed) {
      ++curr_depth;
      nodes_printed = 0;
      if (_is_dq_all_nulls(dq))
         dq.clear();
      cout << endl;
   }

   void _on_valid_node(NodeDQ &dq, _AVLNode &n) {
      cout << _node_str(n) << " ";
      dq.push_back(n.left);
      dq.push_back(n.right);
   }

   void _on_null_node(NodeDQ &dq) {
      cout << "(null) ";
      dq.push_back(nullptr);
      dq.push_back(nullptr);
   }

   void _bfs_print(
      NodeDQ &dq, int &curr_depth, int &nodes_printed) {
      if (dq.empty()) return;

      _AVLNode *n = dq.front();
      dq.pop_front();

      if (n) _on_valid_node(dq, *n);
      else _on_null_node(dq);
      ++nodes_printed;

      if (nodes_printed == _nodes_at_lv(curr_depth))
         _on_max_nodes_printed_per_line(dq, curr_depth, nodes_printed);
      _bfs_print(dq, curr_depth, nodes_printed);
   }

   string _node_str(_AVLNode &n) {
      stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", height: " << n.height 
         << ", depth: " << n.depth 
         << ", left: " << (n.left ? to_string(n.left->data) : "null") 
         << ", right: " << (n.right ? to_string(n.right->data) : "null")
         << ", parent: " << (n.parent ? to_string(n.parent->data) : "null")
         << ")";
      return ss.str();
   }

   _AVLNode *_first_in_order(_AVLNode *n) {
      if (!n || !n->left) 
         return n;
      return _first_in_order(n->left);
   }

   _AVLNode *_next_in_order(_AVLNode *n) {
      if (!n) 
         return n;
      return _first_in_order(n->right);
   }

   _AVLNode *_find(_AVLNode *subtree, int val) {
      if (!subtree)
         return nullptr;

      if (val == subtree->data)
         return subtree;
      else if (val < subtree->data)
         return _find(subtree->left, val);
      else
         return _find(subtree->right, val);
   }

public:
   Vindex(): _head(nullptr) {}

   void insert(T val) {
      _AVLNode *n = new _AVLNode(val);
      _nodes[n] = NodeP(n);
      ++n->depth;
      ++n->height;
      _nodes.size() == 1 ? _init_first_node(*n) : _insert(*n, _head);
   }

   void remove(int val) {
      _AVLNode *rm = _find(_head, val);
      _AVLNode *next = _next_in_order(rm);
      if (rm) {
         cout << "found something to remove: " << rm->data << endl;
         if (next)
            cout << "next in order: " << next->data << endl;
         else
            cout << "next in order: null" << endl;
      }
      assert(false);
      // if (rm == _nodes.end())
      //    return;
      // _nodes.erase(rm);
   }

   void dump() {
      NodeDQ dq;
      int curr_depth = 1;
      int nodes_printed = 0;
      dq.push_back(_head);
      _bfs_print(dq, curr_depth, nodes_printed);
      cout << endl;
   }
};
