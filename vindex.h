#ifndef VINDEX_H
#define VINDEX_H

#include <iostream>
#include <memory>
#include <functional>
#include <map>
#include <algorithm>
#include <utility>
#include <deque>
#include <cmath>
#include <sstream>
#include <string>
#include <list>
#include <functional>
#include <mutex>

#define DEPTH_DATA_ENABLED 0

#ifdef NDEBUG
#define assert(condition, message) 0
#else
#define assert(condition, message)\
   (!(condition)) ?\
      (std::cerr << "Assertion failed: (" << #condition << "), "\
      << "function " << __FUNCTION__\
      << ", file " << __FILE__\
      << ", line " << __LINE__ << "."\
      << std::endl << message << std::endl, abort(), 0) : 1
#endif

template <typename T, typename DerivedTy>
class Singleton {
private:
   static T resource;
   static std::once_flag flag;

public:
   static void init(T *resource) {
      DerivedTy::init(resource);
   };

   virtual T& get() {
      std::call_once(flag, init, &resource);
      return resource;
   } 
};

template <typename T, typename DerivedTy>
std::once_flag Singleton<T, DerivedTy>::flag;

template <typename T, typename DerivedTy>
T Singleton<T, DerivedTy>::resource;

template <typename T>
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

template <typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   typedef std::unique_ptr<_AVLState> AVLStateOwner;

   size_t height;
   size_t depth;
   AVLStateOwner left;
   AVLStateOwner right;
   _AVLState *parent;

   _AVLState *left_raw() const {
      return left.get();
   }

   _AVLState *right_raw() const {
      return right.get();
   }

   _AVLState(BASE_TY data): 
      T(data), height(0), depth(0), 
      left(nullptr), right(nullptr), parent(nullptr) {}

   _AVLState(): 
      T(BASE_TY()), height(0), depth(0),
      left(nullptr), right(nullptr), parent(nullptr) {}
};

namespace Direction {
   enum Direction { LEFT, RIGHT, ROOT };
}

namespace OrderType {
   enum OrderType { INORDER, PREORDER, POSTORDER, BREADTHFIRST, INSERTION };
}

template <typename T>
class Vindex {
public:
   class const_iterator;
   class const_reverse_iterator;

private:
   typedef _AVLState<_Node<T>> AVLNode;
   typedef std::unique_ptr<AVLNode> AVLNodeOwner;
   typedef std::map<AVLNode *, AVLNodeOwner> NodeCache;
   typedef typename NodeCache::iterator NodeCacheIter;
   typedef std::deque<AVLNode *> NodeDQ;
   typedef std::list<AVLNode *> NodeList;
   typedef std::function<void(AVLNode *)> NodeListener;
   typedef std::function<void()> VoidFunc;
   typedef Direction::Direction Direction;
   typedef OrderType::OrderType OrderType;
   typedef std::function<AVLNodeOwner(AVLNodeOwner *)> TreeEditAction;
   typedef std::map<OrderType, std::string> OrderTypeToStr;

   class OrderTypeToStrSingleton : 
      public Singleton<OrderTypeToStr, OrderTypeToStrSingleton> {
   public:
      static void init(OrderTypeToStr *order_ty_to_str) {
         (*order_ty_to_str)[OrderType::INORDER] = "INORDER";
         (*order_ty_to_str)[OrderType::PREORDER] = "PREORDER";
         (*order_ty_to_str)[OrderType::POSTORDER] = "POSTORDER";
         (*order_ty_to_str)[OrderType::BREADTHFIRST] = "BREADTHFIRST";
         (*order_ty_to_str)[OrderType::INSERTION] = "INSERTION";
      }
   }; 

   class AVLNodeDefaultSingleton :
      public Singleton<AVLNode, AVLNodeDefaultSingleton> {
   public:
      static void init(AVLNode *) {}
   };

