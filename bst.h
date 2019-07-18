#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <iostream>
#include <cassert>
#include <list>
#include <deque>
#include <functional>
#include <algorithm>
#include <sstream>
#include <vector>
#include <mutex>
#include <utility>

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
   static size_t combine_hashes(const std::vector<size_t>& hashes) {
      size_t res = 17;
      for (auto it = hashes.begin(); it != hashes.end(); ++it)
         res = res*31 + *it;
      return res;
   }

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
      return !(lt(a, b) && gt(a, b)); 
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

namespace HeadType {
   struct node_ref {};
   struct node_data {};
}

template <typename T>
struct Node {
   T data;

   Node() {}
   Node(const T& data): data(data) {}
};

template <typename T, typename U=decltype(T::data)>
struct BSTState: public T {
   std::unique_ptr<BSTState> left;
   std::unique_ptr<BSTState> right;
   BSTState *parent;

   BSTState(): parent(nullptr) {}
   BSTState(const U& data): T(data), parent(nullptr) {}
};

template <typename T>
class BST;

template <typename T>
class _Heads {
   friend class BST<T>;

private:
   typedef IComparator<T> Comparator;
   typedef std::unique_ptr<Comparator> ComparatorOwner; 
   typedef BSTState<Node<T>> NodeData;
   typedef BSTState<Node<T&>> NodeRef;
   typedef std::unique_ptr<NodeData> NodeDataOwner;
   typedef std::unique_ptr<NodeRef> NodeRefOwner;

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
      friend class _Heads<T>;

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

      iterator& end() {
         _iter = _heads->end();
         return *this;
      }
   };

