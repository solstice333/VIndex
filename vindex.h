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
#include <unordered_map>
#include <vector>

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

#define make_extractor(CLS, MEM)\
   [](const CLS& obj) -> decltype(CLS::MEM) { return obj.MEM; }

#define make_vindex(CLS, MEM)\
   Vindex<decltype(CLS::MEM), CLS>(make_extractor(CLS, MEM))

template <typename T, typename KeyTy> class Vindex;

namespace hash_helpers {
   size_t combine(const std::vector<size_t>& hashes) {
      size_t res = 17;
      for (auto it = hashes.begin(); it != hashes.end(); ++it)
         res = res*31 + *it;
      return res;
   }
}

namespace OrderType {
   enum OrderType { INORDER, PREORDER, POSTORDER, BREADTHFIRST, INSERTION };
}

namespace _head_type {
   struct node_ref {};
   struct node_data {};
}

namespace _Direction {
   enum Direction { LEFT, RIGHT, ROOT };
}

namespace _IterTracker {
   template <typename IterTy>
   class IterTrackerBase {
   private:
      IterTy _curr;
      IterTy _begin;
      IterTy _end;

   protected:
      IterTrackerBase() {}
      IterTrackerBase(IterTy curr, IterTy begin, IterTy end): 
         _curr(curr), _begin(begin), _end(end) {}
      IterTrackerBase(IterTy begin, IterTy end): 
         IterTrackerBase(begin, begin, end) {}

   public:
      virtual IterTy& curr() { return _curr; }
      virtual IterTy& begin() { return _begin; }
      virtual IterTy& end() { return _end; }
   };

   template <typename NodeDataTy, typename KeyTy, typename IterTy> 
   class IterTracker: public IterTrackerBase<IterTy> {
   public:
      IterTracker(): IterTrackerBase<IterTy>() {}

      IterTracker(
         typename Vindex<KeyTy, NodeDataTy>::template NodeList<NodeDataTy>& l):
         IterTrackerBase<IterTy>(l.begin(), l.end()) {}
   };

   template <typename T, typename KeyTy>
   using NodeListRevIter = 
      typename Vindex<KeyTy, T>::template NodeList<T>::reverse_iterator;

   template <typename NodeDataTy, typename KeyTy>
   class IterTracker<
         NodeDataTy, 
         KeyTy,
         NodeListRevIter<NodeDataTy, KeyTy>
      >: public IterTrackerBase<NodeListRevIter<NodeDataTy, KeyTy>> {
   public:
      IterTracker(): IterTrackerBase<NodeListRevIter<NodeDataTy, KeyTy>>() {}

      IterTracker(
         typename Vindex<KeyTy, NodeDataTy>::template NodeList<NodeDataTy>& l):
         IterTrackerBase<NodeListRevIter<NodeDataTy, KeyTy>>(
            l.rbegin(), l.rend()) {}
   };
}

template <typename T>
class _IConstResult {
public:
   virtual ~_IConstResult() {}
};

template <typename T>
class ConstResultSuccess: public _IConstResult<T> {
private:
   T _data;

public:
   ConstResultSuccess(const T& data): _data(data) {}
   const T& data() { return _data; }
};

template <typename T>
class ConstResultFailure: public _IConstResult<T> {};

template <typename T>
using ConstResult = std::unique_ptr<_IConstResult<T>>;

template <typename T>
class _IResult {
public:
   virtual ~_IResult() {}
};

template <typename T>
class ResultSuccess: public _IResult<T> {
private:
   T _data;

public:
   template <typename U>
   ResultSuccess(U&& data): _data(std::forward<U>(data)) {}
   T& data() { return _data; }
};

template <typename T>
class ResultFailure: public _IResult<T> {};

template <typename T>
using Result = std::unique_ptr<_IResult<T>>;

template <typename T, typename DerivedTy>
class _Singleton {
private:
   static T resource;
   static std::once_flag flag;

public:
   static void init(T* resource) {
      DerivedTy::init(resource);
   };

   virtual T& get() {
      std::call_once(flag, init, &resource);
      return resource;
   } 
};

template <typename T, typename DerivedTy>
std::once_flag _Singleton<T, DerivedTy>::flag;

template <typename T, typename DerivedTy>
T _Singleton<T, DerivedTy>::resource;

template <typename T>
struct IComparator {
   virtual bool operator==(const IComparator& other) const = 0;

   virtual bool lt(const T& a, const T& b) const = 0;

   virtual bool gt(const T &a, const T& b) const { 
      return lt(b, a); 
   }

   virtual bool lte(const T& a, const T& b) const { 
      return !gt(a, b); 
   }

   virtual bool gte(const T& a, const T& b) const { 
      return !lt(a, b); 
   }

   virtual bool eq(const T& a, const T& b) const { 
      return !lt(a, b) && !gt(a, b); 
   }

   virtual bool ne(const T& a, const T& b) const { 
      return !eq(a, b); 
   }

   virtual size_t hash() const { 
      return typeid(*this).hash_code(); 
   }

   virtual ~IComparator() {}
};

template <typename T>
struct std::hash<IComparator<T>> {
   virtual size_t operator()(const IComparator<T>& other) const {
      return other.hash();
   }
};

template <typename T>
struct DefaultComparator: public IComparator<T> {
   bool operator==(const IComparator<T>& other) const override { 
      return dynamic_cast<const DefaultComparator *>(&other); 
   }

   bool lt(const T& a, const T& b) const override { 
      return a < b; 
   }
};

template <typename T>
struct _Node {
   T data;
   _Node() {}
   _Node(const T& data): data(data) {}
};

template <typename T, typename BASE_TY = decltype(T::data)>
struct _AVLState: public T {
   typedef std::unique_ptr<_AVLState> AVLStateOwner;

