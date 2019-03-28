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
#include <string>

#include "helpers.h"
#include "custom_exceptions.h"

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
   // ~_Node() { cout << "removed node with data " << data << endl; }
};

template<typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   int height;
   _AVLState *left;
   _AVLState *right;
   _AVLState *parent;

   _AVLState *left_safe() {
      if (!left)
         throw NullPointerError();
      return left;
   }

   void *left_safe(_AVLState *n) {
      if (n == this)
         throw PointerToSelfError();
      left = n; 
   }

   _AVLState *right_safe() {
      if (!right)
         throw NullPointerError();
      return right;
   }

   void *right_safe(_AVLState *n) {
      if (n == this)
         throw PointerToSelfError();
      right = n; 
   }

   _AVLState *parent_safe() {
      if (!parent)
         throw NullPointerError();
      return parent;
   }

   void *parent_safe(_AVLState *n) {
      if (n == this)
         throw PointerToSelfError();
      parent = n; 
   }

   _AVLState(BASE_TY data): 
      T(data), height(0), 
      left(nullptr), right(nullptr), parent(nullptr) {}
};

enum _ChildDirection { LEFT, RIGHT, ROOT };

template<typename T>
class Vindex {
private:
   typedef _AVLState<_Node<T>> _AVLNode;
   typedef unique_ptr<_AVLNode> NodeP;
   typedef map<_AVLNode *, NodeP> NodeCache;
   typedef typename NodeCache::iterator NodeCacheIter;
   typedef deque<_AVLNode *> NodeDQ;
   typedef _ChildDirection Dxn;

   NodeCache _nodes;
   _AVLNode *_head;

   void _init_first_node(_AVLNode &n) {
      _head = &n;
   }

   int _height(_AVLNode *tree) {
      if (!tree)
         return 0;
      return max(_height(tree->left), _height(tree->right)) + 1;
   }

   void _insert(_AVLNode &n, _AVLNode *subtree, _AVLNode *parent = nullptr) {
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
         max(_height(subtree->left), _height(subtree->right)) + 1;
   }

   int _nodes_at_lv(int lv) {
      if (lv < 1)
         throw LessThanOneError();
      double val = pow(2, lv - 1);
      return dtoi(val);
   }