   template <bool reverse>
   class _const_iterator : 
      public std::iterator<std::bidirectional_iterator_tag, T> {
   private:
      typedef Vindex::AVLNode AVLNode;
      typedef Vindex::AVLNodeOwner AVLNodeOwner;
      typedef Vindex::Direction Direction;
      typedef typename Vindex::NodeList::iterator NodeListIter;
      typedef typename Vindex::NodeList::reverse_iterator NodeListRevIter;

      AVLNode *_curr;
      AVLNode *_prev;
      bool _prev_incr;
      OrderType _order_ty;

      size_t _curr_lv;
      size_t _prev_lv;

      // TODO change to IteratorTracker object that has begin, end, curr 
      // properties of type 
      // std::conditional<reverse, NodeListRevIter, NodeListIter>
      NodeListIter _insert_iter;
      NodeListRevIter _insert_riter;
      NodeListIter _insert_end;
      NodeListRevIter _insert_rend;
      NodeListIter _insert_begin;
      NodeListRevIter _insert_rbegin;

      AVLNode *_get_leftest_node(AVLNode *tree) {
         return tree->left ? _get_leftest_node(tree->left_raw()) : tree;
      }

      AVLNode *_get_rightest_node(AVLNode *tree) {
         return tree->right ? _get_rightest_node(tree->right_raw()) : tree;
      }

      AVLNode *_get_right_deepest_node(AVLNode *tree) {
         if (tree->right)
            return _get_right_deepest_node(tree->right_raw());
         else if (tree->left)
            return _get_right_deepest_node(tree->left_raw());
         else
            return tree;
      }

      AVLNode *_get_deepest_right_node_recurs(
         AVLNode *tree, size_t *curr_lv=nullptr) {
         AVLNode *left = tree->left_raw();
         AVLNode *right = tree->right_raw();

         if (curr_lv)
            ++*curr_lv;

         if (left && right) {
            return right->height >= left->height ?
               _get_deepest_right_node_recurs(right, curr_lv) :
               _get_deepest_right_node_recurs(left, curr_lv);
         }
         else if (right)
            return _get_deepest_right_node_recurs(right, curr_lv);
         else if (left)
            return _get_deepest_right_node_recurs(left, curr_lv);
         else
            return tree;
      }

      AVLNode *_get_deepest_right_node(
         AVLNode *tree, size_t *curr_lv=nullptr) {
         if (curr_lv)
            *curr_lv = 0;
         return _get_deepest_right_node_recurs(tree, curr_lv);
      }

      AVLNode *_get_root_node(AVLNode *tree) {
         return tree;
      }

      AVLNode *_get_left_sibling(AVLNode *tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode *parent = tree->parent;
         return parent->right_raw() == tree ? parent->left_raw() : nullptr;
      }

      AVLNode *_get_right_sibling(AVLNode *tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode *parent = tree->parent;
         return parent->left_raw() == tree ? parent->right_raw() : nullptr;
      }

      bool _visited_subtree(AVLNode *subtree) {
         AVLNode *child = subtree; 

         while (child) {
            if (child == _prev)
               return true;
            child = *_prev < *child ? child->left_raw() : child->right_raw();
         }
         return false;
      }

      bool _visited_left_subtree() {
         return _curr ? _visited_subtree(_curr->left_raw()) : false; 
      }

      bool _visited_right_subtree() {
         return _curr ? _visited_subtree(_curr->right_raw()) : false; 
      }

      bool _visited_sibling_subtree() {
         if (!_curr || !_curr->parent)
            return false;

         AVLNode *parent = _curr->parent;
         return parent->left_raw() == _curr ?
            _visited_subtree(parent->right_raw()) :
            _visited_subtree(parent->left_raw());
      }

      bool _visited_parent() {
         if (!_curr)
            return false;

         AVLNode *parent = _curr->parent;
         while (parent) {
            if (_prev == parent)
               return true;
            parent = parent->parent;
         }
         return false;
      }

      AVLNode *_retrace_while_child(
         AVLNode *n, Direction dir, size_t *parent_distance=nullptr) {
         assert(n && n->parent, "NullPointerError");

         size_t dist = 0;
         AVLNode *parent = n->parent;
         AVLNodeOwner *parents_child = dir == Direction::RIGHT ? 
            &parent->right : &parent->left;

         while (parent && parents_child->get() == n) {
            ++dist;
            n = n->parent;
            parent = n->parent;
            parents_child = dir == Direction::RIGHT ? 
               &parent->right : &parent->left;
         }
         if (parent_distance)
            *parent_distance = dist + 1;
         return parent;
      }

      AVLNode *_retrace_until_unvisited_right_child() {
         AVLNode *n = _retrace_while_child(_curr, Direction::RIGHT);
         return n ? (n->right ? n->right_raw() : nullptr) : nullptr;
      }

      AVLNode *_retrace_until_left_child() {
         assert(_curr && _curr->parent, "NullPointerError");

         AVLNode *curr = _curr;
         AVLNode *parent = curr->parent;

         while (parent && !parent->left) {
            curr = curr->parent;
            parent = curr->parent;
         }
         return parent->left_raw();
      }

      AVLNode *_retrace_until_unvisited_left_child() {
         AVLNode *n = _retrace_while_child(_curr, Direction::LEFT);
         return n ? (n->left ? n->left_raw() : nullptr) : nullptr;
      }

      void _in_order_increment() {
         AVLNode *tmp = _curr;
         if (!_prev_incr)
            _curr = _prev;   
         else if (_visited_left_subtree() || _visited_parent() || !_prev)
            _curr = tmp->right ? 
               _get_leftest_node(tmp->right_raw()) : 
               _retrace_while_child(_curr, Direction::RIGHT);
         else if (_visited_right_subtree())
            assert(false, "InvalidAdvanceStateError");
         else if (!_curr)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _in_order_decrement() {
         AVLNode *tmp = _curr;
         if (_prev_incr)
            _curr = _prev;
         else if (_visited_right_subtree() || _visited_parent() || !_prev)
            _curr = tmp->left ?
               _get_rightest_node(tmp->left_raw()) : 
               _retrace_while_child(_curr, Direction::LEFT);
         else if (!_curr)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _pre_order_increment() {
         AVLNode *tmp = _curr;   

         if (!_prev_incr)
            _curr = _prev;
         else if (tmp) {
            if (tmp->left)
               _curr = _get_root_node(tmp->left_raw());
            else if (tmp->right)
               _curr = _get_root_node(tmp->right_raw());
            else
               _curr = _retrace_until_unvisited_right_child();
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _pre_order_decrement() {
         AVLNode *tmp = _curr;

         if (_prev_incr)
            _curr = _prev;
         else if (tmp) { 
            if (tmp->parent && tmp->parent->right_raw() == tmp) {
               AVLNode *left_sibling = _get_left_sibling(tmp);
               _curr = left_sibling ? 
                  _get_right_deepest_node(left_sibling) : tmp->parent;
            }
            else if (!tmp->parent || tmp->parent->left_raw() == tmp)
               _curr = tmp->parent;
            else
               assert(false, "InvalidAdvanceStateError");
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _post_order_increment() {
         AVLNode *tmp = _curr;

         if (!_prev_incr)
            _curr = _prev;
         else if (tmp) {
            if (tmp->parent && tmp->parent->left_raw() == tmp) {
               AVLNode *right_sibling = _get_right_sibling(tmp);
               _curr = right_sibling ?
                  _get_leftest_node(right_sibling) : tmp->parent;
            }
            else if (!tmp->parent || tmp->parent->right_raw() == tmp)
               _curr = tmp->parent;
            else
               assert(false, "InvalidAdvanceStateError");
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _post_order_decrement() {
         AVLNode *tmp = _curr;

         if (_prev_incr)
            _curr = _prev;
         else if (tmp) {
            if (tmp->right)
               _curr = _get_root_node(tmp->right_raw());
            else if (tmp->left)
               _curr = _get_root_node(tmp->left_raw());
            else if (tmp->parent && tmp->parent->right_raw() == tmp)
               _curr = _get_root_node(_retrace_until_left_child());
            else if (tmp->parent && tmp->parent->left_raw() == tmp)
               _curr = _get_root_node(_retrace_until_unvisited_left_child());
            else
               assert(false, "InvalidAdvanceStateError");
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      AVLNode *_get_node_at_depth(
         AVLNode *tree, size_t tree_lv, size_t want_lv, Direction dir) {
         if (!tree || tree_lv > want_lv)
            return nullptr;
         else if (tree_lv == want_lv)
            return tree;
         else if (tree_lv < want_lv) {
            AVLNode *n = _get_node_at_depth(
               dir == Direction::RIGHT ? tree->right_raw() : tree->left_raw(), 
               tree_lv + 1, want_lv, dir);
            if (!n)
               n = _get_node_at_depth(
                  dir == Direction::RIGHT ? 
                     tree->left_raw() : tree->right_raw(), 
                  tree_lv + 1, want_lv, dir);
            return n;
         }
         else
            assert(false, "InvalidOperationError");
      }

      AVLNode *_get_leftest_node_at_depth(
         AVLNode *tree, size_t tree_lv, size_t want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::LEFT);
      }

      AVLNode *_get_rightest_node_at_depth(
         AVLNode *tree, size_t tree_lv, size_t want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::RIGHT);
      }

      void _get_next_sibling_positions(
         AVLNode *parent, Direction dir, AVLNode **pos1, AVLNode **pos2) {
         if (dir == Direction::RIGHT) {
            *pos1 = parent->left_raw();
            *pos2 = parent->right_raw();
         }
         else if (dir == Direction::LEFT) {
            *pos1 = parent->right_raw();
            *pos2 = parent->left_raw();
         }
         else
            assert(false, "InvalidOperationError");
      }

      AVLNode *_get_next_sibling(AVLNode *n, size_t depth, Direction dir) {
         assert(n, "NullPointerError");

         AVLNode *parent = n->parent;

         if (parent) {
            AVLNode *pos1 = nullptr;
            AVLNode *pos2 = nullptr;
            _get_next_sibling_positions(parent, dir, &pos1, &pos2);

            if (pos1 == n && pos2)
               return pos2;
            else if (pos2 == n) {
               size_t parent_distance = 0;
               parent = _retrace_while_child(n, dir, &parent_distance);

               return parent ? 
                  (dir == Direction::RIGHT ?
                     _get_leftest_node_at_depth(
                        parent->right_raw(),
                        depth - parent_distance + 1,
                        depth) :
                     _get_rightest_node_at_depth(
                        parent->left_raw(),
                        depth - parent_distance + 1,
                        depth)
                  ) : nullptr;
            }
            else
               assert(false, "InvalidOperationError");
         }
         else
            return nullptr;
      }

      AVLNode *_get_start_node(AVLNode *refnode, size_t depth, Direction dir) {
         AVLNode *n = refnode;
         AVLNode *prev = nullptr;
         assert(dir != Direction::ROOT, "InvalidDirectionError");

         do {
            prev = n;
         } while (n = _get_next_sibling(n, depth, dir));
         return prev;
      }

      AVLNode *_get_first_node_on_lower_lv(AVLNode *refnode) {
         ++_curr_lv;
         AVLNode *n = _get_start_node(refnode, _curr_lv - 1, Direction::LEFT);

         do {
            AVLNode *first = n->left_raw();
            AVLNode *second = n->right_raw();

            if (first)
               return first;
            else if (second)
               return second;
         } while (n = _get_next_sibling(n, _curr_lv - 1, Direction::RIGHT));

         --_curr_lv;
         return nullptr;
      }

      AVLNode *_get_first_node_on_upper_lv(AVLNode *refnode) {
         if (refnode) {
            --_curr_lv;
            return _get_start_node(refnode, _curr_lv - 1, Direction::RIGHT);
         }
         else
            return nullptr;
      }

      void _swap(size_t *a, size_t *b) {
         size_t tmp = *a;
         *a = *b;
         *b = tmp;
      }

      void _breadth_first_increment() {
         AVLNode *tmp = _curr;

         if (!_prev_incr) {
            _curr = _prev;
            _swap(&_curr_lv, &_prev_lv);
         }
         else if (tmp) {
            _curr = _get_next_sibling(tmp, _curr_lv, Direction::RIGHT);
            if (!_curr) {
               _prev_lv = _curr_lv;
               _curr = _get_first_node_on_lower_lv(tmp);
            }
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      void _breadth_first_decrement() {
         AVLNode *tmp = _curr;

         if (_prev_incr) {
            _curr = _prev;
            _swap(&_curr_lv, &_prev_lv);
         }
         else if (tmp) {
            _curr = _get_next_sibling(tmp, _curr_lv, Direction::LEFT);
            if (!_curr) {
               _prev_lv = _curr_lv;
               _curr = _get_first_node_on_upper_lv(tmp->parent);
            }
         }
         else if (!tmp)
            return;
         else
            assert(false, "InvalidAdvanceStateError");

         _prev = tmp;
      }

      // TODO change ITER_TY to 
      // std::conditional<reverse, NodeListRevIter, NodeListIter>
      template <typename ITER_TY>
      void _insertion_order_advance(
         ITER_TY *iter, ITER_TY *begin, ITER_TY *end, bool towards_end) {
         if (towards_end) {
            if (*iter == *end)
               _curr = nullptr;
            else if (!_curr)
               _curr = **iter;
            else {
               ++*iter;
               _curr = *iter == *end ? nullptr : **iter;
            }
         }
         else
            _curr = *iter == *begin ? nullptr : *--*iter;
      }

      // TODO just pass in an IteratorTracker
      void _insertion_order_increment() {
         if (reverse)
            _insertion_order_advance<NodeListRevIter>(
               &_insert_riter, &_insert_rbegin, &_insert_rend, 
               /*towards_end=*/false);
         else
            _insertion_order_advance<NodeListIter>(
               &_insert_iter, &_insert_begin, &_insert_end, 
               /*towards_end=*/true);
      }

      // TODO just pass in an IteratorTracker
      void _insertion_order_decrement() {
         if (reverse)
            _insertion_order_advance<NodeListRevIter>(
               &_insert_riter, &_insert_rbegin, &_insert_rend, 
               /*towards_end=*/true);
         else
            _insertion_order_advance<NodeListIter>(
               &_insert_iter, &_insert_begin, &_insert_end, 
               /*towards_end=*/false);
      }

      std::string _node_data(AVLNode *n) const {
         using namespace std;
         stringstream ss;
         if (n)
            ss << n->data;
         else
            ss << "null";
         return ss.str();
      }

      std::string _order_type_name(OrderType ty) const {
         OrderTypeToStrSingleton order_ty_to_str;
         return order_ty_to_str.get().at(ty);
      }

      std::string _str() const {
         using namespace std;
         stringstream ss;
         ss << "curr: " << _curr << " = " << _node_data(_curr) << endl;
         ss << "prev: " << _prev << " = " << _node_data(_prev) << endl;
         ss << boolalpha;
         ss << "prev_incr: " << _prev_incr << endl;
         ss << "reverse: " << reverse << endl;
         ss << "order_ty: " << _order_type_name(_order_ty) << endl;
         ss << "curr_lv: " << _curr_lv << endl;
         return ss.str();
      }

   public:
      _const_iterator(): 
         _curr(nullptr), 
         _prev(nullptr), 
         _prev_incr(true), 
         _order_ty(OrderType::INORDER),

         _curr_lv(0), 
         _prev_lv(0) {}

      _const_iterator(Vindex *vin, OrderType order_ty): 
         _prev(nullptr),
         _prev_incr(reverse ? false : true), 
         _order_ty(order_ty),

         _curr_lv(0), 
         _prev_lv(0),

         _insert_iter(vin->_insertion_list.begin()),
         _insert_riter(vin->_insertion_list.rbegin()),
         _insert_end(vin->_insertion_list.end()),
         _insert_rend(vin->_insertion_list.rend()),
         _insert_begin(vin->_insertion_list.begin()),
         _insert_rbegin(vin->_insertion_list.rbegin()) {

         if (_order_ty == OrderType::INORDER)
            _curr = reverse ? 
               _get_rightest_node(vin->_head_raw()) : 
               _get_leftest_node(vin->_head_raw());
         else if (_order_ty == OrderType::PREORDER)
            _curr = reverse ?
               _get_rightest_node(vin->_head_raw()) :
               _get_root_node(vin->_head_raw());
         else if (_order_ty == OrderType::POSTORDER)
            _curr = reverse ?
               _get_root_node(vin->_head_raw()) :
               _get_leftest_node(vin->_head_raw());
         else if (_order_ty == OrderType::BREADTHFIRST) {
            _curr = reverse ?
               _get_deepest_right_node(vin->_head_raw(), &_curr_lv) :
               _get_root_node(vin->_head_raw()); 

            _curr_lv = reverse ? _curr_lv : 1;
         }
         else if (_order_ty == OrderType::INSERTION)
            _curr = reverse ? *_insert_riter : *_insert_iter;
         else
            assert(false, "NotYetImplementedError");
      }

      _const_iterator(const _const_iterator &other): 
         _curr(other._curr), 
         _prev(other._prev), 
         _prev_incr(other._prev_incr), 
         _order_ty(other._order_ty), 

         _curr_lv(other._curr_lv),
         _prev_lv(other._prev_lv),

         _insert_iter(other._insert_iter),
         _insert_riter(other._insert_riter),
         _insert_end(other._insert_end),
         _insert_rend(other._insert_rend),
         _insert_begin(other._insert_begin),
         _insert_rbegin(other._insert_rbegin) {}

      _const_iterator& operator=(const _const_iterator &other) {
         _curr = other._curr;
         _prev = other._prev;
         _prev_incr = other._prev_incr;
         _order_ty = other._order_ty;

         _curr_lv = other._curr_lv;
         _prev_lv = other._prev_lv;

         _insert_iter = other._insert_iter;
         _insert_riter = other._insert_riter;
         _insert_end = other._insert_end;
         _insert_rend = other._insert_rend;
         _insert_begin = other._insert_begin;
         _insert_rbegin = other._insert_rbegin;

         return *this;
      }

      bool operator==(const _const_iterator& other) const {
         return _curr == other._curr;
      }

      bool operator!=(const _const_iterator& other) const {
         return !operator==(other);
      }

      _const_iterator operator++() {
         if (_order_ty == OrderType::INORDER)
            _in_order_increment();
         else if (_order_ty == OrderType::PREORDER)
            _pre_order_increment();
         else if (_order_ty == OrderType::POSTORDER)
            _post_order_increment();
         else if (_order_ty == OrderType::BREADTHFIRST)
            _breadth_first_increment();
         else if (_order_ty == OrderType::INSERTION)
            _insertion_order_increment();
         else
            assert(false, "NotYetImplementedError");

         _prev_incr = true;
         return *this;
      }

      _const_iterator operator++(int) {
         _const_iterator tmp(*this);
         operator++();
         return tmp;
      }

      _const_iterator operator--() {
         if (_order_ty == OrderType::INORDER)
            _in_order_decrement();
         else if (_order_ty == OrderType::PREORDER)
            _pre_order_decrement();
         else if (_order_ty == OrderType::POSTORDER)
            _post_order_decrement();
         else if (_order_ty == OrderType::BREADTHFIRST)
            _breadth_first_decrement();
         else if (_order_ty == OrderType::INSERTION)
            _insertion_order_decrement();
         else
            assert(false, "NotYetImplementedError");

         _prev_incr = false;
         return *this;
      }

      _const_iterator operator--(int) {
         _const_iterator tmp(*this);
         operator--();
         return tmp;
      }

      const T& operator*() const {
         return _curr ? _curr->data : Vindex<T>::_default()->data;
      }

      const T* operator->() const {
         return _curr ? &_curr->data : &Vindex<T>::_default()->data;
      }

      _const_iterator end() const { 
         return _const_iterator();
      }

      std::string str() const {
         return _str();
      }

      size_t curr_level() const {
         return _curr_lv;
      }
   };

   AVLNodeOwner _head;
   OrderType _order_ty;
   const_iterator _cend;
   const_reverse_iterator _crend;
   NodeList _rebalanced_trees;
   NodeList _insertion_list;

public:
   class const_iterator: public _const_iterator<false> {
   public:
      const_iterator() {}

      const_iterator(Vindex *vin, OrderType order_ty): 
         _const_iterator<false>(vin, order_ty) {}

      const_iterator(const const_iterator &other):
         _const_iterator<false>(other) {}

      const_iterator& operator=(const const_iterator& other) {
         _const_iterator<false>::operator=(other);   
         return *this;
      }

      const_iterator operator++() {
         _const_iterator<false>::operator++();
         return *this;
      } 

      const_iterator operator++(int val) {
         _const_iterator<false>::operator++(val);
         return *this;
      }

      const_iterator operator--() {
         _const_iterator<false>::operator--();
         return *this;
      } 

      const_iterator operator--(int val) {
         _const_iterator<false>::operator--(val);
         return *this;
      }

      const_iterator end() {
         return const_iterator();
      }
   };

   class const_reverse_iterator: public _const_iterator<true> {
   public:
      const_reverse_iterator() {}

      const_reverse_iterator(Vindex *vin, OrderType order_ty): 
         _const_iterator<true>(vin, order_ty) {}

      const_reverse_iterator(const const_reverse_iterator &other):
         _const_iterator<true>(other) {}

      const_reverse_iterator& operator=(const const_reverse_iterator& other) {
         _const_iterator<true>::operator=(other);   
         return *this;
      }

      const_reverse_iterator operator++() {
         _const_iterator<true>::operator--();
         return *this;
      } 

      const_reverse_iterator operator++(int val) {
         _const_iterator<true>::operator--(val);
         return *this;
      }

      const_reverse_iterator operator--() {
         _const_iterator<true>::operator++();
         return *this;
      } 

      const_reverse_iterator operator--(int val) {
         _const_iterator<true>::operator++(val);
         return *this;
      }

      const_reverse_iterator end() {
         return const_reverse_iterator();
      }
   }; 

private:
   static AVLNode *_default() {
      return &AVLNodeDefaultSingleton().get();
   }

   static int dtoi(double val) {
      std::stringstream ss;
      ss << val;
      int val_i;
      ss >> val_i;
      return val_i;
   }

   static int max(int a, int b) {
      return a > b ? a : b;
   }

   static std::string itos(int x) {
      std::stringstream ss;
      ss << x;
      return ss.str();
   }

   static std::string _node_data_str(AVLNode *n) {
      using namespace std;
      stringstream ss;
      if (n)
         ss << n->data;
      else
         ss << "null";
      return ss.str();
   }

   static std::string _node_str(const AVLNode &n) {
      using namespace std;
      stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", height: " << n.height 

#if DEPTH_DATA_ENABLED
         << ", depth: " << n.depth 
#endif

         << ", left: " << _node_data_str(n.left_raw()) 
         << ", right: " << _node_data_str(n.right_raw())
         << ", parent: " << _node_data_str(n.parent)
         << ")";
      return ss.str();
   }

   AVLNode *_head_raw() {
      return _head.get();
   }

   size_t _nodes_at_lv(size_t lv) {
      return 1 << (lv - 1);
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

   AVLNodeOwner& _first_child(AVLNode *n) {
      assert(n, "NullPointerError");
      if (n->left)
         return n->left;
      else if (n->right)
         return n->right;
      else
         assert(false, "NoChildError");
   }

   Direction _which_child(AVLNode *n) {
      assert(n, "NullPointerError");
      AVLNode *parent = n->parent;

      if (!parent)
         return Direction::ROOT;
      else if (parent->left_raw() == n)
         return Direction::LEFT;
      else if (parent->right_raw() == n)
         return Direction::RIGHT;
      else
         assert(false, "DetachedNodeError");
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

   // TODO function to get highest rebalanced tree
   AVLNode *_get_highest_rebalanced_tree() {
      assert(false, "NotYetImplementedError");
   }

   void _update_depths_if_rebalanced() {
      if (!_rebalanced_trees.empty()) {
         _update_depths(_get_highest_rebalanced_tree());
         _rebalanced_trees.clear();
      }
   }

   int _height(AVLNode *tree) {
      if (!tree)
         return 0;
      return max(_height(tree->left_raw()), _height(tree->right_raw())) + 1;
   }

   bool _is_too_left_heavy(int bf) {
      return bf < -1;
   }

   bool _is_too_right_heavy(int bf) {
      return bf > 1;
   }

   bool _is_too_heavy(int bf) {
      return _is_too_left_heavy(bf) || _is_too_right_heavy(bf);
   }

   bool _is_left_heavy(int bf) {
      return bf < 0;
   }

   bool _is_right_heavy(int bf) {
      return bf > 0;
   }

   int _balance_factor(AVLNode *subtree) {
      return subtree ? 
         _height(subtree->right_raw()) - _height(subtree->left_raw()) : 0;
   }

   bool _is_left_left(AVLNode *subtree) {
      return _is_left_heavy(_balance_factor(subtree->left_raw()));
   }

   bool _is_left_right(AVLNode *subtree) {
      return _is_right_heavy(_balance_factor(subtree->left_raw()));
   }

   bool _is_right_right(AVLNode *subtree) {
      return _is_right_heavy(_balance_factor(subtree->right_raw()));
   }

   bool _is_right_left(AVLNode *subtree) {
      return _is_left_heavy(_balance_factor(subtree->right_raw()));
   }

   AVLNodeOwner *_xchild_by_rotation(AVLNode *x, Direction rot_dxn) {
      AVLNodeOwner *xchild = nullptr;
      if (rot_dxn == Direction::LEFT)
         xchild = &x->left;
      else if (rot_dxn == Direction::RIGHT)
         xchild = &x->right;
      else
         assert(false, "InvalidDirectionError");
      return xchild;
   }

   AVLNodeOwner *_ychild_by_rotation(AVLNode *y, Direction rot_dxn) {
      AVLNodeOwner *ychild = nullptr;
      if (rot_dxn == Direction::LEFT)
         ychild = &y->right;
      else if (rot_dxn = Direction::RIGHT)
         ychild = &y->left;
      else
         assert(false, "InvalidDirectionError");
      return ychild;
   }

   void _single_rotation_parts(
      const AVLNodeOwner &x,
      AVLNodeOwner *y, AVLNodeOwner *t2, 
      AVLNodeOwner **x_owner, AVLNodeOwner **t2_owner,
      Direction y_dxn) {

      if (y_dxn == Direction::LEFT) {
         *y = std::move(x->left);
         *t2 = std::move((*y)->right);
         *x_owner = &(*y)->right;
         *t2_owner = &x->left;
      }
      else if (y_dxn == Direction::RIGHT) {
         *y = std::move(x->right);
         *t2 = std::move((*y)->left);
         *x_owner = &(*y)->left;
         *t2_owner = &x->right;
      }
      else
         assert(false, "InvalidDirectionError");
   }

   void _single_rotation_metadata_update(
      AVLNodeOwner *parent, AVLNodeOwner **child) {
      if (**child)
         (**child)->parent = parent->get();
      (*parent)->height = _height(parent->get());
   }

   AVLNodeOwner& _modify_proxy_tree(
      AVLNodeOwner *tree, const TreeEditAction &edit) {

      AVLNodeOwner working_tree = std::move(*tree);
      *tree = edit(&working_tree);
      return *tree;
   }

   AVLNodeOwner& _single_rotation(AVLNodeOwner *tree, Direction y_dxn) {
      return _modify_proxy_tree(
         tree, 
         [&y_dxn, this](AVLNodeOwner *x) -> AVLNodeOwner {

         AVLNodeOwner y;
         AVLNodeOwner t2;
         AVLNodeOwner *x_owner = nullptr;
         AVLNodeOwner *t2_owner = nullptr;

         _single_rotation_parts(*x, &y, &t2, &x_owner, &t2_owner, y_dxn);
         *t2_owner = std::move(t2);
         _single_rotation_metadata_update(x, &t2_owner);
         *x_owner = std::move(*x); 
         _single_rotation_metadata_update(&y, &x_owner);

         return y;
      });
   }

   AVLNodeOwner& _right_rotation(AVLNodeOwner *subtree) {
      return _modify_proxy_tree(
         subtree, 
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_single_rotation(working_tree, Direction::LEFT));
         });
   }

   AVLNodeOwner& _left_rotation(AVLNodeOwner *subtree) {
      return _modify_proxy_tree(
         subtree, 
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_single_rotation(working_tree, Direction::RIGHT));
         });
   }

   AVLNodeOwner& _left_right_double_rotation(AVLNodeOwner *subtree) {
      _modify_proxy_tree(
         &(*subtree)->left,
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_left_rotation(working_tree));
         });
      return _modify_proxy_tree(
         subtree,
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_right_rotation(working_tree));
         });
   }

   AVLNodeOwner& _right_left_double_rotation(AVLNodeOwner *subtree) {
       _modify_proxy_tree(
         &(*subtree)->right,
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_right_rotation(working_tree));
         });
      return _modify_proxy_tree(
         subtree,
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_left_rotation(working_tree));
         });
   }

