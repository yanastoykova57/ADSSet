#ifndef ADS_set_first
#define ADS_set_first

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator;
  using key_compare = std::less<key_type>;                         // B+-Tree
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

private:
    
    struct Entry {
        key_type key;
        Entry* head = nullptr;
        Entry* next = nullptr;
        Entry() : head{nullptr}, next{nullptr} {}
        ~Entry() {
            delete head;
            delete next;
        };
    };
    
    Entry* table = nullptr;
    size_type maxsz{N};
    size_type currsz{0};
    float lf{0.7};
    
    void add_entry(const key_type &key) {
        size_type idx{h(key)};

        Entry* ne = new Entry(); // (key) additional
        ne->key = key;

        if(table[idx].head == nullptr){
            table[idx].head = ne;
        } else if (table[idx].head != nullptr){
            ne->next = table[idx].head;
            table[idx].head = ne;
        }
        
        /*ne->next = table[idx].head;
        table[idx].head = ne;*/
        
        ++currsz;
        
        reserve(currsz+1);
    }
    
    Entry* find_entry (const key_type &key) const {
        
        size_type idx{h(key)};

        if (table == nullptr || table[idx].head == nullptr) return nullptr;
        Entry* help = table[idx].head;
        
        
            while(help != nullptr){
                if(key_equal{}(help->key, key)){
                    return help;
                }
                help = help->next;
            }
       
        return nullptr;
    }
    
    size_type h(const key_type& key) const {return hasher {} (key) % maxsz;}
    
    void reserve(size_type n){
        if (maxsz*lf >= n) return;
        size_type newsz {maxsz};
        while(maxsz*lf <= n){
            ++(newsz*=2);
            rehash(newsz);
        }
    }
    
    void rehash(size_type n) {
        size_type new_maxsz = {std::max(N, std::max(n, size_type(currsz / lf)))};
        
        Entry* new_table = new Entry[new_maxsz]();
        Entry* old_table = table;
        size_type old_maxsz = maxsz;
        
        currsz = 0;
        table = new_table;
        maxsz = new_maxsz;
        
        for(size_type i{0}; i < old_maxsz; ++i) {
           Entry* temp = old_table[i].head;
            while(temp){
            add_entry(temp->key);
            temp = temp->next;
                }
        }
        
        /*for(size_type i{0}; i < old_maxsz; ++i) {
            for(Entry* temp = old_table[i].head; temp != nullptr; temp = temp->next){
                add_entry(temp->key);
                }
        }*/
        
        delete[] old_table;
    }
    
    
