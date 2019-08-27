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
#include <typeinfo>
#include <tuple>

#pragma push_macro("assert")
#ifdef assert
#undef assert
#endif

#pragma push_macro("NDEBUG")
#ifdef NDEBUG
#undef NDEBUG
#endif

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

#if defined(_MSC_VER) && _MSC_VER < 1800 || \
   !defined(_MSC_VER) && __cplusplus <= 201103L 
namespace std {
   template <typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }
}
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900 || \
   !defined(_MSC_VER) && __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif

#define make_extractor(CLS, MEM)\
   [](const CLS& obj) -> decltype(CLS::MEM) { return obj.MEM; }

#define make_vindex(CLS, MEM)\
   Vindex<decltype(CLS::MEM), CLS>(make_extractor(CLS, MEM))

template <typename T, typename KeyTy> class Vindex;

namespace hash_helpers {
   inline size_t combine(const std::vector<size_t>& hashes) NOEXCEPT {
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
         const typename 
         Vindex<KeyTy, NodeDataTy>::template NodeList<NodeDataTy&>& l):
         IterTrackerBase<IterTy>(l.begin(), l.end()) {}
   };

   template <typename T, typename KeyTy>
   using NodeListRevIter = 
      typename Vindex<KeyTy, T>::template NodeList<T&>::const_reverse_iterator;

   template <typename NodeDataTy, typename KeyTy>
   class IterTracker<
         NodeDataTy, 
         KeyTy,
         NodeListRevIter<NodeDataTy, KeyTy>
      >: public IterTrackerBase<NodeListRevIter<NodeDataTy, KeyTy>> {
   public:
      IterTracker(): IterTrackerBase<NodeListRevIter<NodeDataTy, KeyTy>>() {}

      IterTracker(
         const typename 
         Vindex<KeyTy, NodeDataTy>::template NodeList<NodeDataTy&>& l):
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
   ConstResultSuccess& operator=(const ConstResultSuccess&) = delete;

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
      return dynamic_cast<const DefaultComparator *>(&other) != nullptr; 
   }

   bool lt(const T& a, const T& b) const override { 
      return a < b; 
   }
};

template <typename T>
struct _Node {
   typedef T DataTy;
   T data;
   _Node() {}
   _Node(const T& data): data(data) {}

private:
   _Node& operator=(const _Node&) = delete;
};

