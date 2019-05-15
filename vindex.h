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
#include <list>

#include "helpers.h"
#include "custom_exceptions.h"

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
   typedef std::unique_ptr<_AVLState> AVLStateOwner;

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

namespace Direction {
   enum Direction { LEFT, RIGHT, ROOT };
}

namespace OrderType {
   enum OrderType { INORDER, PREORDER, POSTORDER, BREADTHFIRST, INSERTION };
}

template<typename T>
class Vindex {
public:
   class const_iterator;

private:
   typedef _AVLState<_Node<T>> AVLNode;
   typedef std::unique_ptr<AVLNode> AVLNodeOwner;
   typedef std::map<AVLNode *, AVLNodeOwner> NodeCache;
   typedef typename NodeCache::iterator NodeCacheIter;
   typedef std::deque<AVLNode *> NodeDQ;
   typedef std::list<AVLNode *> NodeList;
   typedef std::function<void(AVLNode *)> NodeListener;
   typedef std::function<void()> VoidFunc;
   typedef std::function<void(AVLNodeOwner&)> AVLNodeOwnerFunc;
   typedef Direction::Direction Direction;
   typedef OrderType::OrderType OrderType;

   AVLNodeOwner _head;
   OrderType _order_ty;
   const_iterator _end;

public:
   class const_iterator: 
      public std::iterator<std::bidirectional_iterator_tag, T> {
   private:
      typedef Vindex::AVLNode AVLNode;
      typedef Vindex::AVLNodeOwner AVLNodeOwner;

      AVLNode *_get_leftest_node(AVLNode *tree) {
         return tree->left ? _get_leftest_node(tree->left_raw()) : tree;
      }

      AVLNode *_get_rightest_node(AVLNode *tree) {
         return tree->right ? _get_rightest_node(tree->right_raw()) : tree;
      }

      AVLNode *_get_deepest_right_node(AVLNode *tree) {
         if (tree->right)
            return _get_deepest_right_node(tree->right_raw());
         else if (tree->left)
            return _get_deepest_right_node(tree->left_raw());
         else
            return tree;
      }

      AVLNode *_get_root_node(AVLNode *tree) {
         return tree;
      }

      AVLNode *_get_left_sibling(AVLNode *tree) {
         if (!tree || !tree->parent)
            return nullptr;
         AVLNode *parent = tree->parent;
         return parent->right_raw() == tree ? parent->left_raw() : nullptr;
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

      AVLNode *_retrace_in_order(bool decr=false) {
         if (!_curr || !_curr->parent)
            return nullptr;

         AVLNode *curr = _curr;
         AVLNode *parent = curr->parent;
         AVLNodeOwner *parents_child = decr ? &parent->left : &parent->right;

         while (parent && parents_child->get() == curr) {
            curr = curr->parent;
            parent = curr->parent;
            parents_child = decr ? &parent->left : &parent->right;
         }
         return parent;
      }

      void _in_order_increment() {
         AVLNode *tmp = _curr;
         if (!_prev_incr)
            _curr = _prev;   
         else if (_visited_left_subtree() || _visited_parent() || !_prev)
            _curr = tmp->right ? 
               _get_leftest_node(tmp->right_raw()) : _retrace_in_order();
         else if (_visited_right_subtree())
            throw InvalidAdvanceStateError();
         else if (!_curr)
            return;
         else
            throw InvalidAdvanceStateError();

         _prev = tmp;
      }

      void _in_order_decrement() {
         AVLNode *tmp = _curr;
         if (_prev_incr)
            _curr = _prev;
         else if (_visited_right_subtree() || _visited_parent() || !_prev)
            _curr = tmp->left ?
               _get_rightest_node(tmp->left_raw()) : 
               _retrace_in_order(/*decr=*/true);
         else if (!_curr)
            return;
         else
            throw InvalidAdvanceStateError();

         _prev = tmp;
      }

      AVLNode *_retrace_pre_order() {
         AVLNode *n = _retrace_in_order();
         return n ? (n->right ? n->right_raw() : nullptr) : nullptr;
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
               _curr = _retrace_pre_order();
         }
         else if (!tmp)
            return;
         else
            throw InvalidAdvanceStateError();

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
                  _get_deepest_right_node(left_sibling) : tmp->parent;
            }
            else if (!tmp->parent || tmp->parent->left_raw() == tmp)
               _curr = tmp->parent;
            else
               throw InvalidAdvanceStateError();
         }
         else if (!tmp)
            return;
         else
            throw InvalidAdvanceStateError();