   AVLNodeOwner& _rebalance(AVLNodeOwner *subtree) {
      int bf = _balance_factor(subtree->get());
      AVLNodeOwner *balanced_tree = nullptr;

      if (_is_too_heavy(bf)) {
         balanced_tree = &_modify_proxy_tree(
            subtree,
            [this, &bf](AVLNodeOwner *working_tree) -> AVLNodeOwner {
               if (_is_too_left_heavy(bf)) {
                  if (_is_left_left(working_tree->get()))
                     return std::move(_right_rotation(working_tree));
                  else if (_is_left_right(working_tree->get()))
                     return std::move(
                        _left_right_double_rotation(working_tree));
                  else
                     assert(false, "InvalidHeavyStateError");
               }
               else if (_is_too_right_heavy(bf)) {
                  if (_is_right_right(working_tree->get()))
                     return std::move(_left_rotation(working_tree));
                  else if (_is_right_left(working_tree->get()))
                     return std::move(
                        _right_left_double_rotation(working_tree));
                  else
                     assert(false, "InvalidHeavyStateError");
               }
               else
                  assert(false, "InvalidHeavyStateError");
            }
         );

#if DEPTH_DATA_ENABLED
         _rebalanced_trees.emplace_back(subtree->get());
#endif
      }
      else
         balanced_tree = subtree;

      return *balanced_tree;
   }