public:
    ADS_set(): table{new Entry[N]} {} // PH1
    
    ADS_set(std::initializer_list<key_type> ilist): ADS_set() {insert(ilist);}                     // PH1
    
    template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} {insert(first,last);}     // PH1
  
    ADS_set(const ADS_set &other) : ADS_set() {for(const auto& p: other) add_entry(p);}

   ~ADS_set() {
    delete[] table;
   }

    ADS_set &operator=(const ADS_set &other){ //lecture
        ADS_set help{other};
        swap(help);
        return *this;
    }

    ADS_set &operator=(std::initializer_list<key_type> ilist){ //lecture
        ADS_set help{ilist};
        swap(help);
        return *this;
    }

    size_type size() const {return currsz;}                                             // PH1
    
    bool empty() const {return currsz == 0;}      
    
    template<typename InputIt> void insert(InputIt first, InputIt last){
        for(auto it{first}; it != last; ++it){
            if(!count(*it)){
                add_entry(*it);
            }
        }
    }                                           

    void insert(std::initializer_list<key_type> ilist) {insert(ilist.begin(),ilist.end());}                 
  
    std::pair<iterator,bool> insert(const key_type &key){ //lecture
        if(Entry* e{find_entry(key)}){
            //return std::make_pair(iterator{this, e, h(key), maxsz}, false);
            //return std::pair<Iterator,bool>(iterator{this, e, h(key), maxsz}, false);
            return {iterator{this, e, h(key), maxsz}, false};
        }
        reserve(currsz+1);
        add_entry(key);
        //return std::make_pair(iterator{this, find_entry(key), h(key), maxsz}, true);
        //return std::pair<Iterator,bool>(iterator{this, find_entry(key), h(key), maxsz}, true);
        return {iterator{this, find_entry(key), h(key), maxsz}, true};
    }

    void clear() { //lecture
        ADS_set tmp{};
        swap(tmp);
    }

    size_type erase(const key_type &key) {
        if (count(key)) { // if the key is already in the table:
                  auto idx = h(key); // hash the key to find
                  
                  Entry *current = table[idx].head;
                  Entry *previous = nullptr;
                  
                  
                  while(current != nullptr){
                      if (key_equal{}(current->key, key)) { // if you find the key:
                          if (current == table[idx].head) { // if the current entry is equal to the head
                              table[idx].head = current->next; // head moves to next entry
                              current->next = nullptr; // next entry is set back to nullptr
                          }
                          if (previous != nullptr){
                              previous->next = current->next;
                              current->next = nullptr;
                              
                              previous = nullptr;
                              delete previous;
                          }
                          delete current;
                          --currsz;
                          return 1;
                      }
                      previous = current;
                      current = current->next;
                  }
              }
              return 0;
    }

    size_type count(const key_type &key) const {return (find_entry(key) != nullptr);}
    
    iterator find(const key_type &key) const { //lecture
         if(Entry* e{find_entry(key)}) return iterator{this, e, h(key), maxsz};
        
        return end();
 }

    void swap(ADS_set &other){ //lecture
        using std::swap;
        swap(table, other.table);
        swap(currsz,other.currsz);
        swap(maxsz,other.maxsz);
        swap(lf, other.lf);
    }

    const_iterator begin() const { // could change but not necessary
        if(currsz == 0) return end();

        for (size_t i = 0; i < maxsz; i++)
                  if (table[i].head) {
                    return const_iterator(this, table[i].head, i, maxsz);
                  }
              return end();
    }

    const_iterator end() const {
        return const_iterator(nullptr);
        }                        
  
    void dump(std::ostream &o = std::cerr) const {
        o<< "max size: " << maxsz << ", current size: " << currsz << "\n";
        for (size_type i{0}; i < maxsz; ++i) {
                  if (table[i].head == nullptr) {
                      o << "[" << i << "]" << ": nullptr" << '\n';
                      continue;
                  };
                  o << "[" << i << "]" << ": ";
                  Entry* a = table[i].head;
                  while (a != nullptr) {
                      o << a->key;
                      if (a->next != nullptr)
                      o << " -> ";
                      a = a->next;
                  }
                  o << '\n';
              }
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) { //l
        if(lhs.currsz != rhs.currsz) return false;
        for(const auto& k: lhs) if(!rhs.count(k)) return false;
        return true;
    } 

    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {return !(lhs == rhs);} //l
};

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
private:
const ADS_set* tbl;
Entry* e;
size_type it_sz;
size_type it_maxsz;
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  
  explicit Iterator(const ADS_set* t = nullptr ,Entry* en = nullptr, size_type sz = 0, size_type msz = 0): tbl {t},
    e{en}, it_sz{sz}, it_maxsz{msz} {}

  reference operator*() const {return e->key;}
  pointer operator->() const {return &e->key;}

  void skip(){
        while (it_sz < it_maxsz && tbl->table[it_sz].head == nullptr)
            ++it_sz;
    }
    
    Iterator &operator++() {
        if(e == nullptr || e->next == nullptr){
            ++it_sz;
            skip();
            if(it_sz != it_maxsz){
                e = tbl->table[it_sz].head;
            } else if (it_sz == it_maxsz){
                e = nullptr;
            }
        } else {
            e = e->next;
        }
        return *this;
    }

  Iterator operator++(int) {auto rc {*this}; ++*this; return rc;} //lecture

  friend bool operator==(const Iterator &lhs, const Iterator &rhs) {return lhs.e == rhs.e;} //l
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {return !(lhs==rhs);} //l
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif /* ADS_set_first_h */