template <typename T, typename BASE_TY=typename T::DataTy>
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
      std::pair<Comparator&, NodeRefOwner&>
   > {
   private:
      typedef decltype(_Heads<T>::_secondary_heads) HeadTy;
      typedef std::pair<Comparator&, NodeRefOwner&> Entry;
      typedef std::vector<Entry> EntryArray;
      typedef typename EntryArray::iterator EntryArrayIter;

      std::shared_ptr<EntryArray> _heads;
      EntryArrayIter _iter;

   public:
      iterator() {}

      iterator(HeadTy* heads): 
         _heads(std::make_shared<EntryArray>()) {
         for (auto it = heads->begin(); it != heads->end(); ++it) {
            _heads->emplace_back(std::make_pair<
               std::reference_wrapper<Comparator>, 
               std::reference_wrapper<NodeRefOwner>
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

   template <typename CmpTy>
   std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>> 
      _get(const CmpTy& cmp, _head_type::node_ref) {

      auto it = _secondary_heads.find(std::make_unique<CmpTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>>();
   }

   template <typename CmpTy>
   std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>> 
      _get(const CmpTy& cmp, _head_type::node_ref) const {

      auto it = _secondary_heads.find(std::make_unique<CmpTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, const NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>>();
   }

   template <typename CmpTy>
   std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>> 
      _get(const CmpTy& cmp, _head_type::node_data) {

      if (_primary_head.first && cmp == *_primary_head.first)
         return std::make_unique<std::pair<Comparator&, NodeDataOwner&>>(
            *_primary_head.first, _primary_head.second);
      return std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>>();
   }

   template <typename CmpTy>
   std::unique_ptr<std::pair<Comparator&, const NodeDataOwner&>> 
      _get(const CmpTy& cmp, _head_type::node_data) const {

      if (_primary_head.first && cmp == *_primary_head.first)
         return std::make_unique<std::pair<Comparator&, const NodeDataOwner&>>(
            *_primary_head.first, _primary_head.second);
      return std::unique_ptr<std::pair<Comparator&, const NodeDataOwner&>>();
   }

public:
   _Heads(): _secondary_heads(0, hash_comparator, is_equal_comparator) {}

   _Heads(_Heads&& other) {
      _primary_head = std::move(other._primary_head);
      _secondary_heads = std::move(other._secondary_heads);
   };

   template <typename CmpTy>
   void push(const CmpTy& cmp) { 
      ComparatorOwner new_cmp = std::make_unique<CmpTy>(cmp);
      if (!_primary_head.first)
         _primary_head.first = std::move(new_cmp);
      else if (_secondary_heads.find(new_cmp) == _secondary_heads.end())
         _secondary_heads[std::move(new_cmp)];
   }

   template <typename HeadTy, typename CmpTy>
   std::unique_ptr<
      std::pair<
         Comparator&, 
         typename std::conditional<
            std::is_same<HeadTy, _head_type::node_ref>::value, 
            const NodeRefOwner&, 
            const NodeDataOwner&
         >::type
      >
   > get(const CmpTy& cmp) const {
      return _get(cmp, HeadTy());
   }

   template <typename HeadTy, typename CmpTy>
   std::unique_ptr<
      std::pair<
         Comparator&, 
         typename std::conditional<
            std::is_same<HeadTy, _head_type::node_ref>::value, 
            NodeRefOwner&, 
            NodeDataOwner&
         >::type
      >
   > get(const CmpTy& cmp) {
      return _get(cmp, HeadTy());
   }

   iterator begin() {
      iterator it(&_secondary_heads);
      _end_iter = it.end();
      return it;
   }

   iterator end() {
      return _end_iter;
   }

   template <typename CmpTy>
   bool exists(const CmpTy& cmp) const {
      return static_cast<bool>(get<_head_type::node_ref>(cmp));
   }
};

template <typename KeyTy, typename T>
class Vindex {
   static_assert(
      !std::is_lvalue_reference<T>::value && !std::is_pointer<T>::value, 
      "T cannot be an lvalue reference");
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
   using NodeTracker = std::tuple<
      AVLNode<U>*, 
      const Comparator&, 
      AVLNodeOwner<U>*
   >;

   template <typename U>
   using Index = std::unordered_map<
      KeyTy, 
      std::list<NodeTracker<U>>
   >;

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
      static void init(DefaultComparator<T>*) {}
   };

   template <bool reverse>
   class _const_iterator : 
      public std::iterator<std::bidirectional_iterator_tag, T> {
   private:
      typedef typename Vindex<KeyTy, T>::Direction Direction;
      typedef typename _Heads<T>::Comparator Comparator;

      template <typename U>
      using AVLNode = Vindex::AVLNode<U>;

      template <typename U>
      using AVLNodeOwner = Vindex::AVLNodeOwner<U>;

      typedef 
         typename 
         Vindex<KeyTy, T>::template NodeList<T&>::const_iterator 
         NodeListIter;

      typedef 
         typename 
         Vindex<KeyTy, T>::template NodeList<T&>::const_reverse_iterator 
         NodeListRevIter;

      typedef 
         typename 
            std::conditional<reverse, NodeListRevIter, NodeListIter>::type 
         IterTy;

      typedef _IterTracker::IterTracker<T, KeyTy, IterTy> IterTracker;

      AVLNode<T&>* _curr;
      AVLNode<T&>* _prev;
      bool _prev_incr;
      OrderType _order_ty;
      Comparator* _cmp; 

      int _curr_lv;
      int _prev_lv;

      IterTracker _tracker;

      AVLNode<T&>* _get_leftest_node(AVLNode<T&>* tree) {
         return tree->left ? _get_leftest_node(tree->left_raw()) : tree;
      }

      AVLNode<T&>* _get_rightest_node(AVLNode<T&>* tree) {
         return tree->right ? _get_rightest_node(tree->right_raw()) : tree;
      }

      AVLNode<T&>* _get_right_deepest_node(AVLNode<T&>* tree) {
         if (tree->right)
            return _get_right_deepest_node(tree->right_raw());
         else if (tree->left)
            return _get_right_deepest_node(tree->left_raw());
         else
            return tree;
      }

      AVLNode<T&>* _get_deepest_right_node_recurs(
         AVLNode<T&>* tree, int* curr_lv=nullptr) {
         AVLNode<T&>* left = tree->left_raw();
         AVLNode<T&>* right = tree->right_raw();

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

      AVLNode<T&>* _get_deepest_right_node(
         AVLNode<T&>* tree, int* curr_lv=nullptr) {
         if (curr_lv)
            *curr_lv = 0;
         return _get_deepest_right_node_recurs(tree, curr_lv);
      }

      AVLNode<T&>* _get_root_node(AVLNode<T&>* tree) {
         return tree;
      }

      AVLNode<T&>* _get_left_sibling(AVLNode<T&>* tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode<T&>* parent = tree->parent;
         return parent->right_raw() == tree ? parent->left_raw() : nullptr;
      }

      AVLNode<T&>* _get_right_sibling(AVLNode<T&>* tree) {
         assert(tree && tree->parent, "NullPointerError");
         AVLNode<T&>* parent = tree->parent;
         return parent->left_raw() == tree ? parent->right_raw() : nullptr;
      }

      bool _visited_subtree(AVLNode<T&>* subtree) {
         AVLNode<T&>* child = subtree; 

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

         AVLNode<T&>* parent = _curr->parent;
         while (parent) {
            if (_prev == parent)
               return true;
            parent = parent->parent;
         }
         return false;
      }

      AVLNode<T&>* _retrace_while_child(
         AVLNode<T&>* n, Direction dir, int* parent_distance=nullptr) {
         assert(n && n->parent, "NullPointerError");

         int dist = 0;
         AVLNode<T&>* parent = n->parent;
         AVLNodeOwner<T&>* parents_child = dir == Direction::RIGHT ? 
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

      AVLNode<T&>* _retrace_until_unvisited_right_child() {
         AVLNode<T&>* n = _retrace_while_child(_curr, Direction::RIGHT);
         return n ? (n->right ? n->right_raw() : nullptr) : nullptr;
      }

      AVLNode<T&>* _retrace_until_left_child() {
         assert(_curr && _curr->parent, "NullPointerError");

         AVLNode<T&>* curr = _curr;
         AVLNode<T&>* parent = curr->parent;

         while (parent && !parent->left) {
            curr = curr->parent;
            parent = curr->parent;
         }
         return parent->left_raw();
      }

      AVLNode<T&>* _retrace_until_unvisited_left_child() {
         AVLNode<T&>* n = _retrace_while_child(_curr, Direction::LEFT);
         return n ? (n->left ? n->left_raw() : nullptr) : nullptr;
      }

      void _in_order_increment() {
         AVLNode<T&>* tmp = _curr;
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
         AVLNode<T&>* tmp = _curr;
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
         AVLNode<T&>* tmp = _curr;   

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
         AVLNode<T&>* tmp = _curr;

         if (_prev_incr)
            _curr = _prev;
         else if (tmp) { 
            if (tmp->parent && tmp->parent->right_raw() == tmp) {
               AVLNode<T&>* left_sibling = _get_left_sibling(tmp);
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
         AVLNode<T&>* tmp = _curr;

         if (!_prev_incr)
            _curr = _prev;
         else if (tmp) {
            if (tmp->parent && tmp->parent->left_raw() == tmp) {
               AVLNode<T&>* right_sibling = _get_right_sibling(tmp);
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
         AVLNode<T&>* tmp = _curr;

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

      AVLNode<T&>* _get_node_at_depth(
         AVLNode<T&>* tree, int tree_lv, int want_lv, Direction dir) {
         if (!tree || tree_lv > want_lv)
            return nullptr;
         else if (tree_lv == want_lv)
            return tree;
         else if (tree_lv < want_lv) {
            AVLNode<T&>* n = _get_node_at_depth(
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

      AVLNode<T&>* _get_leftest_node_at_depth(
         AVLNode<T&>* tree, int tree_lv, int want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::LEFT);
      }

      AVLNode<T&>* _get_rightest_node_at_depth(
         AVLNode<T&>* tree, int tree_lv, int want_lv) {
         return _get_node_at_depth(tree, tree_lv, want_lv, Direction::RIGHT);
      }

      void _get_next_sibling_positions(
         AVLNode<T&>* parent, Direction dir, 
         AVLNode<T&>** pos1, AVLNode<T&>** pos2) {
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

      AVLNode<T&>* _get_next_sibling(AVLNode<T&>* n, int depth, Direction dir) {
         assert(n, "NullPointerError");

         AVLNode<T&>* parent = n->parent;

         if (parent) {
            AVLNode<T&>* pos1 = nullptr;
            AVLNode<T&>* pos2 = nullptr;
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

      AVLNode<T&>* _get_start_node(
         AVLNode<T&>* refnode, int depth, Direction dir) {
         AVLNode<T&>* n = refnode;
         AVLNode<T&>* prev = nullptr;
         assert(dir != Direction::ROOT, "InvalidDirectionError");

         do {
            prev = n;
            n = _get_next_sibling(n, depth, dir);
         } while (n);
         return prev;
      }

      AVLNode<T&>* _get_first_node_on_lower_lv(AVLNode<T&>* refnode) {
         ++_curr_lv;
         AVLNode<T&>* n = 
            _get_start_node(refnode, _curr_lv - 1, Direction::LEFT);

         do {
            AVLNode<T&>* first = n->left_raw();
            AVLNode<T&>* second = n->right_raw();

            if (first)
               return first;
            else if (second)
               return second;
            n = _get_next_sibling(n, _curr_lv - 1, Direction::RIGHT);
         } while (n);

         --_curr_lv;
         return nullptr;
      }

      AVLNode<T&>* _get_first_node_on_upper_lv(AVLNode<T&>* refnode) {
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
         AVLNode<T&>* tmp = _curr;

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
         AVLNode<T&>* tmp = _curr;

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

      AVLNode<T&>* _init_curr(const AVLNodeOwner<T&>& head) {
         AVLNode<T&>* raw_head = head.get();
         AVLNode<T&>* curr = nullptr;

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

      _const_iterator(const NodeList<T&>& insertion_list, OrderType order_ty):
         _curr(nullptr), 
         _prev(nullptr),
         _prev_incr(reverse ? false : true), 
         _order_ty(order_ty),
         _cmp(nullptr),

         _curr_lv(0), 
         _prev_lv(0),

         _tracker(insertion_list) 
         {}

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

      template <typename CmpTy>
      void _init_curr_from_cmp(const _Heads<T>& heads, const CmpTy& cmp) {
         auto head = heads.template get<_head_type::node_ref>(cmp);
         assert(head, 
            "CmpHead" << typeid(cmp).name() << "EntryDoesNotExistError");
         _cmp = &head->first;
         _curr = _init_curr(head->second);
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

public:
   class const_iterator: public _const_iterator<false> {
   public:
      const_iterator() NOEXCEPT {}

      const_iterator(
         const NodeList<T&>& insertion_list, OrderType order_ty) NOEXCEPT: 
         _const_iterator<false>(insertion_list, order_ty) {}

      const_iterator(const const_iterator& other) NOEXCEPT :
         _const_iterator<false>(other) {}


      template <typename CmpTy>
      void init_from_cmp(const _Heads<T>& heads, const CmpTy& cmp) {
         _const_iterator<false>::_init_curr_from_cmp(heads, cmp);   
      }

      const_iterator& operator=(const const_iterator& other) NOEXCEPT {
         _const_iterator<false>::operator=(other);   
         return *this;
      }

      const_iterator operator++() NOEXCEPT {
         _const_iterator<false>::operator++();
         return *this;
      } 

      const_iterator operator++(int) NOEXCEPT {
         const_iterator tmp(*this);
         operator++();
         return tmp;
      }

      const_iterator operator--() NOEXCEPT {
         _const_iterator<false>::operator--();
         return *this;
      } 

      const_iterator operator--(int) NOEXCEPT {
         const_iterator tmp(*this);
         operator--();
         return tmp;
      }

      const_iterator end() const NOEXCEPT {
         return const_iterator();
      }
   };

   class const_reverse_iterator: public _const_iterator<true> {
   public:
      const_reverse_iterator() NOEXCEPT {}

      const_reverse_iterator(
         const NodeList<T&>& insertion_list, OrderType order_ty) NOEXCEPT: 
         _const_iterator<true>(insertion_list, order_ty) {}

      const_reverse_iterator(const const_reverse_iterator& other) NOEXCEPT:
         _const_iterator<true>(other) {}

      template <typename CmpTy>
      void init_from_cmp(const _Heads<T>& heads, const CmpTy& cmp) {
         _const_iterator<true>::_init_curr_from_cmp(heads, cmp);   
      }

      const_reverse_iterator& operator=(const const_reverse_iterator& other) 
         NOEXCEPT {
         _const_iterator<true>::operator=(other);   
         return *this;
      }

      const_reverse_iterator operator++() NOEXCEPT {
         _const_iterator<true>::operator--();
         return *this;
      } 

      const_reverse_iterator operator++(int) NOEXCEPT {
         const_reverse_iterator tmp(*this);
         operator++();
         return tmp;
      }

      const_reverse_iterator operator--() NOEXCEPT {
         _const_iterator<true>::operator++();
         return *this;
      } 

      const_reverse_iterator operator--(int) NOEXCEPT {
         const_reverse_iterator tmp(*this);
         operator--();
         return tmp;
      }

      const_reverse_iterator end() const NOEXCEPT {
         return const_reverse_iterator();
      }
   }; 

private:
   _Heads<T> _heads;
   const_iterator _cend;
   const_reverse_iterator _crend;
   NodeList<T&> _insertion_list;
   Index<T&> _index;
   Extractor _get_member;
   size_t _size;

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

   template <typename U>
   static std::string _node_data_str(AVLNode<U>* n) {
      std::stringstream ss;
      if (n)
         ss << n->data;
      else
         ss << "null";
      return ss.str();
   }

   template <typename U>
   static std::string _node_str(const AVLNode<U>& n) {
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

   template <typename U>
   static std::string _node_str(AVLNode<U>* n) {
      std::stringstream ss;

      if (n)
         return _node_str(*n);
      else {
         ss << "(null)";
         return ss.str();
      }
   }

   static size_t _nodes_at_lv(size_t lv) {
      return 1ULL << (lv - 1);
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
   AVLNodeOwner<U> _rebalance(AVLNodeOwner<U>* subtree) {
      int bf = _balance_factor(subtree->get());

      if (_is_too_heavy(bf)) {
         _modify_proxy_tree<U>(
            subtree,
            [this, &bf](
               AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {

               if (_is_too_left_heavy(bf))
                  return _is_left_right(working_tree->get()) ?
                     std::move(
                        _left_right_double_rotation(working_tree)) :
                     std::move(_right_rotation(working_tree));
               else if (_is_too_right_heavy(bf))
                  return _is_right_left(working_tree->get()) ?
                     std::move(
                        _right_left_double_rotation(working_tree)) :
                     std::move(_left_rotation(working_tree));
               else
                  assert(false, "InvalidHeavyStateError: "
                     << _bfs_str_immed("\n", *working_tree));
            }
         );
      }

      return std::move(*subtree);
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
            return _rebalance(working_tree);
         });
   }

   template <typename U>
   AVLNode<U>* _insert(
      AVLNodeOwner<U>* n, 
      AVLNodeOwner<U>* head, 
      const Comparator& cmp) {

      ++(*n)->height;

      if (!*head) {
         *head = std::move(*n);
         return head->get();
      }

      AVLNode<U>* result;
      *head = std::move(_insert_recurs(n, head, &result, cmp));
      (*head)->parent = nullptr;
      return result;
   }

   std::list<NodeTracker<T&>> _insert_each_head(const T& val) {
      AVLNodeOwner<T> real_n = std::make_unique<AVLNode<T>>(val);
      T& data = real_n->data;
      auto head = _heads.template
         get<_head_type::node_data>(_default_comparator());
      assert(head, "InvalidHeadError");
      _insert(&real_n, &head->second, head->first);

      std::list<NodeTracker<T&>> rtn;

      for (auto head_it = _heads.begin(); head_it != _heads.end(); ++head_it) {
         AVLNodeOwner<T&> ref_n = std::make_unique<AVLNode<T&>>(data); 
         AVLNode<T&>* n = _insert(&ref_n, &head_it->second, head_it->first);

         std::tuple<AVLNode<T&>*, const Comparator&, AVLNodeOwner<T&>*> p(
            n, head_it->first, &head_it->second);

         head_it->first == _default_comparator() ?
            rtn.emplace_front(std::move(p)) :
            rtn.emplace_back(std::move(p));
      }

      assert(!rtn.empty(), "EmptyListError");
      return std::move(rtn);
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
      _modify_proxy_tree<U>(
         tree,
         [this, &next](
            AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {

            if ((*working_tree)->left) {
               (*working_tree)->left = 
                  std::move(_rm_next_in_order(&(*working_tree)->left, next));
               if (*working_tree)
                  (*working_tree)->height = _height(working_tree->get());
               return std::move(*working_tree);
            }
            else {
               if ((*working_tree)->right)
                  return std::move(_on_removal_one_child(working_tree, next));
               else
                  return std::move(_on_removal_leaf(working_tree, next));
            }
         }
      );

      return std::move(*tree);
   }

   template <typename U>
   AVLNodeOwner<U>& _on_removal_two_children(
      AVLNodeOwner<U>* n, AVLNodeOwner<U>* rm = nullptr) {
      assert(n, "NullPointerError");

      return  _modify_proxy_tree<U>(n, 
         [this, &rm](
            AVLNodeOwner<U>* working_tree) -> AVLNodeOwner<U> {
            AVLNodeOwner<U> next;
            AVLNodeOwner<U> tmp;
            if (!rm)
               rm = &tmp;

            (*working_tree)->right = 
               _rm_next_in_order(&(*working_tree)->right, &next);
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
            return _rebalance(working_tree);
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

   template <typename U>
   bool _is_dq_all_nulls(const NodeDQ<U>& dq) const {
      auto it = find_if(
         dq.begin(), dq.end(), 
            [](AVLNode<U>* n) -> bool { return n != nullptr; });
      return it == dq.end();
   }

   template <typename U>
   void _on_max_nodes_per_line(NodeDQ<U>* dq, const VoidFunc& func) const {
      func();
      if (_is_dq_all_nulls(*dq))
         dq->clear();
   }

   template <typename U>
   void _on_valid_node(
      NodeDQ<U>* dq, AVLNode<U>* n, const NodeListener<U>& func) const {
      func(n);
      dq->push_back(n->left_raw());
      dq->push_back(n->right_raw());
   }

   template <typename U>
   void _on_null_node(NodeDQ<U>* dq, const NodeListener<U>& func) const {
      func(nullptr);
      dq->push_back(nullptr);
      dq->push_back(nullptr);
   }

   template <typename U>
   void _gather_bfs(NodeDQ<U>* dq, size_t* curr_depth,
      size_t* node_cnt, const NodeListener<U>& func) const {

      if (dq->empty()) 
         return;

      AVLNode<U>* n = dq->front();
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

   template <typename U>
   NodeList<U> _gather_bfs_list(const AVLNodeOwner<U>& root) const {
      NodeDQ<U> dq;
      NodeList<U> nl;
      size_t curr_depth = 1;
      size_t node_cnt = 0;

      if (root)
         dq.push_back(root.get());

      _gather_bfs<U>(&dq, &curr_depth, &node_cnt,
         [&nl](AVLNode<U>* n) { nl.push_back(n); });
      return nl;
   }

   template <typename U>
   std::string _gather_bfs_str(
      const std::string& delim, const AVLNodeOwner<U>& root) const {
      NodeList<U> nl = _gather_bfs_list<U>(root);
      std::stringstream ss;
      int node_cnt = 0;
      int curr_depth = 1;

      for (auto nit = nl.begin(); nit != nl.end(); ++nit) {
         AVLNode<U>* n = *nit;
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

      auto head = _heads.template get<_head_type::node_ref>(cmp);
      return _gather_bfs_str<T&>(delim, head->second);
   }

   template <typename U>
   std::string _bfs_str_immed(
      const std::string& delim, const AVLNodeOwner<U>& subtree) const {
      return _gather_bfs_str<U>(delim, subtree);
   }

   template <typename U>
   std::string _bfs_str_immed(
      const AVLNodeOwner<U>& subtree) const {
      return _gather_bfs_str<U>("|", subtree);
   }

   std::string _index_str(const std::string& delim = "|") const {
      std::stringstream ss;
      for (auto it = _index.begin(); it != _index.end(); ++it) {
         AVLNode<T&>* n = std::get<0>(it->second.front());
         ss << it->first << ": "<< _node_str(*n) << delim;
      }
      return ss.str();
   }

   _Heads<T> _init_heads() {
      _Heads<T> heads;
      heads.push(_default_comparator());
      heads.push(_default_comparator());
      return heads;
   }

   template <typename U>
   AVLNodeOwner<U>* _get_owner(AVLNode<U>* n, AVLNodeOwner<U>* root) {
      if (n->parent) {
         if (n->parent->left.get() == n)
            return &n->parent->left;
         else if (n->parent->right.get() == n)
            return &n->parent->right;
         else
            assert(false, 
               "ParentChildError: "
               << std::endl
               << "child: " << _node_str(*n) 
               << std::endl
               << "parent: " << _node_str(*n->parent)
               << std::endl
            );
      }
      else
         return root;
   }

   template <typename U>
   void _remove_and_rebalance_iteratively(
      AVLNode<U>* n, const Comparator& cmp, AVLNodeOwner<U>* root) {
      AVLNodeOwner<U>* owner = _get_owner(n, root);
      AVLNode<U>* next_node = n->parent;

      _remove_and_rebalance<U>(n->data, owner, n->parent, cmp);

      while (next_node) {
         AVLNode<U>* parent = next_node->parent;
         AVLNodeOwner<U>* subtree = _get_owner(next_node, root);
         *subtree = _rebalance(subtree);
         (*subtree)->height = _height(subtree->get());
         (*subtree)->parent = parent;
         next_node = parent;
      }
   }

public:
   Vindex(const Extractor& get_member) NOEXCEPT: 
      _heads(_init_heads()),
      _get_member(get_member),
      _size(0)
      {}

   Vindex(Vindex&& other) NOEXCEPT: 
      _get_member(std::move(other._get_member)),
      _heads(std::move(other._heads)),
      _insertion_list(std::move(other._insertion_list)),
      _index(std::move(other._index)),
      _size(std::move(other._size)), 
      _cend(std::move(other._cend)),
      _crend(std::move(other._crend)) 
      {}

   Vindex& operator=(Vindex&& other) NOEXCEPT {
      _get_member = std::move(other._get_member);
      _heads = std::move(other._heads);
      _insertion_list = std::move(other._insertion_list);
      _index = std::move(other._index);
      _size = std::move(other._size);
      _cend = std::move(other._cend);
      _crend = std::move(other._crend);
      return *this;
   }

   template <typename ComparatorTy>
   void push_comparator(const ComparatorTy& cmp) NOEXCEPT {
      static_assert(std::is_base_of<IComparator<T>, ComparatorTy>::value,
         "ComparatorTy must be derived from IComparator<T>");
      if (_heads.exists(cmp))
         return;
      _heads.push(cmp);
      auto head = _heads.template get<_head_type::node_ref>(cmp);
      assert(head, "NullPointerError");

      for (
         auto it = _insertion_list.begin(); 
         it != _insertion_list.end(); 
         ++it) {

         AVLNodeOwner<T&> n = std::make_unique<AVLNode<T&>>((*it)->data);
         AVLNode<T&>* np = _insert(&n, &head->second, head->first);
         _index.at(_get_member(np->data)).emplace_back(
            np, head->first, &head->second);
      }
   }

   ConstResult<T&> insert(const T& val) NOEXCEPT {
      if (_index.find(_get_member(val)) != _index.end())
         return std::make_unique<ConstResultFailure<T&>>();
      std::list<NodeTracker<T&>> nodes = _insert_each_head(val);
      assert(!nodes.empty(), "EmptyListError");
      ++_size;
      AVLNode<T&>* default_n = std::get<0>(nodes.front());
      _insertion_list.emplace_back(default_n);
      _index[_get_member(default_n->data)] = std::move(nodes);
      return std::make_unique<ConstResultSuccess<T&>>(default_n->data);
   }

   template <typename... Args>
   ConstResult<T&> emplace(Args&&... args) NOEXCEPT {
      return insert(T(std::forward<Args>(args)...));
   }

   Result<T> remove(const T& val) NOEXCEPT {
      const KeyTy& key = _get_member(val);

      if (!_index.count(key))
         return std::make_unique<ResultFailure<T>>();

      for (auto& node_tracker : _index.at(key)) {
         AVLNode<T&>* n = std::get<0>(node_tracker);
         const Comparator& cmp = std::get<1>(node_tracker);
         AVLNodeOwner<T&>* root = std::get<2>(node_tracker);

         _remove_and_rebalance_iteratively<T&>(n, cmp, root);

         if (cmp == _default_comparator())
            _insertion_list.remove(n);
      }
      _index.erase(key);

      auto head = _heads.template
         get<_head_type::node_data>(_default_comparator());
      AVLNodeOwner<T> rm =
          _remove_and_rebalance<T>(val, &head->second, nullptr, head->first);

      assert(rm, "NullPointerError");
      --_size;
      return std::make_unique<ResultSuccess<T>>(rm->data);
   }

   Result<T> remove(const KeyTy& key) NOEXCEPT {
      try {
         const T& val = at(key);
         Result<T> res = remove(val);
         assert(dynamic_cast<ResultSuccess<T>*>(res.get()) != nullptr, 
            "NullPointerError");
         return std::move(res);
      }
      catch (std::out_of_range&) {
         return std::make_unique<ResultFailure<T>>();
      }
   }

   template <typename CmpTy=DefaultComparator<T>> 
   const_iterator cbegin(
      OrderType order_ty, const CmpTy& cmp=_default_comparator()) NOEXCEPT {
      if (!_heads.exists(cmp))
         push_comparator(cmp);

      const_iterator it(_insertion_list, order_ty);
      it.init_from_cmp(_heads, cmp);
      _cend = it.end();
      return it;
   }

   const_iterator cend() NOEXCEPT {
      return _cend;
   }

   template <typename CmpTy=DefaultComparator<T>>
   const_reverse_iterator crbegin(
      OrderType order_ty, const CmpTy& cmp=_default_comparator()) NOEXCEPT {
      if (!_heads.exists(cmp))
         push_comparator(cmp);

      const_reverse_iterator it(_insertion_list, order_ty);
      it.init_from_cmp(_heads, cmp);
      _crend = it.end();
      return it;
   }

   const_reverse_iterator crend() NOEXCEPT {
      return _crend;
   }

   template <typename CmpTy=DefaultComparator<T>>
   const_iterator find(
      const KeyTy& key,
      OrderType order_ty, 
      const CmpTy& cmp=_default_comparator()) NOEXCEPT {

      auto it = cbegin(order_ty, cmp);
      return std::find_if(it, cend(), [this, key](const T& elem) -> bool {
         return key == _get_member(elem);
      });
   }

   const T& at(const KeyTy& key) const {
      return std::get<0>(_index.at(key).front())->data;
   }

   size_t size() NOEXCEPT {
      return _size;
   }

   void clear() NOEXCEPT {
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

#pragma pop_macro("NDEBUG")
#pragma pop_macro("assert")

#endif