   AVLNodeOwner& _child_insertion_side(AVLNodeOwner *n, AVLNodeOwner *subtree) {
      return **n < **subtree ? (*subtree)->left : (*subtree)->right;
   }

   AVLNodeOwner& _insert(
      AVLNodeOwner *n, AVLNodeOwner *subtree, AVLNodeOwner *parent = nullptr) {

      (*n)->parent = subtree->get();
      ++(*n)->depth;

      AVLNodeOwner &child_tree = _child_insertion_side(n, subtree);

      if (child_tree) {
         _modify_proxy_tree(&child_tree,
            [this, n, subtree](AVLNodeOwner *working_tree) -> AVLNodeOwner {
               return std::move(_insert(n, working_tree, subtree));
            });
         child_tree->parent = subtree->get();
      }
      else {
         child_tree = std::move(*n);
         _insertion_list.emplace_back(child_tree.get());
      }

      (*subtree)->height = _height(subtree->get());

      return _modify_proxy_tree(
         subtree,
         [this](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            return std::move(_rebalance(working_tree));
         });
   }

   AVLNodeOwner _on_removal_leaf(AVLNodeOwner *n, AVLNodeOwner *rm = nullptr) {
      assert(n, "NullPointerError");
      assert(!(*n)->left && !(*n)->right, "NotLeafError");

      if (rm)
         *rm = std::move(*n);
      return AVLNodeOwner();
   }

