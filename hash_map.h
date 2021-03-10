/*
Copyright 2020 A.I.Arzhantsev
email: aiarzhantsev@edu.hse.ru

This hashmap uses separate chaining with linked lists. Key-value pairs are stored in one list.
For each hash value we store an iterator to the position in list with elements of this hash.
This a strategy is required for linear iteration.
We also store the number of elements with this hash value.

This hashmap uses stop-the-world technique for resizing.
*/

#include <functional>
#include <vector>
#include <list>
#include <utility>
#include <stdexcept>


template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
    using ListPairKeyVal =
        typename std::list<std::pair<const KeyType, ValueType>>;
    using ListPairKeyValIter =
        typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using ListPairKeyValConstIter =
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;

 public:
    HashMap() {
        iters_.resize(hash_map_capacity_, body_.end());
        sizes_.resize(hash_map_capacity_, 0);
        hash = Hash();
    }

    explicit HashMap(Hash h) : hash(h) {
        iters_.resize(hash_map_capacity_, body_.end());
        sizes_.resize(hash_map_capacity_, 0);
    }

    HashMap(const HashMap& other) : hash(other.hash) {
        iters_.resize(hash_map_capacity_, body_.end());
        sizes_.resize(hash_map_capacity_, 0);
        for (auto p : other) {
            (*this)[p.first] = p.second;
        }
    }

    // Returns the element with given key value.
    // If this key value is not in the map, creates a new element.
    ValueType& operator[](const KeyType k) {
        enlarge_hashtable_if_needed();
        size_t ind = hash(k) % hash_map_capacity_;
        auto it = iters_[ind];
        for (int t = 0; t < sizes_[ind]; ++t, ++it) {
            if (it->first == k) {
                return it->second;
            }
        }
        sizes_[ind]++;
        hash_map_size_++;
        iters_[ind] = body_.insert(iters_[ind], {k, ValueType()});
        return iters_[ind]->second;
    }

    // Const version of operator[].
    // If key value is not in the map, throws std::out_of_range error.
    const ValueType& at(const KeyType k) const {
        auto it = iters_[hash(k) % hash_map_capacity_];
        for (int t = 0; t < sizes_[hash(k) % hash_map_capacity_]; ++t, ++it) {
            if (it->first == k) {
                return it->second;
            }
        }
        throw std::out_of_range("Out of range error");
    }

    // Inserts an element.
    // If element with this key value already exists, does nothing.
    void insert(std::pair<const KeyType, ValueType> p) {
        enlarge_hashtable_if_needed();
        KeyType k = p.first;
        size_t ind = hash(k) % hash_map_capacity_;
        auto it = iters_[ind];
        for (int t = 0; t < sizes_[ind]; ++t, ++it) {
            if (it->first == k) {
                return;
            }
        }
        sizes_[ind]++;
        hash_map_size_++;
        iters_[ind] = body_.insert(iters_[ind], p);
    }

    // Erases an element by its kay value.
    // If this key value is not in the map, does nothing.
    void erase(const KeyType k) {
        enlarge_hashtable_if_needed();
        size_t ind = hash(k) % hash_map_capacity_;
        auto it = iters_[ind];
        for (int t = 0; t < sizes_[ind]; ++t, ++it) {
            if (it->first == k) {
                if (t == 0) {
                    iters_[ind] = body_.erase(iters_[ind]);
                } else {
                    body_.erase(it);
                }
                if (--sizes_[ind] == 0) {
                    iters_[ind] = body_.end();
                }
                hash_map_size_--;
                return;
            }
        }
    }

    template<typename ForwardIt>
    HashMap(ForwardIt first, ForwardIt last, Hash h = Hash()) : hash(h) {
        iters_.resize(hash_map_capacity_, body_.end());
        sizes_.resize(hash_map_capacity_, 0);
        for (; first != last; first++) {
            (*this)[first->first] = first->second;
        }
    }

    HashMap(std::initializer_list<std::pair<const KeyType, ValueType>> L,
                                                    Hash h = Hash()) : hash(h) {
        iters_.resize(hash_map_capacity_, body_.end());
        sizes_.resize(hash_map_capacity_, 0);
        for (auto p : L) {
            (*this)[p.first] = p.second;
        }
    }

    // Returns true if hashmap is empty.
    bool empty() const {
        return body_.empty();
    }

    // Returns the number of elements in hashmap.
    size_t size() const {
        return hash_map_size_;
    }

    // Returns the hash function.
    Hash hash_function() const {
        return hash;
    }


    // Hashmap iterator type.
    // Based on std::list::iterator.
    class iterator {
    private:
        ListPairKeyValIter it;

    public:
        iterator() { }

        explicit iterator(ListPairKeyValIter _it) : it(_it) { }

        iterator& operator=(const iterator& other) {
            it = other.it;
            return *this;
        }

        std::pair<const KeyType, ValueType>& operator*() {
            return *it;
        }

        std::pair<const KeyType, ValueType>* operator->() {
            return &(*it);
        }

        // Requires constant time.
        iterator operator++() {
            return iterator(++it);
        }

        // Requires constant time.
        iterator operator++(int) {
            return iterator(it++);
        }

        bool operator==(iterator other) const {
            return (it == other.it);
        }

        bool operator!=(iterator other) const {
            return !((*this) == other);
        }
    };

    // Const version of iterator.
    class const_iterator {
    private:
        ListPairKeyValConstIter it;

    public:
        const_iterator() { }

        explicit const_iterator(ListPairKeyValConstIter _it) : it(_it) { }

        const_iterator& operator=(const const_iterator& other) {
            it = other.it;
            return *this;
        }

        const std::pair<const KeyType, ValueType>& operator*() {
            return *it;
        }

        const std::pair<const KeyType, ValueType>* operator->() {
            return &(*it);
        }

        // Requires constant time.
        const_iterator operator++() {
            return const_iterator(++it);
        }

        // Requires constant time.
        const_iterator operator++(int) {
            return const_iterator(it++);
        }

        bool operator==(const_iterator other) const {
            return (it == other.it);
        }

        bool operator!=(const_iterator other) const {
            return !((*this) == other);
        }
    };

    // Returns iterator to the first element in hashmap.
    iterator begin() {
        return iterator(body_.begin());
    }

    // Returns iterator to the position after the last element.
    iterator end() {
        return iterator(body_.end());
    }

    // Const version of begin().
    const_iterator begin() const {
        return const_iterator(body_.begin());
    }

    // Const version of end().
    const_iterator end() const {
        return const_iterator(body_.end());
    }

    // Finds an element with given key value. Returns iterator to the element.
    // If this key value is not in the map, returns end iterator.
    iterator find(const KeyType k) {
        auto it = iters_[hash(k) % hash_map_capacity_];
        for (int t = 0; t < sizes_[hash(k) % hash_map_capacity_]; ++t, ++it) {
            if (it->first == k) {
                return iterator(it);
            }
        }
        return end();
    }

    // Const version of find. Returns const iterator.
    const_iterator find(const KeyType k) const {
        auto it = iters_[hash(k) % hash_map_capacity_];
        for (int t = 0; t < sizes_[hash(k) % hash_map_capacity_]; ++t, ++it) {
            if (it->first == k) {
                return const_iterator(it);
            }
        }
        return end();
    }

    // Clears the hash map. Works linear from the number of elements in the map.
    void clear() {
        std::vector<int> hashes_to_clear;
        auto p = body_.begin();
        while (p != body_.end()) {
            hashes_to_clear.push_back(hash(p->first));
            p = body_.erase(p);
        }
        for (int h : hashes_to_clear) {
            sizes_[h % hash_map_capacity_] = 0;
            iters_[h % hash_map_capacity_] = body_.end();
        }
        hash_map_size_ = 0;
    }

    HashMap& operator=(const HashMap& other) {
        if (&other == this) {
            return *this;
        }
        clear();
        for (auto p : other) {
            (*this)[p.first] = p.second;
        }
        return *this;
    }

    // Resizes the hash map. Is used to avoid collisions.
    // If the number of elements in the map is above the number
    // of different hash values, maximum hash value is multipied
    // by RESIZE_FACTOR constant.
    // After that all element are reinserted into the map.
    // Works linear in the number of elements.
    void enlarge_hashtable_if_needed() {
        if (hash_map_size_ < hash_map_capacity_) {
            return;
        }

        auto old_body_ = body_;
        hash_map_capacity_ *= RESIZE_FACTOR;
        body_.clear();
        iters_.assign(hash_map_capacity_, body_.end());
        sizes_.assign(hash_map_capacity_, 0);
        hash_map_size_ = 0;
        for (auto p : old_body_)
            (*this)[p.first] = p.second;
    }

 private:
    const int RESIZE_FACTOR = 2;
    Hash hash;
    size_t hash_map_capacity_ = 1;
    size_t hash_map_size_ = 0;
    std::list<std::pair<const KeyType, ValueType>> body_;
    std::vector<ListPairKeyValIter> iters_;
    std::vector<int> sizes_;
};