         _prev = tmp;
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

      std::string _str() const {
         using namespace std;
         stringstream ss;
         ss << "curr: " << _curr << " = " << _node_data(_curr) << endl;
         ss << "prev: " << _prev << " = " << _node_data(_prev) << endl;
         ss << boolalpha;
         ss << "prev_incr: " << _prev_incr << endl;
         ss << "reverse: " << _reverse << endl;
         return ss.str();
      }

      AVLNode *_curr;
      AVLNode *_prev;
      bool _prev_incr;
      bool _reverse;
      OrderType _order_ty;
      std::unique_ptr<AVLNode> _default;

   public:
      const_iterator(): 
         _curr(nullptr), _prev(nullptr), 
         _prev_incr(true), _reverse(false),
         _order_ty(OrderType::INORDER),
         _default(std::make_unique<AVLNode>(T())) {}

      const_iterator(Vindex *vin, OrderType order_ty, bool reverse=false): 
         _reverse(reverse), _prev(nullptr), 
         _prev_incr(true), _order_ty(order_ty),
         _default(std::make_unique<AVLNode>(T())) {

         if (order_ty == OrderType::INORDER)
            _curr = _reverse ? 
               _get_rightest_node(vin->_head_raw()) : 
               _get_leftest_node(vin->_head_raw());
         else if (order_ty == OrderType::PREORDER)
            _curr = _reverse ?
               _get_rightest_node(vin->_head_raw()) :
               _get_root_node(vin->_head_raw());
      }

      const_iterator(const const_iterator &other): 
         _curr(other._curr), _prev(other._prev), 
         _reverse(other._reverse), _prev_incr(other._prev_incr), 
         _order_ty(other._order_ty), 
         _default(std::make_unique<AVLNode>(T())) {}

      const_iterator& operator=(const const_iterator &other) {
         _curr = other._curr;
         _prev = other._prev;
         _prev_incr = other._prev_incr;
         _reverse = other._reverse;
         _order_ty = other._order_ty;
         return *this;
      }

      bool operator==(const const_iterator& other) const {
         return _curr == other._curr;
      }

      bool operator!=(const const_iterator& other) const {
         return !operator==(other);
      }

      const_iterator operator++() {
         if (_order_ty == OrderType::INORDER)
            _in_order_increment();
         else if (_order_ty == OrderType::PREORDER)
            _pre_order_increment();
         else
            throw InvalidOperationError();

         _prev_incr = true;
         return *this;
      }

      const_iterator operator++(int) {
         const_iterator tmp(*this);
         operator++();
         return tmp;
      }

      const_iterator operator--() {
         if (_order_ty == OrderType::INORDER)
            _in_order_decrement();
         else if (_order_ty == OrderType::PREORDER)
            _pre_order_decrement();
         else
            throw InvalidOperationError();

         _prev_incr = false;
         return *this;
      }

      const_iterator operator--(int) {
         const_iterator tmp(*this);
         operator--();
         return tmp;
      }

      const T& operator*() const {
         return _curr ? _curr->data : _default->data;
      }

      const T* operator->() const {
         return _curr ? &_curr->data : &_default->data;
      }

      const_iterator end() const { 
         return const_iterator();
      }

      std::string str() const {
         return _str();
      }
   };