   void _update_depths(AVLNode *n, size_t depth) {
      if (!n)
         return;
      n->depth = depth;
      if (n->left)
         _update_depths(n->left_raw(), depth + 1);
      if (n->right)
         _update_depths(n->right_raw(), depth + 1);
   }

   void _update_depths(AVLNode *n) {
      assert(n, "NullPointerError");
      size_t start_depth = n->parent ? n->parent->depth + 1 : 1;
      _update_depths(n, start_depth);
   }

   AVLNodeOwner& _on_removal_one_child(
      AVLNodeOwner *n, AVLNodeOwner *rm = nullptr) {
      assert(*n, "NullPointerError");
      assert(_num_children(n->get()) == 1, "MustHaveExactlyOneChildError");

      return _modify_proxy_tree(n,
         [this, &rm](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            AVLNodeOwner child = std::move(_first_child(working_tree->get()));

            AVLNodeOwner tmp;
            if (!rm)
               rm = &tmp;

            *rm = std::move(*working_tree);

            child->parent = (*rm)->parent;
            (*rm)->height = _height(rm->get());
            (*rm)->parent = nullptr;
            (*rm)->left = nullptr;
            (*rm)->right = nullptr;

            return child;
         }
      );
   }

   void _transfer_to_next(AVLNodeOwner *next, AVLNodeOwner *temp) {
      (*next)->left = std::move((*temp)->left);
      (*next)->right = std::move((*temp)->right);
      (*next)->parent = (*temp)->parent;
      (*next)->height = (*temp)->height;   
      (*next)->depth = (*temp)->depth;

      if ((*next)->left)
         (*next)->left->parent = next->get();
      if ((*next)->right)
         (*next)->right->parent = next->get();
   }

