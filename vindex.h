#include <iostream>
#include <memory>
#include <functional>
#include <unordered_set>
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
   typedef unordered_set<NodeP> NodeSet;
   typedef typename NodeSet::iterator NodeSetIter;
   typedef deque<_AVLNode *> NodeDQ;

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
      _head = &n;
   }

   void _insert(_AVLNode &n, _AVLNode *subtree, _AVLNode *parent = nullptr) {
      ++n.depth;
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
      cout << "null ";
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
         << ", depth: " << n.depth 
         << ", left: " << (n.left ? to_string(n.left->data) : "null") 
         << ", right: " << (n.right ? to_string(n.right->data) : "null")
         << ")";
      return ss.str();
   }

public:
   Vindex(): _head(nullptr) {}

   bool insert(T val) {
      pair<NodeSetIter, bool> inserted = 
         _nodes.emplace(new _AVLNode(val));

      bool emplace_success = get<1>(inserted);
      if (!emplace_success) return false;

      const NodeP &n = *get<0>(inserted);
      ++n->depth;
      ++n->height;
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
      NodeDQ dq;
      int curr_depth = 1;
      int nodes_printed = 0;
      dq.push_back(_head);
      _bfs_print(dq, curr_depth, nodes_printed);
      cout << endl;
   }
};