private:
   iterator _iter;

   static size_t hash_comparator(const ComparatorOwner& c) {
      return std::hash<Comparator>()(*c);
   }

   static bool is_equal_comparator(
      const ComparatorOwner& c1, const ComparatorOwner& c2) {
      return *c1 == *c2;
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>> 
      _get(const ComparatorTy& cmp, HeadType::node_ref) {

      auto it = _secondary_heads.find(std::make_unique<ComparatorTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, NodeRefOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>> 
      _get(const ComparatorTy& cmp, HeadType::node_ref) const {

      auto it = _secondary_heads.find(std::make_unique<ComparatorTy>(cmp));
      if (it != _secondary_heads.end())
         return std::make_unique<std::pair<Comparator&, const NodeRefOwner&>>(
            *it->first, it->second);
      return std::unique_ptr<std::pair<Comparator&, const NodeRefOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>> 
      _get(const ComparatorTy& cmp, HeadType::node_data) {

      if (_primary_head.first && cmp == *_primary_head.first)
         return std::make_unique<std::pair<Comparator&, NodeDataOwner&>>(
            *_primary_head.first, _primary_head.second);
      return std::unique_ptr<std::pair<Comparator&, NodeDataOwner&>>();
   }

   template <typename ComparatorTy>
   std::unique_ptr<std::pair<Comparator&, const NodeDataOwner&>> 
      _get(const ComparatorTy& cmp, HeadType::node_data) const {

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
            std::is_same<HeadTy, HeadType::node_ref>::value, 
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
            std::is_same<HeadTy, HeadType::node_ref>::value, 
            NodeRefOwner&, 
            NodeDataOwner&
         >::type
      >
   > get(const ComparatorTy& cmp) {
      return _get(cmp, HeadTy());
   }

   iterator begin() {
      _iter = iterator(_secondary_heads);
      return _iter;
   }

   iterator end() {
      iterator iter = _iter;
      return iter.end();
   }
};

template <typename T>
class BST {
   friend class _Heads<T>;

private:
   template <typename U>
   using BSTNode = BSTState<Node<U>>;

   template <typename U>
   using BSTNodeOwner = std::unique_ptr<BSTNode<U>>;

   typedef std::list<BSTNode<T&>*> NodeList;
   typedef std::deque<BSTNode<T&>*> NodeDQ;
   typedef std::function<void(BSTNode<T&>*)> NodeListener;

   typedef typename _Heads<T>::Comparator Comparator;
   typedef typename _Heads<T>::NodeRefOwner NodeRefOwner;

   struct DefaultComparatorSingleton:
      public _Singleton<DefaultComparator<T>, DefaultComparatorSingleton> {
      static void init(DefaultComparator<T>* dc) {}
   };

   _Heads<T> _heads;
   NodeList _insertion;

   static DefaultComparator<T> default_comparator() {
      return DefaultComparatorSingleton().get();
   }

   static size_t _nodes_at_lv(size_t lv) {
      return 1 << (lv - 1);
   }

   static std::string _node_data_str(BSTNode<T&>* n) {
      using namespace std;
      stringstream ss;
      if (n)
         ss << n->data;
      else
         ss << "null";
      return ss.str();
   }

   static std::string _node_str(const BSTNode<T&> &n) {
      std::stringstream ss;
      ss << "(" 
         << "data: " << n.data 
         << ", left: " << _node_data_str(n.left.get()) 
         << ", right: " << _node_data_str(n.right.get())
         << ", parent: " << _node_data_str(n.parent)
         << ")";
      return ss.str();
   }

   bool _is_dq_all_nulls(const NodeDQ &dq) const {
      auto it = find_if(
         dq.begin(), dq.end(), [](BSTNode<T&> *n) -> bool { return n; });
      return it == dq.end();
   }

   void _on_max_nodes_per_line(
      NodeDQ *dq, const std::function<void()> &func) const {
      func();
      if (_is_dq_all_nulls(*dq))
         dq->clear();
   }

   void _on_valid_node(
      NodeDQ *dq, BSTNode<T&> *n, const NodeListener &func) const {
      func(n);
      dq->push_back(n->left.get());
      dq->push_back(n->right.get());
   }

   void _on_null_node(NodeDQ *dq, const NodeListener &func) const {
      func(nullptr);
      dq->push_back(nullptr);
      dq->push_back(nullptr);
   }

   void _gather_bfs(NodeDQ *dq, size_t *curr_depth,
      size_t *node_cnt, const NodeListener &func) const {

      if (dq->empty()) 
         return;

      BSTNode<T&> *n = dq->front();
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

   template <typename ComparatorTy=DefaultComparator<T>>
   NodeList _gather_bfs_list(const ComparatorTy& cmp) const {
      NodeDQ dq;
      NodeList nl;
      size_t curr_depth = 1;
      size_t node_cnt = 0;
      auto entry = _heads.template get<HeadType::node_ref>(cmp);

      assert(entry);

      const NodeRefOwner& head = entry->second;
      if (head.get())
         dq.push_back(head.get());

      _gather_bfs(&dq, &curr_depth, &node_cnt,
         [&nl](BSTNode<T&> *n) { nl.push_back(n); });
      return nl;
   }

   template <typename ComparatorTy=DefaultComparator<T>>
   std::string _gather_bfs_str(
      const std::string &delim, const ComparatorTy& cmp) const {
      NodeList nl = _gather_bfs_list(cmp);
      std::stringstream ss;
      int node_cnt = 0;
      int curr_depth = 1;

      for (auto nit = nl.begin(); nit != nl.end(); ++nit) {
         BSTNode<T&> *n = *nit;
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

   template <typename DATA_TY>
   void _insert_recurs(
      BSTNodeOwner<DATA_TY>* n, BSTNodeOwner<DATA_TY>* subtree, 
      const Comparator& cmp, BSTNode<DATA_TY>* parent) {

      if (*subtree) {
         if (cmp.lt((*n)->data, (*subtree)->data))
            _insert_recurs(n, &(*subtree)->left, cmp, subtree->get());
         else
            _insert_recurs(n, &(*subtree)->right, cmp, subtree->get());
      }
      else {
         *subtree = std::move(*n);   
         (*subtree)->parent = parent;
      }
   }

   template <typename DATA_TY>
   void _insert(BSTNodeOwner<DATA_TY>* n, BSTNodeOwner<DATA_TY>* head,
      const Comparator& cmp) {

      if (!*head) {
         *head = std::move(*n);
         return;
      }
      _insert_recurs(n, head, cmp, head->get());
   }

   _Heads<T> _init_heads() {
      _Heads<T> heads;
      heads.push(default_comparator());
      heads.push(default_comparator());
      return heads;
   }

public:
   BST(): 
      _heads(_init_heads())
      {}

   template <typename ComparatorTy>
   void push_comparator(const ComparatorTy& cmp) {
      _heads.push(cmp);
      auto clone_entry = _heads.template get<HeadType::node_ref>(cmp);
      assert(clone_entry);

      for (auto it = _insertion.begin(); it != _insertion.end(); ++it) {
         auto clone_n = std::make_unique<BSTNode<T&>>((*it)->data);
         _insert(&clone_n, &clone_entry->second, clone_entry->first);
      }
   }

   void insert(const T& x) {
      DefaultComparator<T> dc;
      auto real_n = std::make_unique<BSTNode<T>>(x);
      T& data = real_n->data;
      auto real_entry = _heads.template get<HeadType::node_data>(dc);
      assert(real_entry);

      _insert(&real_n, &real_entry->second, real_entry->first);  

      for (auto it = _heads.begin(); it != _heads.end(); ++it) {
         auto clone_n = std::make_unique<BSTNode<T&>>(data);
         BSTNode<T&>* raw_clone_n = clone_n.get();

         _insert(&clone_n, &it->second, it->first);

         if (dynamic_cast<DefaultComparator<T>*>(&it->first))
            _insertion.emplace_back(raw_clone_n);
      }
   }   

   template <typename ComparatorTy=DefaultComparator<T>>
   std::string str(
      const std::string &delim = "|", 
      const ComparatorTy& cmp=default_comparator()) const {
      return _gather_bfs_str(delim, cmp);
   }
};