   size_t height;
   AVLStateOwner left;
   AVLStateOwner right;
   _AVLState* parent;

   _AVLState* left_raw() const {
      return left.get();
   }

   _AVLState* right_raw() const {
      return right.get();
   }

   _AVLState(): 
      T(BASE_TY()), height(0),
      left(nullptr), right(nullptr), parent(nullptr) {}

   _AVLState(const BASE_TY& data): 
      T(data), height(0), 
      left(nullptr), right(nullptr), parent(nullptr) {}
};

template <typename T>
class _Heads {
public:
   typedef IComparator<T> Comparator;
   typedef _AVLState<_Node<T&>> NodeRef;
   typedef std::unique_ptr<NodeRef> NodeRefOwner;

private:
   typedef std::unique_ptr<Comparator> ComparatorOwner; 
   typedef _AVLState<_Node<T>> NodeData;
   typedef std::unique_ptr<NodeData> NodeDataOwner;

   std::pair<ComparatorOwner, NodeDataOwner> _primary_head;

   std::unordered_map<
      ComparatorOwner, 
      NodeRefOwner,
      std::function<size_t(const ComparatorOwner&)>,
      std::function<bool(const ComparatorOwner&, const ComparatorOwner&)>
   > _secondary_heads;

public:
   class iterator: public std::iterator<
      std::forward_iterator_tag, 
      typename decltype(_secondary_heads)::value_type
   > {
   private:
      typedef decltype(_Heads::_secondary_heads) HeadMap;
      typedef typename HeadMap::iterator MapIter;
      typedef typename HeadMap::value_type MapEntry;
      typedef typename HeadMap::mapped_type MapVal;

      typedef std::pair<Comparator&, MapVal&> Entry;
      typedef std::vector<Entry> EntryArray;
      typedef typename EntryArray::iterator EntryArrayIter;

      std::shared_ptr<EntryArray> _heads;
      EntryArrayIter _iter;

   public:
      iterator() {}

      iterator(HeadMap& heads): _heads(std::make_shared<EntryArray>()) {
         for (auto it = heads.begin(); it != heads.end(); ++it) {
            _heads->emplace_back(std::make_pair<
               std::reference_wrapper<Comparator>, 
               std::reference_wrapper<MapVal>
            >(*it->first, it->second));
         }
         _iter = _heads->begin();
      }

      bool operator==(const iterator& other) {
         return _iter == other._iter;
      }

      bool operator!=(const iterator& other) {
         return _iter != other._iter;
      }

      iterator& operator++() {
         ++_iter;
         return *this;   
      }

      iterator operator++(int) {
         auto tmp = *this;
         operator++();
         return tmp;
      }

      Entry& operator*() {
         return *_iter;
      }

      EntryArrayIter operator->() {
         return _iter;
      }

      iterator end() {
         iterator it = *this;
         it._iter = it._heads->end();
         return it;
      }
   };

private:
   iterator _end_iter;

   static size_t hash_comparator(const ComparatorOwner& c) {
      return std::hash<Comparator>()(*c);
   }

