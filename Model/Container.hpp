#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <string>
#include <functional>
#include "ContainerException.hpp"

////////////////////////////////////////////////////////////////////////////////
// INTERFACCIA /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Forwarding
template <class T, class K>
class iterator;

// Container
template <class T, class K = std::string>
class Container
{
  friend class iterator<T, K>;
  
private:

  // Nodi nel bucket
  class node
  {
    friend class iterator<T, K>;
    
  public:
    K key;
    T info;
    node *next;

    node(const K &, const T &, node * = nullptr);
  };

  // Metodi per la gestione dei bucket
  static void destroy(node *&);
  static node * search(node *, const K &);
  static node *& search_unsafe(node *&, const K &);
  static void insert(node *&, const K &, const T &);
  static void remove(node *&);
  static node * copy(node *);

  // table
  static const unsigned int INIT_TABLE_LENGTH = 5;
  
  node **table;
  
  unsigned int tableLength;

  unsigned int tableSize;

  std::hash<K> keyHash;
  
  unsigned int getIndex(const K &) const;

public:
  
  Container();
  Container(const Container &);
  ~Container();

  unsigned int size() const;

  bool empty() const;

  T & get(const K &) const;

  void put(const K &, const T &);

  void remove(const K &);

  Container<T, K> & operator=(const Container<T, K> &);

  class iterator
  {
  private:
    const Container<T, K> * const ref;
    node ** itArray;
    const unsigned int itArrayLength;
    unsigned int itPos;
    bool end;
    
  public:
    iterator(Container<T, K> const *);
    ~iterator();
    
    T & operator*();
    T * operator->();
    
    K getKey();
    bool isEnd();
    iterator & operator++();
    iterator operator++(int);
  };

  // Metodi per la gestione degli iteratori
  iterator begin() const;
};

////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTAZIONI /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// IMPLEMENTAZIONE BUCKET
template <class T, class K>
Container<T, K>::node::node(const K &k, const T &obj, node *n) : key(k), info(obj), next(n) {}

template <class T, class K>
void Container<T, K>::destroy(node *&n)
{
  if (!n) return;
  
  if (n->next) destroy(n->next);

  delete n;

  n = nullptr;
}

template <class T, class K>
typename Container<T, K>::node * Container<T, K>::search(node *n, const K &k)
{
  if (!n) return nullptr;
  
  if (n->key == k)
    return n;

  return search(n->next, k);
}

template <class T, class K>
typename Container<T, K>::node *& Container<T, K>::search_unsafe(node *&n, const K &k)
{
  if (!n) throw ContainerCellNotFoundException();

  if (n->key == k)
    return n;

  return search_unsafe(n->next, k);
}

template <class T, class K>
void Container<T, K>::insert(node *&n, const K &k, const T &obj)
{
    n = new node(k, obj, n);
}

template <class T, class K>
void Container<T, K>::remove(node *&n)
{
  node *tmp = n->next;
  delete n;
  n = tmp;
}

template <class T, class K>
typename Container<T, K>::node * Container<T, K>::copy(node *n)
{
  if (!n) return nullptr;

  return new node(n->key, n->info, copy(n->next));
}

// IMPLEMENTAZIONE TABLE
template <class T, class K>
unsigned int Container<T, K>::getIndex(const K &k) const
{
  return keyHash(k) % tableLength;
}

template <class T, class K>
Container<T, K>::Container() : table(new node*[INIT_TABLE_LENGTH]),
			       tableLength(INIT_TABLE_LENGTH),
			       tableSize(0)
{  
  for (unsigned int i = 0; i < tableLength; ++i)
    table[i] = nullptr;
}

template <class T, class K>
Container<T, K>::Container(const Container &c) : table(new node*[c.tableLength]),
						 tableLength(c.tableLength),
						 tableSize(c.tableSize)
{
  for (unsigned int i = 0; i < tableSize; ++i)
    table[i] = copy(c.table[i]);
}

template <class T, class K>
Container<T, K>::~Container()
{
  for (unsigned int i = 0; i < tableLength; ++i)
    destroy(table[i]);

  delete[] table;
}

template <class T, class K>
unsigned int Container<T, K>::size() const
{
  return tableSize;
}

template <class T, class K>
bool Container<T, K>::empty() const
{
  return tableSize == 0;
}

template <class T, class K>
T & Container<T, K>::get(const K &k) const
{
  if (tableSize == 0) throw ContainerEmptyTableException();

  node *tmp = search(table[getIndex(k)], k);

  if (!tmp) throw ContainerCellNotFoundException();

  return tmp->info;
}

template <class T, class K>
void Container<T, K>::put(const K &k, const T &obj)
{
  if(search(table[getIndex(k)], k)) throw ContainerDuplicateKeyException();
  
  insert(table[getIndex(k)], k, obj);
  tableSize++;
}

template <class T, class K>
void Container<T, K>::remove(const K &k)
{
  if (tableSize == 0) throw ContainerEmptyTableException();

  remove(search_unsafe(table[getIndex(k)], k));
  
  tableSize--;
}

 template <class T, class K>
 Container<T, K> & Container<T, K>::operator=(const Container<T, K> &c)
 {
   if (table == c.table)
     return *this;

   for (unsigned int i = 0; i < tableLength; ++i)
     destroy(table[i]);

   for (unsigned int i = 0; i < tableSize; ++i)
     table[i] = copy(c.table[i]);
 }

// IMPLEMENTAZIONE ITERATORI
template <class T, class K>
Container<T, K>::iterator::iterator(const Container<T, K> *c) :
  ref(c),
  itArray(new node*[c->tableSize]),
  itArrayLength(c->tableSize),
  itPos(0),
  end(false)
{
  if (c->empty())
    {
      delete[] itArray;
      itArray = nullptr;
      end = true;
      throw ContainerEmptyTableException();
    }

  unsigned int k = 0;

  for (unsigned int i = 0; i < c->tableLength; ++i)
      for (node *tmp = c->table[i]; tmp; tmp = tmp->next)
        itArray[k++] = tmp;

  if (k != itArrayLength) throw ContainerException();
}

template <class T, class K>
Container<T, K>::iterator::~iterator()
{
  delete[] itArray;
}

template <class T, class K>
T & Container<T, K>::iterator::operator*()
{
  return itArray[itPos]->info;
}

template <class T, class K>
T * Container<T, K>::iterator::operator->()
{
  return &(itArray[itPos]->info);
}

template <class T, class K>
K Container<T, K>::iterator::getKey()
{
  return itArray[itPos]->key;
}

template <class T, class K>
bool Container<T, K>::iterator::isEnd()
{
  return end;
}

template <class T, class K>
typename Container<T, K>::iterator & Container<T, K>::iterator::operator++()
{  
  if (++itPos >= itArrayLength)
    end = true;

  return *this;
}

template <class T, class K>
typename Container<T, K>::iterator Container<T, K>::iterator::operator++(int)
{
  auto tmp(*this);
  operator++();
  return tmp;
}

template <class T, class K>
typename Container<T, K>::iterator Container<T, K>::begin() const
{
  return iterator(this);
}

#endif // CONTAINER_HPP