   bool _is_dq_all_nulls(NodeDQ &dq) {
      auto it = find_if(
         dq.begin(), dq.end(), [](_AVLNode *n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_printed_per_line(
      NodeDQ &dq, int &curr_depth, int &nodes_printed, 
      stringstream &ss, const string &row_delim = "\n") {
      ++curr_depth;
      nodes_printed = 0;
      if (_is_dq_all_nulls(dq))
         dq.clear();
      else
         ss << row_delim;
   }

   void _on_valid_node(
      NodeDQ &dq, _AVLNode &n, stringstream &ss, bool last_node) {
      ss << _node_str(n) << (last_node ? "" : " ");
      dq.push_back(n.left);
      dq.push_back(n.right);
   }

   void _on_null_node(NodeDQ &dq, stringstream &ss, bool last_node) {
      ss << "(null)" << (last_node ? "" : " ");
      dq.push_back(nullptr);
      dq.push_back(nullptr);
   }

   void _gather_bfs_str(
      NodeDQ &dq, int &curr_depth, int &nodes_printed, 
      stringstream &ss, const string &row_delim="\n") {
      if (dq.empty()) return;

      _AVLNode *n = dq.front();
      dq.pop_front();

      ++nodes_printed;
      bool last_node = nodes_printed == _nodes_at_lv(curr_depth);
      if (n) _on_valid_node(dq, *n, ss, last_node);
      else _on_null_node(dq, ss, last_node);

      if (last_node)
         _on_max_nodes_printed_per_line(
            dq, curr_depth, nodes_printed, ss, row_delim);

      _gather_bfs_str(dq, curr_depth, nodes_printed, ss, row_delim);
   }

   string _node_str(_AVLNode &n) {
      stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", height: " << n.height 
         << ", left: " << (n.left ? to_string(n.left->data) : "null") 
         << ", right: " << (n.right ? to_string(n.right->data) : "null")
         << ", parent: " << (n.parent ? to_string(n.parent->data) : "null")
         << ")";
      return ss.str();
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

   _AVLNode *_get_to_root_from(_AVLNode *node) {
      while (node->parent)
         node = node->parent;
      return node;
   }

   _AVLNode *_first_child(_AVLNode *n) {
      if (!n)
         throw NullPointerError();
      if (n->left)
         return n->left;
      else if (n->right)
         return n->right;
      else
         return nullptr;
   }

   Dxn _which_child(_AVLNode *n) {
      if (!n)
         throw NullPointerError();

      _AVLNode *parent = n->parent;

      if (!parent)
         return Dxn::ROOT;
      else if (parent->left == n)
         return Dxn::LEFT;
      else if (parent->right == n)
         return Dxn::RIGHT;
      else
         throw DetachedNodeError();
   }

   int _num_children(_AVLNode *n) {
      int cnt = 0;

      if (!n)
         return 0;
      if (n->left)
         ++cnt;
      if (n->right)
         ++cnt;
      return cnt;
   }

   bool _has_children(_AVLNode *n) {
      return _num_children(n) > 0;
   }

   _AVLNode *_on_removal_leaf(_AVLNode *n, bool erase = true) {
      if (!n) 
         throw NullPointerError();
      if (n->left || n->right)
         throw NotLeafError();

      if (erase)
         _nodes.erase(n);
      return nullptr;
   }

   _AVLNode *_detach_next_in_order(_AVLNode *tree, _AVLNode *&next) {
      if (tree->left) {
         tree->left = _detach_next_in_order(tree->left, next);
         tree->height = _height(tree);
         return tree;
      }
      else {
         next = tree;
         if (tree->right)
            return _on_removal_one_child(tree, false);
         else
            return _on_removal_leaf(next, false);
      }
   }

   void _rewire_next(_AVLNode *next, _AVLNode *temp) {
      next->left = temp->left;
      next->right = temp->right;
      next->parent = temp->parent;
      next->height = temp->height;   

      if (next->left)
         next->left->parent = next;
      if (next->right)
         next->right->parent = next;
   }

   _AVLNode *_on_removal_two_children(_AVLNode *n, bool erase = true) {
      if (!n) 
         throw NullPointerError();

      _AVLNode *next = nullptr;
      n->right = _detach_next_in_order(n->right, next);
      n->height = _height(n);

      if (!next)
         throw NullPointerError();
      _rewire_next(next, n);

      if (erase)
         _nodes.erase(n);
      return next;
   }

   _AVLNode *_on_removal_one_child(_AVLNode *n, bool erase = true) {
      if (!n)
         throw NullPointerError();
      if (_num_children(n) != 1)
         throw MustHaveExactlyOneChildError();

      _AVLNode *child = _first_child(n);
      _AVLNode *parent = n->parent;

      child->parent = parent ? parent : nullptr;
      if (erase)
         _nodes.erase(n);
      return child;
   }

   _AVLNode *_remove(int val, _AVLNode* tree) {
      if (val < tree->data)
         tree->left = _remove(val, tree->left);
      else if (val > tree->data)
         tree->right = _remove(val, tree->right);
      else {
         if (!tree)
            return tree;
         else if (_num_children(tree) == 1)
            tree = _on_removal_one_child(tree);
         else if (!_num_children(tree))
            tree = _on_removal_leaf(tree);
         else
            tree = _on_removal_two_children(tree);
      }

      if (tree)
         tree->height = _height(tree);
      return tree;
   }

public:
   Vindex(): _head(nullptr) {}

   void insert(T val) {
      _AVLNode *n = new _AVLNode(val);
      _nodes[n] = NodeP(n);
      ++n->height;
      _nodes.size() == 1 ? _init_first_node(*n) : _insert(*n, _head);
   }

   void remove(int val) {
      _head = _remove(val, _head);
   }

   string bfs_str(const string &delim = "|") {
      stringstream ss;
      NodeDQ dq;
      int curr_depth = 1;
      int nodes_printed = 0;
      if (_head == nullptr) return "";
      dq.push_back(_head);
      _gather_bfs_str(dq, curr_depth, nodes_printed, ss, delim);
      return ss.str();
   }

   void clear() {
      _nodes.clear();
      _head = nullptr;
   }
};