   static bool is_equal_comparator(
      const ComparatorOwner& c1, const ComparatorOwner& c2) {
      return *c1 == *c2;
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>> 
      _get(const ComparatorTy& cmp, _head_type::node_ref) {

      auto it = _secondary_heads.find(std::make_unique<ComparatorTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>> 
      _get(const ComparatorTy& cmp, _head_type::node_ref) const {

      auto it = _secondary_heads.find(std::make_unique<ComparatorTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, const NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>> 
      _get(const ComparatorTy& cmp, _head_type::node_data) {

      if (_primary_head.first && cmp == *_primary_head.first)
         return std::make_unique<std::pair<Comparator&, NodeDataOwner&>>(
            *_primary_head.first, _primary_head.second);
      return std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, const NodeDataOwner&>> 
      _get(const ComparatorTy& cmp, _head_type::node_data) const {

      if (_primary_head.first && cmp == *_primary_head.first)
         return std::make_unique<std::pair<Comparator&, const NodeDataOwner&>>(
            *_primary_head.first, _primary_head.second);
      return std::unique_ptr<std::pair<Comparator&, const NodeDataOwner&>>();
   }

public:
   _Heads(): _secondary_heads(0, hash_comparator, is_equal_comparator) {}

   template <typename ComparatorTy>
   void push(const ComparatorTy& cmp) { 
      ComparatorOwner new_cmp = std::make_unique<ComparatorTy>(cmp);
      if (!_primary_head.first)
         _primary_head.first = std::move(new_cmp);
      else if (_secondary_heads.find(new_cmp) == _secondary_heads.end())
         _secondary_heads[std::move(new_cmp)];
   }

   template <typename HeadTy, typename ComparatorTy>
   std::unique_ptr<
      std::pair<
         Comparator&, 
         typename std::conditional<
            std::is_same<HeadTy, _head_type::node_ref>::value, 
            const NodeRefOwner&, 
            const NodeDataOwner&
         >::type
      >
   > get(const ComparatorTy& cmp) const {
      return _get(cmp, HeadTy());
   }

   template <typename HeadTy, typename ComparatorTy>
   std::unique_ptr<
      std::pair<
         Comparator&, 
         typename std::conditional<
            std::is_same<HeadTy, _head_type::node_ref>::value, 
            NodeRefOwner&, 
            NodeDataOwner&
         >::type
      >
   > get(const ComparatorTy& cmp) {
      return _get(cmp, HeadTy());
   }

   iterator begin() {
      iterator it(_secondary_heads);
      _end_iter = it.end();
      return it;
   }

   iterator end() {
      return _end_iter;
   }
};

template <typename KeyTy, typename T>
class Vindex {
private:
   friend class TestIntVindex;

   typedef _Direction::Direction Direction;
   typedef OrderType::OrderType OrderType;
   typedef std::function<void()> VoidFunc;
   typedef std::function<KeyTy(const T&)> Extractor;
   typedef std::map<OrderType, std::string> OrderTypeToStr;
   typedef typename _Heads<T>::Comparator Comparator;
   typedef typename _Heads<T>::NodeRefOwner NodeRefOwner;

   template <typename U>
   using AVLNode = _AVLState<_Node<U>>;

   template <typename U>
   using AVLNodeOwner = std::unique_ptr<AVLNode<U>>;

   template <typename U>
   using NodeCache = std::map<AVLNode<U>*, AVLNodeOwner<U>>;

   template <typename U>
   using NodeCacheIter = typename NodeCache<U>::iterator;

   template <typename U>
   using NodeDQ = std::deque<AVLNode<U>*>;

   template <typename U>
   using NodeListener = std::function<void(AVLNode<U>*)>;

   template <typename U>
   using TreeEditAction = std::function<AVLNodeOwner<U>(AVLNodeOwner<U>*)>;

   template <typename U>
   using Index = std::unordered_map<KeyTy, AVLNode<U>*>;

public:
   class const_iterator;
   class const_reverse_iterator;

   template <typename U>
   using NodeList = std::list<AVLNode<U>*>;

private:
   class OrderTypeToStrSingleton : 
      public _Singleton<OrderTypeToStr, OrderTypeToStrSingleton> {
   public:
      static void init(OrderTypeToStr* order_ty_to_str) {
         (*order_ty_to_str)[OrderType::INORDER] = "INORDER";
         (*order_ty_to_str)[OrderType::PREORDER] = "PREORDER";
         (*order_ty_to_str)[OrderType::POSTORDER] = "POSTORDER";
         (*order_ty_to_str)[OrderType::BREADTHFIRST] = "BREADTHFIRST";
         (*order_ty_to_str)[OrderType::INSERTION] = "INSERTION";
      }
   }; 

   struct AVLNodeDefaultSingleton :
      public _Singleton<AVLNode<T>, AVLNodeDefaultSingleton> {
      static void init(AVLNode<T>*) {}
   };

   struct DefaultComparatorSingleton:
      public _Singleton<DefaultComparator<T>, DefaultComparatorSingleton> {
      static void init(DefaultComparator<T>* dc) {}
   };

   // TODO T needs to be T& eventually
   template <bool reverse>
   class _const_iterator : 
      public std::iterator<std::bidirectional_iterator_tag, T> {
   private:
      typedef Vindex::Direction Direction;
      typedef typename _Heads<T>::Comparator Comparator;

      template <typename U>
      using AVLNode = Vindex::AVLNode<U>;

      template <typename U>
      using AVLNodeOwner = Vindex::AVLNodeOwner<U>;

      typedef 
         typename Vindex<KeyTy, T>::template NodeList<T>::iterator 
         NodeListIter;

      typedef 
         typename Vindex<KeyTy, T>::template NodeList<T>::reverse_iterator 
         NodeListRevIter;

      typedef 
         typename 
            std::conditional<reverse, NodeListRevIter, NodeListIter>::type 
         IterTy;

      typedef _IterTracker::IterTracker<T, KeyTy, IterTy> IterTracker;

      AVLNode<T>* _curr;
      AVLNode<T>* _prev;
      bool _prev_incr;
      OrderType _order_ty;
      Comparator* _cmp; 

      int _curr_lv;
      int _prev_lv;

      IterTracker _tracker;

      AVLNode<T>* _get_leftest_node(AVLNode<T>* tree) {
         return tree->left ? _get_leftest_node(tree->left_raw()) : tree;
      }

      AVLNode<T>* _get_rightest_node(AVLNode<T>* tree) {
         return tree->right ? _get_rightest_node(tree->right_raw()) : tree;
      }

      AVLNode<T>* _get_right_deepest_node(AVLNode<T>* tree) {
         if (tree->right)
            return _get_right_deepest_node(tree->right_raw());
         else if (tree->left)
            return _get_right_deepest_node(tree->left_raw());
         else
            return tree;
      }

      AVLNode<T>* _get_deepest_right_node_recurs(
         AVLNode<T>* tree, int* curr_lv=nullptr) {
         AVLNode<T>* left = tree->left_raw();
         AVLNode<T>* right = tree->right_raw();

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

      AVLNode<T>* _get_deepest_right_node(
         AVLNode<T>* tree, int* curr_lv=nullptr) {
         if (curr_lv)
            *curr_lv = 0;
         return _get_deepest_right_node_recurs(tree, curr_lv);
      }

      AVLNode<T>* _get_root_node(AVLNode<T>* tree) {
         return tree;
      }

      AVLNode<T>* _get_left_sibling(AVLNode<T>* tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode<T>* parent = tree->parent;
         return parent->right_raw() == tree ? parent->left_raw() : nullptr;
      }

      AVLNode<T>* _get_right_sibling(AVLNode<T>* tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode<T>* parent = tree->parent;
         return parent->left_raw() == tree ? parent->right_raw() : nullptr;
      }

      bool _visited_subtree(AVLNode<T>* subtree) {
         AVLNode<T>* child = subtree; 

         while (child) {
            if (child == _prev)
               return true;
            child = _cmp->lt(_prev->data, child->data) ? 
               child->left_raw() : child->right_raw();
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

         AVLNode<T>* parent = _curr->parent;
         return parent->left_raw() == _curr ?
            _visited_subtree(parent->right_raw()) :
            _visited_subtree(parent->left_raw());
      }

      bool _visited_parent() {
         if (!_curr)
            return false;

         AVLNode<T>* parent = _curr->parent;
         while (parent) {
            if (_prev == parent)
               return true;
            parent = parent->parent;
         }
         return false;
      }

      AVLNode<T>* _retrace_while_child(
         AVLNode<T>* n, Direction dir, int* parent_distance=nullptr) {
         assert(n && n->parent, "NullPointerError");

         int dist = 0;
         AVLNode<T>* parent = n->parent;
         AVLNodeOwner<T>* parents_child = dir == Direction::RIGHT ? 
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

      AVLNode<T>* _retrace_until_unvisited_right_child() {
         AVLNode<T>* n = _retrace_while_child(_curr, Direction::RIGHT);
         return n ? (n->right ? n->right_raw() : nullptr) : nullptr;
      }

      AVLNode<T>* _retrace_until_left_child() {
         assert(_curr && _curr->parent, "NullPointerError");

         AVLNode<T>* curr = _curr;
         AVLNode<T>* parent = curr->parent;

         while (parent && !parent->left) {
            curr = curr->parent;
            parent = curr->parent;
         }
         return parent->left_raw();
      }

      AVLNode<T>* _retrace_until_unvisited_left_child() {
         AVLNode<T>* n = _retrace_while_child(_curr, Direction::LEFT);
         return n ? (n->left ? n->left_raw() : nullptr) : nullptr;
      }

      void _in_order_increment() {
         AVLNode<T>* tmp = _curr;
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
         AVLNode<T>* tmp = _curr;
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
         AVLNode<T>* tmp = _curr;   

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
         AVLNode<T>* tmp = _curr;

         if (_prev_incr)
            _curr = _prev;
         else if (tmp) { 
            if (tmp->parent && tmp->parent->right_raw() == tmp) {
               AVLNode<T>* left_sibling = _get_left_sibling(tmp);
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
         AVLNode<T>* tmp = _curr;

         if (!_prev_incr)
            _curr = _prev;
         else if (tmp) {
            if (tmp->parent && tmp->parent->left_raw() == tmp) {
               AVLNode<T>* right_sibling = _get_right_sibling(tmp);
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
         AVLNode<T>* tmp = _curr;

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

      AVLNode<T>* _get_node_at_depth(
         AVLNode<T>* tree, int tree_lv, int want_lv, Direction dir) {
         if (!tree || tree_lv > want_lv)
            return nullptr;
         else if (tree_lv == want_lv)
            return tree;
         else if (tree_lv < want_lv) {
            AVLNode<T>* n = _get_node_at_depth(
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

      AVLNode<T>* _get_leftest_node_at_depth(
         AVLNode<T>* tree, int tree_lv, int want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::LEFT);
      }

      AVLNode<T>* _get_rightest_node_at_depth(
         AVLNode<T>* tree, int tree_lv, int want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::RIGHT);
      }

      void _get_next_sibling_positions(
         AVLNode<T>* parent, Direction dir, 
         AVLNode<T>** pos1, AVLNode<T>** pos2) {
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

      AVLNode<T>* _get_next_sibling(AVLNode<T>* n, int depth, Direction dir) {
         assert(n, "NullPointerError");

         AVLNode<T>* parent = n->parent;

         if (parent) {
            AVLNode<T>* pos1 = nullptr;
            AVLNode<T>* pos2 = nullptr;
            _get_next_sibling_positions(parent, dir, &pos1, &pos2);

            if (pos1 == n && pos2)
               return pos2;
            else if (pos2 == n) {
               int parent_distance = 0;
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

      AVLNode<T>* _get_start_node(
         AVLNode<T>* refnode, int depth, Direction dir) {
         AVLNode<T>* n = refnode;
         AVLNode<T>* prev = nullptr;
         assert(dir != Direction::ROOT, "InvalidDirectionError");

         do {
            prev = n;
         } while (n = _get_next_sibling(n, depth, dir));
         return prev;
      }

      AVLNode<T>* _get_first_node_on_lower_lv(AVLNode<T>* refnode) {
         ++_curr_lv;
         AVLNode<T>* n = 
            _get_start_node(refnode, _curr_lv - 1, Direction::LEFT);

         do {
            AVLNode<T>* first = n->left_raw();
            AVLNode<T>* second = n->right_raw();

            if (first)
               return first;
            else if (second)
               return second;
         } while (n = _get_next_sibling(n, _curr_lv - 1, Direction::RIGHT));

         --_curr_lv;
         return nullptr;
      }

      AVLNode<T>* _get_first_node_on_upper_lv(AVLNode<T>* refnode) {
         if (refnode) {
            --_curr_lv;
            return _get_start_node(refnode, _curr_lv - 1, Direction::RIGHT);
         }
         else
            return nullptr;
      }

      void _swap(int* a, int* b) {
         int tmp = *a;
         *a = *b;
         *b = tmp;
      }

      void _breadth_first_increment() {
         AVLNode<T>* tmp = _curr;

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
         AVLNode<T>* tmp = _curr;

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

      void _insertion_order_advance(bool towards_end) {
         if (towards_end) {
            if (_tracker.curr() == _tracker.end())
               _curr = nullptr;
            else if (!_curr)
               _curr = *_tracker.curr();
            else {
               ++_tracker.curr();
               _curr = _tracker.curr() == _tracker.end() ? 
                  nullptr : *_tracker.curr();
            }
         }
         else
            _curr = _tracker.curr() == _tracker.begin() ? 
               nullptr : *--_tracker.curr();
      }

      void _insertion_order_increment() {
         reverse ?
            _insertion_order_advance(/*towards_end=*/false) :
            _insertion_order_advance(/*towards_end=*/true);
      }

      void _insertion_order_decrement() {
         reverse ?
            _insertion_order_advance(/*towards_end=*/true) :
            _insertion_order_advance(/*towards_end=*/false);
      }

      std::string _node_data(AVLNode<T>* n) const {
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

      AVLNode<T>* _init_curr(const AVLNodeOwner<T>& head) {
         AVLNode<T>* raw_head = head.get();
         AVLNode<T>* curr = nullptr;

         if (raw_head) {
            if (_order_ty == OrderType::INORDER)
               curr = reverse ? 
                  _get_rightest_node(raw_head) : 
                  _get_leftest_node(raw_head);
            else if (_order_ty == OrderType::PREORDER)
               curr = reverse ?
                  _get_rightest_node(raw_head) :
                  _get_root_node(raw_head);
            else if (_order_ty == OrderType::POSTORDER)
               curr = reverse ?
                  _get_root_node(raw_head) :
                  _get_leftest_node(raw_head);
            else if (_order_ty == OrderType::BREADTHFIRST) {
               curr = reverse ?
                  _get_deepest_right_node(raw_head, &_curr_lv) :
                  _get_root_node(raw_head); 

               _curr_lv = reverse ? _curr_lv : 1;
            }
            else if (_order_ty == OrderType::INSERTION)
               curr = *_tracker.curr();
            else
               assert(false, "NotYetImplementedError");
         }

         return curr;
      }

   protected:
      _const_iterator(): 
         _curr(nullptr), 
         _prev(nullptr), 
         _prev_incr(true), 
         _order_ty(OrderType::INORDER),
         _cmp(nullptr),

         _curr_lv(0), 
         _prev_lv(0) 
         {}

      // TODO specify comparator with function templates and get 
      // `_head_type::node_ref`
      _const_iterator(Vindex* vin, OrderType order_ty): 
         _curr(nullptr), 
         _prev(nullptr),
         _prev_incr(reverse ? false : true), 
         _order_ty(order_ty),
         _cmp(nullptr),

         _curr_lv(0), 
         _prev_lv(0),

         _tracker(vin->_insertion_list) {

         auto head = vin->_heads.template
            get<_head_type::node_data>(Vindex<KeyTy, T>::_default_comparator());
         _cmp = &head->first;
         assert(_cmp, "NullPointerError");
         _curr = _init_curr(head->second);
      }

      _const_iterator(const _const_iterator& other): 
         _curr(other._curr), 
         _prev(other._prev), 
         _prev_incr(other._prev_incr), 
         _order_ty(other._order_ty), 
         _cmp(other._cmp),

         _curr_lv(other._curr_lv),
         _prev_lv(other._prev_lv),

         _tracker(other._tracker) 
         {}

      _const_iterator& operator=(const _const_iterator& other) {
         _curr = other._curr;
         _prev = other._prev;
         _prev_incr = other._prev_incr;
         _order_ty = other._order_ty;
         _cmp = other._cmp;

         _curr_lv = other._curr_lv;
         _prev_lv = other._prev_lv;

         _tracker = other._tracker;

         return *this;
      }

   public:
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
         return _curr ? _curr->data : Vindex<KeyTy, T>::_default()->data;
      }

      const T* operator->() const {
         return _curr ? &_curr->data : &Vindex<KeyTy, T>::_default()->data;
      }

      _const_iterator end() const { 
         return _const_iterator();
      }

      std::string str() const {
         return _str();
      }

      int curr_level() const {
         return _curr_lv;
      }
   };

   _Heads<T> _heads;
   OrderType _order_ty;
   const_iterator _cend;
   const_reverse_iterator _crend;
   NodeList<T> _rebalanced_trees;
   NodeList<T> _insertion_list;
   Index<T> _index;
   Extractor _get_member;
   size_t _size;

public:
   class const_iterator: public _const_iterator<false> {
   public:
      const_iterator() noexcept {}

      const_iterator(Vindex* vin, OrderType order_ty) noexcept: 
         _const_iterator<false>(vin, order_ty) {}

      const_iterator(const const_iterator& other) noexcept :
         _const_iterator<false>(other) {}

      const_iterator& operator=(const const_iterator& other) noexcept {
         _const_iterator<false>::operator=(other);   
         return *this;
      }

      const_iterator operator++() noexcept {
         _const_iterator<false>::operator++();
         return *this;
      } 

      const_iterator operator++(int val) noexcept {
         _const_iterator<false>::operator++(val);
         return *this;
      }

      const_iterator operator--() noexcept {
         _const_iterator<false>::operator--();
         return *this;
      } 

      const_iterator operator--(int val) noexcept {
         _const_iterator<false>::operator--(val);
         return *this;
      }

      const_iterator end() const noexcept {
         return const_iterator();
      }
   };

   class const_reverse_iterator: public _const_iterator<true> {
   public:
      const_reverse_iterator() noexcept {}

      const_reverse_iterator(Vindex* vin, OrderType order_ty) noexcept: 
         _const_iterator<true>(vin, order_ty) {}

      const_reverse_iterator(const const_reverse_iterator& other) noexcept:
         _const_iterator<true>(other) {}

      const_reverse_iterator& operator=(const const_reverse_iterator& other) 
         noexcept {
         _const_iterator<true>::operator=(other);   
         return *this;
      }

      const_reverse_iterator operator++() noexcept {
         _const_iterator<true>::operator--();
         return *this;
      } 

      const_reverse_iterator operator++(int val) noexcept {
         _const_iterator<true>::operator--(val);
         return *this;
      }

      const_reverse_iterator operator--() noexcept {
         _const_iterator<true>::operator++();
         return *this;
      } 

      const_reverse_iterator operator--(int val) noexcept {
         _const_iterator<true>::operator++(val);
         return *this;
      }

      const_reverse_iterator end() const noexcept {
         return const_reverse_iterator();
      }
   }; 

private:
   static AVLNode<T>* _default() {
      return &AVLNodeDefaultSingleton().get();
   }

   static DefaultComparator<T>& _default_comparator() {
      return DefaultComparatorSingleton().get();
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

   static std::string _node_data_str(AVLNode<T&>* n) {
      std::stringstream ss;
      if (n)
         ss << n->data;
      else
         ss << "null";
      return ss.str();
   }

   static std::string _node_str(const AVLNode<T&>& n) {
      std::stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", height: " << n.height 
         << ", left: " << _node_data_str(n.left_raw()) 
         << ", right: " << _node_data_str(n.right_raw())
         << ", parent: " << _node_data_str(n.parent)
         << ")";
      return ss.str();
   }

   static size_t _nodes_at_lv(size_t lv) {
      return 1 << (lv - 1);
   }

   template <typename U>
   AVLNodeOwner<U>& _first_child(AVLNode<U>* n) {
      assert(n, "NullPointerError");
      if (n->left)
         return n->left;
      else if (n->right)
         return n->right;
      else
         assert(false, "NoChildError");
   }

   template <typename U>
   int _num_children(AVLNode<U>* n) {
      int cnt = 0;

      if (!n)
         return 0;
      if (n->left)
         ++cnt;
      if (n->right)
         ++cnt;
      return cnt;
   }

   template <typename U>
   int _height(AVLNode<U>* tree) {
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

   template <typename U>
   int _balance_factor(AVLNode<U>* subtree) {
      return subtree ? 
         _height(subtree->right_raw()) - _height(subtree->left_raw()) : 0;
   }

   template <typename U>
   bool _is_left_left(AVLNode<U>* subtree) {
      return _is_left_heavy(_balance_factor(subtree->left_raw()));
   }

   template <typename U>
   bool _is_left_right(AVLNode<U>* subtree) {
      return _is_right_heavy(_balance_factor(subtree->left_raw()));
   }

   template <typename U>
   bool _is_right_right(AVLNode<U>* subtree) {
      return _is_right_heavy(_balance_factor(subtree->right_raw()));
   }

   template <typename U>
   bool _is_right_left(AVLNode<U>* subtree) {
      return _is_left_heavy(_balance_factor(subtree->right_raw()));
   }

   template <typename U>
   void _single_rotation_parts(
      const AVLNodeOwner<U>& x,
      AVLNodeOwner<U>* y, AVLNodeOwner<U>* t2, 
      AVLNodeOwner<U>** x_owner, AVLNodeOwner<U>** t2_owner,
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

   template <typename U>
   void _single_rotation_metadata_update(
      AVLNodeOwner<U>* parent, AVLNodeOwner<U>** child) {
      if (**child)
         (**child)->parent = parent->get();
      (*parent)->height = _height(parent->get());
   }

   template <typename U>
   AVLNodeOwner<U>& _modify_proxy_tree(
      AVLNodeOwner<U>* tree, const TreeEditAction<U>& edit) {

      AVLNodeOwner<U> working_tree = std::move(*tree);
      *tree = edit(&working_tree);
      return *tree;
   }

   template <typename U>
   AVLNodeOwner<U>& _single_rotation(
      AVLNodeOwner<U>* tree, Direction y_dxn) {
      return _modify_proxy_tree<U>(
         tree, 
         [&y_dxn, this](AVLNodeOwner<U>* x) -> AVLNodeOwner<U> {

         AVLNodeOwner<U> y;
         AVLNodeOwner<U> t2;
         AVLNodeOwner<U>* x_owner = nullptr;
         AVLNodeOwner<U>* t2_owner = nullptr;

         _single_rotation_parts(*x, &y, &t2, &x_owner, &t2_owner, y_dxn);
         *t2_owner = std::move(t2);
         _single_rotation_metadata_update(x, &t2_owner);
         *x_owner = std::move(*x); 
         _single_rotation_metadata_update(&y, &x_owner);

         return y;
      });
   }

   template <typename U>
   AVLNodeOwner<U>& _right_rotation(AVLNodeOwner<U>* subtree) {
      return _modify_proxy_tree<U>(
         subtree, 
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_single_rotation(working_tree, Direction::LEFT));
         });
   }

   template <typename U>
   AVLNodeOwner<U>& _left_rotation(AVLNodeOwner<U>* subtree) {
      return _modify_proxy_tree<U>(
         subtree, 
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_single_rotation(working_tree, Direction::RIGHT));
         });
   }

   template <typename U>
   AVLNodeOwner<U>& _left_right_double_rotation(
      AVLNodeOwner<U>* subtree) {
      _modify_proxy_tree<U>(
         &(*subtree)->left,
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_left_rotation(working_tree));
         });
      return _modify_proxy_tree<U>(
         subtree,
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_right_rotation(working_tree));
         });
   }

   template <typename U>
   AVLNodeOwner<U>& _right_left_double_rotation(
      AVLNodeOwner<U>* subtree) {
       _modify_proxy_tree<U>(
         &(*subtree)->right,
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_right_rotation(working_tree));
         });
      return _modify_proxy_tree<U>(
         subtree,
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_left_rotation(working_tree));
         });
   }

   template <typename U>
   AVLNodeOwner<U>& _rebalance(AVLNodeOwner<U>* subtree) {
      int bf = _balance_factor(subtree->get());
      AVLNodeOwner<U>* balanced_tree = nullptr;

      if (_is_too_heavy(bf)) {
         balanced_tree = &_modify_proxy_tree<U>(
            subtree,
            [this, &bf](
               AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
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
      }
      else
         balanced_tree = subtree;

      return *balanced_tree;
   }

   template <typename U>
   AVLNodeOwner<U>& _child_insertion_side(
      AVLNodeOwner<U>* n, 
      AVLNodeOwner<U>* subtree, 
      const Comparator& cmp) {

      return cmp.lt((*n)->data, (*subtree)->data) ?
         (*subtree)->left : (*subtree)->right;
   }

   template <typename U>
   AVLNodeOwner<U>& _insert_recurs(
      AVLNodeOwner<U>* n, 
      AVLNodeOwner<U>* subtree, 
      AVLNode<U>** result,
      const Comparator& cmp) {

      (*n)->parent = subtree->get();

      AVLNodeOwner<U>& child_tree = 
         _child_insertion_side(n, subtree, cmp);

      if (child_tree) {
         _modify_proxy_tree<U>(&child_tree,
            [this, &n, &subtree, &result, &cmp](
               AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
               return std::move(
                  _insert_recurs(n, working_tree, result, cmp));
            });
         child_tree->parent = subtree->get();
      }
      else {
         child_tree = std::move(*n);
         *result = child_tree.get();
      }

      (*subtree)->height = _height(subtree->get());

      return _modify_proxy_tree<U>(
         subtree,
         [this](AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            return std::move(_rebalance(working_tree));
         });
   }

   template <typename U>
   AVLNode<U>*  _insert(
      AVLNodeOwner<U>* n, 
      AVLNodeOwner<U>* head, 
      const Comparator& cmp) {

      ++(*n)->height;

      if (!*head) {
         *head = std::move(*n);
         ++(*head)->height;
         return head->get();
      }

      AVLNode<U>* result;
      *head = std::move(_insert_recurs(n, head, &result, cmp));
      (*head)->parent = nullptr;
      return result;
   }

   AVLNode<T>* _insert_each_head(const T& val) {
      AVLNodeOwner<T> real_n = std::make_unique<AVLNode<T>>(val);
      T& data = real_n->data;
      auto head = _heads.template
         get<_head_type::node_data>(_default_comparator());
      assert(head, "InvalidHeadError");
      AVLNode<T>* rtn = _insert(&real_n, &head->second, head->first);

      for (auto head_it = _heads.begin(); head_it != _heads.end(); ++head_it) {
         AVLNodeOwner<T&> ref_n = std::make_unique<AVLNode<T&>>(data); 
         _insert(&ref_n, &head_it->second, head_it->first);
      }

      return rtn;
   }

   template <typename U>
   AVLNodeOwner<U> _on_removal_leaf(
      AVLNodeOwner<U>* n, AVLNodeOwner<U>* rm = nullptr) {
      assert(n, "NullPointerError");
      assert(!(*n)->left && !(*n)->right, "NotLeafError");

      if (rm)
         *rm = std::move(*n);
      return AVLNodeOwner<U>();
   }

   template <typename U>
   AVLNodeOwner<U>& _on_removal_one_child(
      AVLNodeOwner<U>* n, AVLNodeOwner<U>* rm = nullptr) {
      assert(*n, "NullPointerError");
      assert(_num_children(n->get()) == 1, "MustHaveExactlyOneChildError");

      return _modify_proxy_tree<U>(n,
         [this, &rm](
            AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            AVLNodeOwner<U> child = 
               std::move(_first_child(working_tree->get()));

            AVLNodeOwner<U> tmp;
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

   template <typename U>
   void _transfer_to_next(AVLNodeOwner<U>* next, AVLNodeOwner<U>* temp) {
      (*next)->left = std::move((*temp)->left);
      (*next)->right = std::move((*temp)->right);
      (*next)->parent = (*temp)->parent;
      (*next)->height = (*temp)->height;   

      if ((*next)->left)
         (*next)->left->parent = next->get();
      if ((*next)->right)
         (*next)->right->parent = next->get();
   }

   template <typename U>
   AVLNodeOwner<U> _rm_next_in_order(
      AVLNodeOwner<U>* tree, AVLNodeOwner<U>* next) {
      if ((*tree)->left) {
         return std::move(_modify_proxy_tree<U>(&(*tree)->left,
            [this, &next](
               AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
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

   template <typename U>
   AVLNodeOwner<U>& _on_removal_two_children(
      AVLNodeOwner<U>* n, AVLNodeOwner<U>* rm = nullptr) {
      assert(n, "NullPointerError");

      return  _modify_proxy_tree<U>(n, 
         [this, &n, &rm](
            AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            AVLNodeOwner<U> next;
            AVLNodeOwner<U> tmp;
            if (!rm)
               rm = &tmp;

            (*working_tree)->right = 
               std::move(_rm_next_in_order(&(*working_tree)->right, &next));
            (*working_tree)->height = _height(working_tree->get());
            _transfer_to_next(&next, working_tree);
            *rm = std::move(*working_tree);
            return next;
         }
      );
   }

   template <typename U>
   AVLNodeOwner<U> _remove_and_rebalance(
      const T& val, AVLNodeOwner<U>* subtree, 
      AVLNode<U>* parent, const Comparator& cmp) {

      AVLNodeOwner<U> rm;

      _modify_proxy_tree<U>(subtree,
         [this, &val, &cmp, &rm](
            AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {

            *working_tree = 
               std::move(_remove(val, working_tree, cmp, &rm));
            return std::move(_rebalance(working_tree));
         });

      if (*subtree)
         (*subtree)->parent = parent;

      return rm;
   }

   template <typename U>
   AVLNodeOwner<U>& _remove(
      const T& val, AVLNodeOwner<U>* tree, 
      const Comparator& cmp, AVLNodeOwner<U>* rm) {
      if (!*tree) {
         *rm = nullptr;
         return *tree;
      }

      if (cmp.lt(val, (*tree)->data))
         *rm = _remove_and_rebalance(val, &(*tree)->left, tree->get(), cmp);
      else if (cmp.gt(val, (*tree)->data))
         *rm = _remove_and_rebalance(val, &(*tree)->right, tree->get(), cmp);
      else {
         if (_num_children(tree->get()) == 1)
            *tree = std::move(_on_removal_one_child(tree, rm));
         else if (!_num_children(tree->get()))
            *tree = std::move(_on_removal_leaf(tree, rm));
         else
            *tree = std::move(_on_removal_two_children(tree, rm));
      }

      if (*tree)
         (*tree)->height = _height(tree->get());
      return *tree;
   }

   bool _is_dq_all_nulls(const NodeDQ<T&>& dq) const {
      auto it = find_if(
         dq.begin(), dq.end(), [](AVLNode<T&>* n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_per_line(NodeDQ<T&>* dq, const VoidFunc& func) const {
      func();
      if (_is_dq_all_nulls(*dq))
         dq->clear();
   }

   void _on_valid_node(
      NodeDQ<T&>* dq, AVLNode<T&>* n, const NodeListener<T&>& func) const {
      func(n);
      dq->push_back(n->left_raw());
      dq->push_back(n->right_raw());
   }

   void _on_null_node(NodeDQ<T&>* dq, const NodeListener<T&>& func) const {
      func(nullptr);
      dq->push_back(nullptr);
      dq->push_back(nullptr);
   }

   void _gather_bfs(NodeDQ<T&>* dq, size_t* curr_depth,
      size_t* node_cnt, const NodeListener<T&>& func) const {

      if (dq->empty()) 
         return;

      AVLNode<T&>* n = dq->front();
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

   template <typename CmpTy>
   NodeList<T&> _gather_bfs_list(const CmpTy& cmp) const {
      NodeDQ<T&> dq;
      NodeList<T&> nl;
      size_t curr_depth = 1;
      size_t node_cnt = 0;
      auto head = _heads.template get<_head_type::node_ref>(cmp);

      if (head->second)
         dq.push_back(head->second.get());

      _gather_bfs(&dq, &curr_depth, &node_cnt,
         [&nl](AVLNode<T&>* n) { nl.push_back(n); });
      return nl;
   }

   template <typename CmpTy>
   std::string _gather_bfs_str(
      const std::string& delim, const CmpTy& cmp) const {
      NodeList<T&> nl = _gather_bfs_list(cmp);
      std::stringstream ss;
      int node_cnt = 0;
      int curr_depth = 1;

      for (auto nit = nl.begin(); nit != nl.end(); ++nit) {
         AVLNode<T&>* n = *nit;
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

   template <typename CmpTy=DefaultComparator<T>>
   std::string _bfs_str(
      const std::string& delim = "|",
      const CmpTy& cmp=_default_comparator()) const {
      return _gather_bfs_str(delim, cmp);
   }

   std::string _index_str(const std::string& delim = "|") const {
      std::stringstream ss;
      for (auto it = _index.begin(); it != _index.end(); ++it) {
         ss << it->first << ": " << _node_str(*it->second) << delim;
      }
      return ss.str();
   }

   _Heads<T> _init_heads() {
      _Heads<T> heads;
      heads.push(_default_comparator());
      heads.push(_default_comparator());
      return heads;
   }

public:
   Vindex(const Extractor& get_member) noexcept: 
      _heads(_init_heads()),
      _order_ty(OrderType::INORDER), 
      _get_member(get_member),
      _size(0)
      {}

   ConstResult<T&> insert(const T& val) noexcept {
      if (_index.find(_get_member(val)) != _index.end())
         return std::make_unique<ConstResultFailure<T&>>();
      AVLNode<T>* n = _insert_each_head(val);
      ++_size;
      _insertion_list.emplace_back(n);
      _index[_get_member(n->data)] = n;
      return std::make_unique<ConstResultSuccess<T&>>(n->data);
   }

   template <typename... Args>
   void emplace(Args&&... args) noexcept {
      insert(T(std::forward<Args>(args)...));
   }

   Result<T> remove(const T& val) noexcept {
      for (auto head_it = _heads.begin(); head_it != _heads.end(); ++head_it) {
         _remove_and_rebalance<T&>(
            val, &head_it->second, nullptr, head_it->first);
      }

      auto head = _heads.template
         get<_head_type::node_data>(_default_comparator());
      AVLNodeOwner<T> rm =
         _remove_and_rebalance<T>(val, &head->second, nullptr, head->first);

      if (rm) {
         _insertion_list.remove(rm.get());
         _index.erase(_get_member(rm->data));
         return std::make_unique<ResultSuccess<T>>(rm->data);
      }
      return std::make_unique<ResultFailure<T>>();
   }

   void order(OrderType order_ty) noexcept {
      _order_ty = order_ty;
   }

   OrderType order() const noexcept {
      return _order_ty;
   }

   const_iterator cbegin() noexcept {
      const_iterator it(this, _order_ty);
      _cend = it.end();
      return it;
   }

   const_iterator cend() noexcept {
      return _cend;
   }

   const_reverse_iterator crbegin() noexcept {
      const_reverse_iterator it(this, _order_ty);
      _crend = it.end();
      return it;
   }

   const_reverse_iterator crend() noexcept {
      return _crend;
   }

   const_iterator find(const KeyTy& key) noexcept {
      auto it = cbegin();
      return std::find_if(it, cend(), [this, key](const T& elem) -> bool {
         return key == _get_member(elem);
      });
   }

   const T& at(const KeyTy& key) const {
      return _index.at(key)->data;
   }

   size_t size() noexcept {
      return _size;
   }

   void clear() noexcept {
      _insertion_list.clear();
      _index.clear();
      auto head = _heads.template
         get<_head_type::node_data>(_default_comparator());
      for (auto head_it = _heads.begin(); head_it != _heads.end(); ++head_it)
         head_it->second = nullptr;
      head->second = nullptr;
      _size = 0;
   }
};

#ifdef assert
#undef assert
#endif

#endif
