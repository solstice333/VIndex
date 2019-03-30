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
   _Node(const T& data): data(data) {}
   bool operator<(const _Node& n) { return data < n.data; }
   bool operator>(const _Node& n) { return data > n.data; }
   bool operator<=(const _Node& n) { return data <= n.data; }
   bool operator>=(const _Node& n) { return data >= n.data; }
   bool operator==(const _Node& n) { return data == n.data; }
   bool operator!=(const _Node& n) { return data != n.data; }
};

template<typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   typedef unique_ptr<_AVLState> AVLStateOwner;

   int height;
   AVLStateOwner left;
   AVLStateOwner right;
   _AVLState *parent;

   _AVLState *left_raw() {
      return left.get();
   }

   _AVLState *right_raw() {
      return right.get();
   }

   _AVLState(BASE_TY data): 
      T(data), height(0), 
      left(nullptr), right(nullptr), parent(nullptr) {}
};

enum _ChildDirection { LEFT, RIGHT, ROOT };

template<typename T>
class Vindex {
private:
   typedef _AVLState<_Node<T>> AVLNode;
   typedef unique_ptr<AVLNode> AVLNodeOwner;
   typedef map<AVLNode *, AVLNodeOwner> NodeCache;
   typedef typename NodeCache::iterator NodeCacheIter;
   typedef deque<AVLNode *> NodeDQ;
   typedef _ChildDirection Dxn;

   AVLNodeOwner _head;

   AVLNode *_head_raw() {
      return _head.get();
   }

   static void _detach_from_owner(AVLNodeOwner &owner) {
      owner.release();
   }

   int _height(AVLNode *tree) {
      if (!tree)
         return 0;
      return max(_height(tree->left_raw()), _height(tree->right_raw())) + 1;
   }

   void _insert(AVLNode *n, AVLNode *subtree, AVLNode *parent = nullptr) {
      n->parent = subtree;

      if (*n < *subtree) {
         if (subtree->left)
            _insert(n, subtree->left_raw(), subtree);
         else
            subtree->left = AVLNodeOwner(n);
      }
      else {
         if (subtree->right)
            _insert(n, subtree->right_raw(), subtree);
         else
            subtree->right = AVLNodeOwner(n);
      }

      subtree->height = 
         max(_height(subtree->left_raw()), _height(subtree->right_raw())) + 1;
   }

   int _nodes_at_lv(int lv) {
      if (lv < 1)
         throw LessThanOneError();
      double val = pow(2, lv - 1);
      return dtoi(val);
   }

