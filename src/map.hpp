/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   // Internal treap node
   struct Node {
       value_type data;
       Node *left, *right, *parent;
       unsigned priority;
       Node(const value_type &v, unsigned p): data(v), left(nullptr), right(nullptr), parent(nullptr), priority(p) {}
   };
   class const_iterator;
   class iterator {
      private:
       Node *node = nullptr;
       map *owner = nullptr;
      public:
       iterator() {}

       iterator(const iterator &other) { node = other.node; owner = other.owner; }

       iterator operator++(int) {
           iterator tmp(*this);
           ++(*this);
           return tmp;
       }

       iterator &operator++() {
           if (owner == nullptr || (node == nullptr)) throw invalid_iterator();
           // next: if right child, go to leftmost of right; else go up until we are left child
           if (node->right) {
               node = node->right;
               while (node->left) node = node->left;
               return *this;
           }
           auto cur = node; auto p = node->parent;
           while (p && cur == p->right) { cur = p; p = p->parent; }
           node = p; // may become nullptr -> end()
           return *this;
       }

       iterator operator--(int) { iterator tmp(*this); --(*this); return tmp; }

       iterator &operator--() {
           if (owner == nullptr) throw invalid_iterator();
           if (node == nullptr) {
               // from end() to max element
               if (owner->root == nullptr) throw invalid_iterator();
               node = owner->root;
               while (node->right) node = node->right;
               return *this;
           }
           if (node->left) {
               node = node->left;
               while (node->right) node = node->right;
               return *this;
           }
           auto cur = node; auto p = node->parent;
           while (p && cur == p->left) { cur = p; p = p->parent; }
           if (p == nullptr) throw invalid_iterator(); // --begin()
           node = p;
           return *this;
       }

       value_type &operator*() const { if (node == nullptr) throw invalid_iterator(); return node->data; }

       bool operator==(const iterator &rhs) const { return node == rhs.node && owner == rhs.owner; }

       bool operator==(const const_iterator &rhs) const { return owner == rhs.owner && node == rhs.node; }

       bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

       bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

       value_type *operator->() const noexcept { return &(node->data); }
       friend class map;
   };
   class const_iterator {
       // it should has similar member method as iterator.
       //  and it should be able to construct from an iterator.
      private:
       Node *node = nullptr;
       const map *owner = nullptr;
      public:
       const_iterator() {}

       const_iterator(const const_iterator &other) { node = other.node; owner = other.owner; }

       const_iterator(const iterator &other) { node = other.node; owner = other.owner; }
       // And other methods in iterator.
       // And other methods in iterator.
       // And other methods in iterator.
       const_iterator operator++(int) { const_iterator tmp(*this); ++(*this); return tmp; }
       const_iterator &operator++() {
           if (owner == nullptr || node == nullptr) throw invalid_iterator();
           if (node->right) { node = node->right; while (node->left) node = node->left; return *this; }
           auto cur = node; auto p = node->parent;
           while (p && cur == p->right) { cur = p; p = p->parent; }
           node = p; return *this;
       }
       const_iterator operator--(int) { const_iterator tmp(*this); --(*this); return tmp; }
       const_iterator &operator--() {
           if (owner == nullptr) throw invalid_iterator();
           if (node == nullptr) {
               if (owner->root == nullptr) throw invalid_iterator();
               node = owner->root; while (node->right) node = node->right; return *this;
           }
           if (node->left) { node = node->left; while (node->right) node = node->right; return *this; }
           auto cur = node; auto p = node->parent;
           while (p && cur == p->left) { cur = p; p = p->parent; }
           if (p == nullptr) throw invalid_iterator();
           node = p; return *this;
       }
       const value_type &operator*() const { if (node == nullptr) throw invalid_iterator(); return node->data; }
       const value_type *operator->() const noexcept { return &(node->data); }
       bool operator==(const const_iterator &rhs) const { return node == rhs.node && owner == rhs.owner; }
       bool operator==(const iterator &rhs) const { return owner == rhs.owner && node == rhs.node; }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
       bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
       friend class map;
   };

   /**
  * TODO two constructors
    */
   map() : root(nullptr), node_count(0), comp(Compare()), rnd_seed(88172645463393265ull) {}

   map(const map &other) : root(nullptr), node_count(0), comp(other.comp), rnd_seed(88172645463393265ull) {
       for (auto it = other.cbegin(); it != other.cend(); ++it) insert(*it);
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this == &other) return *this;
       clear(); comp = other.comp; rnd_seed = 88172645463393265ull;
       for (auto it = other.cbegin(); it != other.cend(); ++it) insert(*it);
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() { clear(); }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *p = find_node(key);
       if (!p) throw index_out_of_bound();
       return p->data.second;
   }

   const T &at(const Key &key) const {
       Node *p = const_cast<map*>(this)->find_node(key);
       if (!p) throw index_out_of_bound();
       return p->data.second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       auto pr = insert(value_type(key, T()));
       return pr.first.node->data.second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const { return at(key); }

   /**
  * return a iterator to the beginning
    */
   iterator begin() { iterator it; it.owner = this; it.node = minimum(root); return it; }

   const_iterator cbegin() const { const_iterator it; it.owner = this; it.node = minimum(root); return it; }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() { iterator it; it.owner = this; it.node = nullptr; return it; }

   const_iterator cend() const { const_iterator it; it.owner = this; it.node = nullptr; return it; }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const { return node_count == 0; }

   /**
  * returns the number of elements.
    */
   size_t size() const { return node_count; }

   /**
  * clears the contents
    */
   void clear() { destroy(root); root = nullptr; node_count = 0; }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *exist = find_node(value.first);
       if (exist) { iterator it; it.owner = this; it.node = exist; return pair<iterator, bool>(it, false); }
       Node *n = new Node(value, next_rand());
       bst_insert(n);
       // sift up to maintain heap property (min-heap on priority)
       while (n->parent && n->priority < n->parent->priority) {
           if (n == n->parent->left) rotate_right(n->parent); else rotate_left(n->parent);
       }
       ++node_count;
       iterator it; it.owner = this; it.node = n; return pair<iterator, bool>(it, true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.owner != this || pos.node == nullptr) throw invalid_iterator();
       Node *z = pos.node;
       // rotate down until at most one child remains
       while (z->left && z->right) {
           if (z->left->priority < z->right->priority) rotate_right(z); else rotate_left(z);
       }
       // now replace z with its single child or null
       Node *child = z->left ? z->left : z->right;
       transplant(z, child);
       delete z; --node_count;
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const { return const_cast<map*>(this)->find_node(key) ? 1 : 0; }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) { iterator it; it.owner = this; it.node = find_node(key); return it.node ? it : end(); }

   const_iterator find(const Key &key) const { const_iterator it; it.owner = this; it.node = const_cast<map*>(this)->find_node(key); return it.node ? it : cend(); }
  private:
   Node *root = nullptr;
   size_t node_count = 0;
   Compare comp;
   unsigned long long rnd_seed;

   // xorshift64*
   unsigned next_rand() {
       rnd_seed ^= rnd_seed >> 12;
       rnd_seed ^= rnd_seed << 25;
       rnd_seed ^= rnd_seed >> 27;
       return (unsigned)((rnd_seed * 2685821657736338717ull) >> 32);
   }

   void destroy(Node *x) {
       if (!x) return;
       destroy(x->left);
       destroy(x->right);
       delete x;
   }

   Node *minimum(Node *x) const {
       if (!x) return nullptr;
       while (x->left) x = x->left;
       return x;
   }
   Node *maximum(Node *x) const {
       if (!x) return nullptr;
       while (x->right) x = x->right;
       return x;
   }

   bool key_less(const Key &a, const Key &b) const { return comp(a, b); }
   bool key_equal(const Key &a, const Key &b) const { return !comp(a, b) && !comp(b, a); }

   Node *find_node(const Key &key) {
       Node *cur = root;
       while (cur) {
           if (key_less(key, cur->data.first)) cur = cur->left;
           else if (key_less(cur->data.first, key)) cur = cur->right;
           else return cur;
       }
       return nullptr;
   }

   void bst_insert(Node *n) {
       if (root == nullptr) { root = n; n->parent = nullptr; return; }
       Node *cur = root, *p = nullptr;
       while (cur) {
           p = cur;
           if (key_less(n->data.first, cur->data.first)) cur = cur->left; else cur = cur->right;
       }
       n->parent = p;
       if (key_less(n->data.first, p->data.first)) p->left = n; else p->right = n;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nullptr) root = v;
       else if (u == u->parent->left) u->parent->left = v; else u->parent->right = v;
       if (v) v->parent = u->parent;
   }

   void rotate_left(Node *x) {
       Node *y = x->right; // should exist
       Node *B = y->left;
       // link y to x's parent
       y->parent = x->parent;
       if (x->parent == nullptr) root = y;
       else if (x == x->parent->left) x->parent->left = y; else x->parent->right = y;
       // put x on y's left
       y->left = x; x->parent = y;
       // move B as x->right
       x->right = B; if (B) B->parent = x;
   }
   void rotate_right(Node *x) {
       Node *y = x->left;
       Node *B = y->right;
       y->parent = x->parent;
       if (x->parent == nullptr) root = y;
       else if (x == x->parent->left) x->parent->left = y; else x->parent->right = y;
       y->right = x; x->parent = y;
       x->left = B; if (B) B->parent = x;
   }
};

}

#endif
