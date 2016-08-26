#ifndef _CACHE_H_
#define _CACHE_H_

#include <iostream>
#include <map>
#include <vector>
#include <cassert>
extern int me, nprocs;


namespace DLTC{
using namespace std;

struct Entry {
  Entry* prev;
  Entry* next;
  pair<int, int> key; // <tensorID, tileIndex>
  double* val; // data pointer
  long long size;
  Entry(pair<int, int> k, double* v, long long s) : key(k), val(v), size(s), prev(NULL), next(NULL) {};
};

class LRUCache {
private:
  long long size_; // in bytes
  long long used_; // in bytes
  typedef map<pair<int, int>, Entry*> EntryMap;
  EntryMap m;
  Entry* head_;
  Entry* tail_;

  void moveToFront(Entry* e) {
    e->next = head_->next;
    head_->next = e;
    e->prev = head_;
    e->next->prev = e;
  }

public:
  inline void init(long long sizeInGB) {
    used_ = 0;
    size_ = sizeInGB * 1024 * 1024 * 1024;
    head_ = new Entry(make_pair(-1,-1), 0, 0);
    tail_ = new Entry(make_pair(-1,-1), 0, 0);
    head_->next = tail_;
    tail_->prev = head_;
    if (!me) cout << "cache init" << endl;
  }
  inline void close() {
    for (EntryMap::iterator it = m.begin(); it != m.end(); it++) {
      Entry* e = it->second;
      delete [] e->val;
      delete e;
    } // deallocate data
    if (!me) cout << "cache close" << endl;
  }
  inline long long size() {
    return size_;
  }
  inline long long used() {
    return used_;
  }
  inline double sizeInGB() {
    return double(size_) / 1024 / 1024 / 1024;
  }
  inline double usedInGB() {
    return double(used_) / 1024 / 1024 / 1024;
  }
  inline bool add(int tid, int tileIndex, double* data, int dsize) {
#if 0
    int bytes = dsize * 8;
    if (used_ + bytes > size_) return false;
    pair<int, int> key = make_pair(tid, tileIndex);
    if (m.find(key) == m.end()) {
      m[key] = new Entry(key, data);
      used_ += bytes;
      return true;
    }
    else {
      cout << "assert add" << endl;
      assert(1);
      return false;
    }
#endif
    if (size_ == 0) return false;
    pair<int, int> key = make_pair(tid, tileIndex);
    int bytes = dsize * 8;
    if (m.find(key) != m.end()) {
      Entry* e = m[key];
      e->val = data;
      e->size = bytes;
      e->prev->next = e->next;
      e->next->prev = e->prev;
      moveToFront(e);
    }
    else {
      Entry* e = new Entry(key, data, bytes);
      used_ += bytes;
      moveToFront(e);
      m[key] = e;
      if (used_ > size_) { // discard LRU
        Entry* last = tail_->prev;
        tail_->prev = last->prev;
        last->prev->next = tail_;
        m.erase(m.find(last->key));
        used_ -= last->size;
        delete last->val;
        delete last;
      }
    }
    return true;
  }
  inline double* get(int tid, int tileIndex) {
    pair<int, int> key = make_pair(tid, tileIndex);
    if (m.find(key) != m.end()) {
      Entry* e = m[key];
      e->prev->next = e->next;
      e->next->prev = e->prev;
      moveToFront(e);
      return e->val;
    }
    else {
      cout << "assert get" << endl;
      assert(1);
      return NULL;
    }
  }
  inline bool contains(int tid, int tileIndex) {
    if (size_ == 0) return false;
    pair<int, int> key = make_pair(tid, tileIndex);
    return m.find(key) != m.end();
  }
};
}
#endif