   AVLNodeOwner _rm_next_in_order(AVLNodeOwner *tree, AVLNodeOwner *next) {
      if ((*tree)->left) {
         return std::move(_modify_proxy_tree(&(*tree)->left,
            [this, &next](AVLNodeOwner *working_tree) -> AVLNodeOwner {
               *working_tree = std::move(_rm_next_in_order(working_tree, next));
               (*working_tree)->height = _height(working_tree->get());
               return std::move(*working_tree);
            }
         ));
      }
      else {
         if ((*tree)->right)
            return std::move(_on_removal_one_child(tree, next));
         else
            return std::move(_on_removal_leaf(tree, next));
      }
   }

   AVLNodeOwner& _on_removal_two_children(AVLNodeOwner *n) {
      assert(n, "NullPointerError");

      return  _modify_proxy_tree(n, 
         [this, &n](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            AVLNodeOwner next;

            (*working_tree)->right = 
               std::move(_rm_next_in_order(&(*working_tree)->right, &next));
            (*working_tree)->height = _height(working_tree->get());
            _transfer_to_next(&next, working_tree);
            return next;
         }
      );
   }

   void _remove_and_rebalance(
      const T& val, AVLNodeOwner *subtree, AVLNode *parent) {

      _modify_proxy_tree(subtree,
         [this, &val](AVLNodeOwner *working_tree) -> AVLNodeOwner {
            *working_tree = std::move(_remove(val, working_tree));
            return std::move(_rebalance(working_tree));
         });

      if (*subtree)
         (*subtree)->parent = parent;
   }