   bool _is_dq_all_nulls(NodeDQ &dq) {
      auto it = find_if(
         dq.begin(), dq.end(), [](AVLNode *n) -> bool { return n; });
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
      NodeDQ &dq, AVLNode &n, stringstream &ss, bool last_node) {
      ss << _node_str(n) << (last_node ? "" : " ");
      dq.push_back(n.left_raw());
      dq.push_back(n.right_raw());
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

      AVLNode *n = dq.front();
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

   string _node_str(AVLNode &n) {
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

   AVLNode *_find(AVLNode *subtree, const T& val) {
      if (!subtree)
         return nullptr;

      if (val == subtree->data)
         return subtree;
      else if (val < subtree->data)
         return _find(subtree->left_raw(), val);
      else
         return _find(subtree->right_raw(), val);
   }

   AVLNode *_get_to_root_from(AVLNode *node) {
      while (node->parent)
         node = node->parent;
      return node;
   }

   AVLNode *_first_child(AVLNode *n) {
      if (!n)
         throw NullPointerError();
      if (n->left)
         return n->left_raw();
      else if (n->right)
         return n->right_raw();
      else
         return nullptr;
   }

   Dxn _which_child(AVLNode *n) {
      if (!n)
         throw NullPointerError();

      AVLNode *parent = n->parent;

      if (!parent)
         return Dxn::ROOT;
      else if (parent->left_raw() == n)
         return Dxn::LEFT;
      else if (parent->right_raw() == n)
         return Dxn::RIGHT;
      else
         throw DetachedNodeError();
   }

   int _num_children(AVLNode *n) {
      int cnt = 0;

      if (!n)
         return 0;
      if (n->left)
         ++cnt;
      if (n->right)
         ++cnt;
      return cnt;
   }

   bool _has_children(AVLNode *n) {
      return _num_children(n) > 0;
   }

   void _act_with_child_owner(
      AVLNode *child, const function<void(AVLNodeOwner&)> &action) {
      AVLNode *parent = child->parent;
      Dxn child_ty = _which_child(child);

      if (child_ty == Dxn::LEFT)
         action(parent->left);
      else if (child_ty == Dxn::RIGHT)
         action(parent->right);
      else if (child_ty == Dxn::ROOT)
         action(_head);
      else
         throw DetachedNodeError();
   } 

   void _assign_if_diff(AVLNodeOwner &o, AVLNode *n) {
      if (o.get() != n)
         o = AVLNodeOwner(n);
   }

   // AVLNode *_detach_next_in_order(AVLNode *tree, AVLNode *&next) {
   //    if (tree->left) {
   //       tree->left = _detach_next_in_order(tree->left, next);
   //       tree->height = _height(tree);
   //       return tree;
   //    }
   //    else {
   //       next = tree;
   //       if (tree->right)
   //          return _on_removal_one_child(tree, true);
   //       else
   //          return _on_removal_leaf(next, true);
   //    }
   // }

   // void _rewire_next(AVLNode *next, AVLNode *temp) {
   //    next->left = temp->left;
   //    next->right = temp->right;
   //    next->parent = temp->parent;
   //    next->height = temp->height;   

   //    if (next->left)
   //       next->left->parent = next;
   //    if (next->right)
   //       next->right->parent = next;
   // }

   // AVLNode *_on_removal_two_children(AVLNode *n, bool erase = true) {
   //    if (!n) 
   //       throw NullPointerError();

   //    AVLNode *next = nullptr;
   //    n->right = _detach_next_in_order(n->right, next);
   //    n->height = _height(n);

   //    if (!next)
   //       throw NullPointerError();
   //    _rewire_next(next, n);

   //    if (erase)
   //       _nodes.erase(n);
   //    return next;
   // }

   AVLNode *_on_removal_leaf(AVLNode *n, bool detach = false) {
      if (!n) 
         throw NullPointerError();
      if (n->left || n->right)
         throw NotLeafError();
      if (detach)
         _act_with_child_owner(n, _detach_from_owner);

      return nullptr;
   }

   AVLNode *_on_removal_one_child(AVLNode *n, bool detach = false) {
      if (!n)
         throw NullPointerError();
      if (_num_children(n) != 1)
         throw MustHaveExactlyOneChildError();

      AVLNode *child = _first_child(n);
      AVLNode *parent = n->parent;
      _act_with_child_owner(child, _detach_from_owner);
      child->parent = parent ? parent : nullptr;

      if (detach)
         _act_with_child_owner(n, _detach_from_owner);

      return child;
   }

   AVLNode *_remove(const T& val, AVLNode* tree) {
      if (val < tree->data)
         _assign_if_diff(tree->left, _remove(val, tree->left_raw()));
      else if (val > tree->data)
         _assign_if_diff(tree->right, _remove(val, tree->right_raw()));
      else {
         if (!tree)
            return tree;
         else if (_num_children(tree) == 1)
            tree = _on_removal_one_child(tree);
         else if (!_num_children(tree))
            tree = _on_removal_leaf(tree);
         else
            throw NotYetImplementedError();
            // tree = _on_removal_two_children(tree);
      }

      if (tree)
         tree->height = _height(tree);
      return tree;
   }

public:
   Vindex(): _head(nullptr) {}

   void insert(const T& val) {
      AVLNode *n = new AVLNode(val);
      ++n->height;

      if (!_head) {
         _head = AVLNodeOwner(n);
         return;
      }
      _insert(n, _head_raw());
   }

   void remove(const T& val) {
      _assign_if_diff(_head, _remove(val, _head_raw()));
   }

   string bfs_str(const string &delim = "|") {
      stringstream ss;
      NodeDQ dq;
      int curr_depth = 1;
      int nodes_printed = 0;
      if (_head_raw() == nullptr) return "";
      dq.push_back(_head_raw());
      _gather_bfs_str(dq, curr_depth, nodes_printed, ss, delim);
      return ss.str();
   }

   void clear() {
      _head = nullptr;
   }
};