private:
   AVLNode *_head_raw() {
      return _head.get();
   }

   int _nodes_at_lv(int lv) {
      if (lv < 1)
         throw LessThanOneError();
      return 1 << (lv - 1);
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

   static std::string _node_str(AVLNode &n) {
      using namespace std;
      stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", height: " << n.height 
         << ", left: " << _node_data_str(n.left_raw()) 
         << ", right: " << _node_data_str(n.right_raw())
         << ", parent: " << _node_data_str(n.parent)
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

   Direction _which_child(AVLNode *n) {
      if (!n)
         throw NullPointerError();

      AVLNode *parent = n->parent;

      if (!parent)
         return Direction::ROOT;
      else if (parent->left_raw() == n)
         return Direction::LEFT;
      else if (parent->right_raw() == n)
         return Direction::RIGHT;
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
      AVLNode *child, const AVLNodeOwnerFunc &action) {
      AVLNode *parent = child->parent;
      Direction child_ty = _which_child(child);

      if (child_ty == Direction::LEFT)
         action(parent->left);
      else if (child_ty == Direction::RIGHT)
         action(parent->right);
      else if (child_ty == Direction::ROOT)
         action(_head);
      else
         throw DetachedNodeError();
   } 

   // TODO avoid non-const lvalue refs
   void _assign_if_diff(AVLNodeOwner &o, AVLNode *n) {
      if (o.get() != n)
         o = AVLNodeOwner(n);
   }

   static void _detach_from_owner(AVLNodeOwner &owner) {
      owner.release();
   }

   static AVLNode *_detach_from_owner_rtn(AVLNodeOwner &owner) {
      AVLNode *n = owner.get();
      owner.release();
      return n;
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
         throw InvalidDirectionError();
      return xchild;
   }

   AVLNodeOwner *_ychild_by_rotation(AVLNode *y, Direction rot_dxn) {
      AVLNodeOwner *ychild = nullptr;
      if (rot_dxn == Direction::LEFT)
         ychild = &y->right;
      else if (rot_dxn = Direction::RIGHT)
         ychild = &y->left;
      else
         throw InvalidDirectionError();     
      return ychild;
   }

   AVLNode *_single_rotation(AVLNode *x, Direction y_dxn) {
      AVLNode *y = nullptr;
      AVLNode *subtree = nullptr;

      _act_with_child_owner(x, _detach_from_owner);
      AVLNodeOwner *xchild = _xchild_by_rotation(x, y_dxn);
      y = _detach_from_owner_rtn(*xchild);
      AVLNodeOwner *ychild = _ychild_by_rotation(y, y_dxn);
      subtree = _detach_from_owner_rtn(*ychild);

      if (*xchild)
         throw NotNullPointerError();
      *xchild = AVLNodeOwner(subtree);

      if (*ychild)
         throw NotNullPointerError();
      *ychild = AVLNodeOwner(x);

      x->parent = y;
      if (subtree)
         subtree->parent = x;
      x->height = _height(x);
      y->height = _height(y);

      return y;     
   }

   AVLNode *_right_rotation(AVLNode *subtree) {
      return _single_rotation(subtree, Direction::LEFT);
   }

   AVLNode *_left_rotation(AVLNode *subtree) {
      return _single_rotation(subtree, Direction::RIGHT);
   }

   AVLNode *_left_right_double_rotation(AVLNode *subtree) {
      _assign_if_diff(subtree->left, _left_rotation(subtree->left_raw()));
      return _right_rotation(subtree);
   }

   AVLNode *_right_left_double_rotation(AVLNode *subtree) {
      _assign_if_diff(subtree->right, _right_rotation(subtree->right_raw()));
      return _left_rotation(subtree);
   }

   AVLNode *_rebalance(AVLNode *subtree) {
      int bf = _balance_factor(subtree);

      if (_is_too_left_heavy(bf)) {
         if (_is_left_left(subtree))
            subtree = _right_rotation(subtree);
         else if (_is_left_right(subtree))
            subtree = _left_right_double_rotation(subtree);
         else
            throw InvalidHeavyStateError();
      }
      else if (_is_too_right_heavy(bf)) {
         if (_is_right_right(subtree))
            subtree = _left_rotation(subtree);
         else if (_is_right_left(subtree))
            subtree = _right_left_double_rotation(subtree);
         else
            throw InvalidHeavyStateError();
      }
      return subtree;
   }

   AVLNodeOwner& _child_insertion_side(AVLNodeOwner &n, AVLNode *subtree) {
      return *n < *subtree ? subtree->left : subtree->right;
   }

   AVLNode *_insert(
      AVLNodeOwner &n, AVLNode *subtree, AVLNode *parent = nullptr) {
      n->parent = subtree;
      AVLNodeOwner &child_tree = _child_insertion_side(n, subtree);

      if (child_tree) {
         _assign_if_diff(child_tree, _insert(n, child_tree.get(), subtree));
         child_tree->parent = subtree;
      }
      else
         child_tree = move(n);

      subtree->height = _height(subtree);
      return _rebalance(subtree);
   }

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

   AVLNode *_detach_next_in_order(AVLNode *tree, AVLNode *&next) {
      if (tree->left) {
         _assign_if_diff(tree->left, 
            _detach_next_in_order(tree->left_raw(), next));
         tree->height = _height(tree);
         return tree;
      }
      else {
         next = tree;
         if (tree->right)
            return _on_removal_one_child(tree, /*detach=*/true);
         else
            return _on_removal_leaf(next, /*detach=*/true);
      }
   }

   void _rewire_next(AVLNode *next, AVLNode *temp) {
      if (next->left)
         throw NotNullPointerError();

      next->left = move(temp->left);
      next->right = move(temp->right);
      next->parent = temp->parent;
      next->height = temp->height;   

      if (next->left)
         next->left->parent = next;
      if (next->right)
         next->right->parent = next;
   }

   AVLNode *_on_removal_two_children(AVLNode *n, bool detach = false) {
      if (!n) 
         throw NullPointerError();

      AVLNode *next = nullptr;
      _assign_if_diff(n->right, 
         _detach_next_in_order(n->right_raw(), next));
      n->height = _height(n);

      if (!next)
         throw NullPointerError();
      _rewire_next(next, n);

      if (detach)
         _act_with_child_owner(n, _detach_from_owner);

      return next;
   }

   void _remove_and_rebalance(
      const T& val, AVLNodeOwner &subtree, AVLNode *parent) {
      _assign_if_diff(subtree, _remove(val, subtree.get()));
      _assign_if_diff(subtree, _rebalance(subtree.get()));
      if (subtree)
         subtree->parent = parent;
   }

   AVLNode *_remove(const T& val, AVLNode* tree) {
      if (!tree)
         return nullptr;

      if (val < tree->data)
         _remove_and_rebalance(val, tree->left, tree);
      else if (val > tree->data)
         _remove_and_rebalance(val, tree->right, tree);
      else {
         if (_num_children(tree) == 1)
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

   bool _is_dq_all_nulls(NodeDQ &dq) {
      auto it = find_if(
         dq.begin(), dq.end(), [](AVLNode *n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_per_line(NodeDQ &dq, const VoidFunc &func) {
      func();
      if (_is_dq_all_nulls(dq))
         dq.clear();
   }

   void _on_valid_node(NodeDQ &dq, AVLNode *n, const NodeListener &func) {
      func(n);
      dq.push_back(n->left_raw());
      dq.push_back(n->right_raw());
   }

   void _on_null_node(NodeDQ &dq, const NodeListener &func) {
      func(nullptr);
      dq.push_back(nullptr);
      dq.push_back(nullptr);
   }

   void _gather_bfs(
      NodeDQ &dq, int &curr_depth, int &node_cnt, const NodeListener &func) {
      if (dq.empty()) 
         return;

      AVLNode *n = dq.front();
      dq.pop_front();

      if (n) 
         _on_valid_node(dq, n, func);
      else 
         _on_null_node(dq, func);

      if (++node_cnt == _nodes_at_lv(curr_depth))
         _on_max_nodes_per_line(dq, [&node_cnt, &curr_depth]() { 
            node_cnt = 0; 
            ++curr_depth;
         });

      _gather_bfs(dq, curr_depth, node_cnt, func);
   }

   NodeList _gather_bfs_list() {
      NodeDQ dq;
      NodeList nl;
      int curr_depth = 1;
      int node_cnt = 0;

      if (_head_raw())
         dq.push_back(_head_raw());

      _gather_bfs(dq, curr_depth, node_cnt,
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
   Vindex(): _head(nullptr) {}

   void insert(const T& val) {
      using namespace std;
      AVLNodeOwner n = make_unique<AVLNode>(val);
      ++n->height;

      if (!_head) {
         _head = move(n);
         ++_head->height;
         return;
      }
      _assign_if_diff(_head, _insert(n, _head_raw()));
      _head->parent = nullptr;
   }

   // TODO test
   template <typename... Args>
   void emplace_back(Args&&... args) {
      using namespace std;
      T val(forward<Args>(args)...);
      insert(val);
   }

   void remove(const T& val) {
      _remove_and_rebalance(val, _head, nullptr);
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

   const_iterator begin() {
      auto it = const_iterator(this, _order_ty);
      _end = it.end();
      return it;
   }

   const_iterator end() {
      return _end;
   }

   const_iterator rbegin() {
      auto it = const_iterator(this, _order_ty, /*reverse=*/true);
      _end = it.end();
      return it;
   }

   const_iterator rend() {
      return end();
   }

   void clear() {
      _head = nullptr;
   }
};