   AVLNodeOwner& _remove(const T& val, AVLNodeOwner *tree) {
      if (!*tree)
         return *tree;

      if (val < (*tree)->data)
         _remove_and_rebalance(val, &(*tree)->left, tree->get());
      else if (val > (*tree)->data)
         _remove_and_rebalance(val, &(*tree)->right, tree->get());
      else {
         _insertion_list.remove(tree->get());

         if (_num_children(tree->get()) == 1)
            *tree = std::move(_on_removal_one_child(tree));
         else if (!_num_children(tree->get()))
            *tree = std::move(_on_removal_leaf(tree));
         else
            *tree = std::move(_on_removal_two_children(tree));
      }

      if (*tree)
         (*tree)->height = _height(tree->get());
      return *tree;
   }

   bool _is_dq_all_nulls(const NodeDQ &dq) {
      auto it = find_if(
         dq.begin(), dq.end(), [](AVLNode *n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_per_line(NodeDQ *dq, const VoidFunc &func) {
      func();
      if (_is_dq_all_nulls(*dq))
         dq->clear();
   }

   void _on_valid_node(NodeDQ *dq, AVLNode *n, const NodeListener &func) {
      func(n);
      dq->push_back(n->left_raw());
      dq->push_back(n->right_raw());
   }

   void _on_null_node(NodeDQ *dq, const NodeListener &func) {
      func(nullptr);
      dq->push_back(nullptr);
      dq->push_back(nullptr);
   }

   void _gather_bfs(
      NodeDQ *dq, size_t *curr_depth, size_t *node_cnt, const NodeListener &func) {
      if (dq->empty()) 
         return;

      AVLNode *n = dq->front();
      dq->pop_front();

      if (n) 
         _on_valid_node(dq, n, func);
      else 
         _on_null_node(dq, func);

      if (++*node_cnt == _nodes_at_lv(*curr_depth))
         _on_max_nodes_per_line(dq, [&node_cnt, &curr_depth]() { 
            *node_cnt = 0; 
            ++*curr_depth;
         });

      _gather_bfs(dq, curr_depth, node_cnt, func);
   }

   NodeList _gather_bfs_list() {
      NodeDQ dq;
      NodeList nl;
      size_t curr_depth = 1;
      size_t node_cnt = 0;

      if (_head_raw())
         dq.push_back(_head_raw());

      _gather_bfs(&dq, &curr_depth, &node_cnt,
         [&nl](AVLNode *n) { nl.push_back(n); });
      return nl;
   }

   std::string _gather_bfs_str(const std::string &delim="|") {
      using namespace std;
      NodeList nl = _gather_bfs_list();

      stringstream ss;
      int node_cnt = 0;
      int curr_depth = 1;

      for (auto nit = nl.begin(); nit != nl.end(); ++nit) {
         AVLNode *n = *nit;
         bool last_node = ++node_cnt == _nodes_at_lv(curr_depth);
         ss << (n ? _node_str(*n) : "(null)") << (last_node ? "" : " ");

         if (last_node) {
            if (nit != prev(nl.end()))
               ss << delim;
            node_cnt = 0;
            ++curr_depth;
         }
      }
      return ss.str();
   }

public:
   Vindex(): 
      _head(nullptr), 
      _order_ty(OrderType::INORDER) {}

   void insert(const T& val) {
      AVLNodeOwner n = std::make_unique<AVLNode>(val);
      ++n->height;
      ++n->depth;

      if (!_head) {
         _head = std::move(n);
         ++_head->height;
         _insertion_list.emplace_back(_head_raw()); 
         return;
      }
      _head = std::move(_insert(&n, &_head));
      _head->parent = nullptr;

#if DEPTH_DATA_ENABLED
      _update_depths_if_rebalanced();
#endif
   }

   template <typename... Args>
   void emplace_back(Args&&... args) {
      insert(T(std::forward<Args>(args)...));
   }

   void remove(const T& val) {
      _remove_and_rebalance(val, &_head, nullptr);
      _update_depths_if_rebalanced();
   }

   std::string bfs_str(const std::string &delim = "|") {
      return _gather_bfs_str(delim);
   }

   void order(OrderType order_ty) {
      _order_ty = order_ty;
   }

   OrderType order() {
      return _order_ty;
   }

   const_iterator cbegin() {
      auto it = const_iterator(this, _order_ty);
      _cend = it.end();
      return it;
   }

   const_iterator cend() {
      return _cend;
   }

   const_reverse_iterator crbegin() {
      auto it = const_reverse_iterator(this, _order_ty);
      _crend = it.end();
      return it;
   }

   const_reverse_iterator crend() {
      return _crend;
   }

   void clear() {
      _insertion_list.clear();
      _head = nullptr;
   }
};

#ifdef assert
#undef assert
#endif

#endif
