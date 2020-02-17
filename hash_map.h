#include <functional>
#include <vector>
#include <list>
#include <utility>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    Hash hash;
    static const size_t HASH_MAP_SIZE = 1e6;
    std::list<std::pair<const KeyType, ValueType>> body;
    std::vector<typename std::list<std::pair<const KeyType, ValueType>>::iterator> iters;
    std::vector<int> sizes;

public:
    HashMap() {
        iters.resize(HASH_MAP_SIZE, body.end());
        sizes.resize(HASH_MAP_SIZE, 0);
        hash = Hash();
    }

    HashMap(Hash h) : hash(h) {
        iters.resize(HASH_MAP_SIZE, body.end());
        sizes.resize(HASH_MAP_SIZE, 0);
    }

    HashMap(HashMap& other) : hash(other.hash) {
        iters.resize(HASH_MAP_SIZE, body.end());
        sizes.resize(HASH_MAP_SIZE, 0);
        for (auto p : other)
            (*this)[p.first] = p.second;
    }

    ValueType& operator[](const KeyType k) {
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k)
                return it->second;
        sizes[hash(k)%HASH_MAP_SIZE]++;
        iters[hash(k)%HASH_MAP_SIZE] = body.insert(iters[hash(k)%HASH_MAP_SIZE], {k, ValueType()});
        return iters[hash(k)%HASH_MAP_SIZE]->second;
    }

    const ValueType& at(const KeyType k) const {
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k)
                return it->second;
        throw std::out_of_range("Out of range error");
    }

    void insert(std::pair<const KeyType, ValueType> p) {
        KeyType k = p.first;
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k)
                return;
        sizes[hash(k)%HASH_MAP_SIZE]++;
        iters[hash(k)%HASH_MAP_SIZE] = body.insert(iters[hash(k)%HASH_MAP_SIZE], p);
    }

    void erase(const KeyType k) {
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k) {
                if (t == 0)
                    iters[hash(k)%HASH_MAP_SIZE] = body.erase(iters[hash(k)%HASH_MAP_SIZE]);
                else
                    body.erase(it);
                if (--sizes[hash(k)%HASH_MAP_SIZE] == 0)
                    iters[hash(k)%HASH_MAP_SIZE] = body.end();
                return;
            }
    }

    template<typename ForwardIt>
    HashMap(ForwardIt first, ForwardIt last, Hash h = Hash()) : hash(h) {
        iters.resize(HASH_MAP_SIZE, body.end());
        sizes.resize(HASH_MAP_SIZE, 0);
        for (; first != last; first++)
            (*this)[first->first] = first->second;
    }

    HashMap(std::initializer_list<std::pair<const KeyType, ValueType>> L, Hash h = Hash()) : hash(h) {
        iters.resize(HASH_MAP_SIZE, body.end());
        sizes.resize(HASH_MAP_SIZE, 0);
        for (auto p : L)
            (*this)[p.first] = p.second;
    }

    bool empty() const {
        return body.empty();
    }

    size_t size() const {
        return body.size();
    }

    Hash hash_function() const {
        return hash;
    }

    class iterator {
    private:
        typename std::list<std::pair<const KeyType, ValueType>>::iterator it;

    public:
        iterator() { }

        iterator(typename std::list<std::pair<const KeyType, ValueType>>::iterator _it) : it(_it) { }

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

        iterator operator++() {
            return ++it;
        }

        iterator operator++(int) {
            return it++;
        }

        bool operator==(iterator other) const {
            return (it == other.it);
        }

        bool operator!=(iterator other) const {
            return !((*this) == other);
        }
    };

    class const_iterator {
    private:
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator it;

    public:
        const_iterator() { }

        const_iterator(typename std::list<std::pair<const KeyType, ValueType>>::const_iterator _it) : it(_it) { }

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

        const_iterator operator++() {
            return ++it;
        }

        const_iterator operator++(int) {
            return it++;
        }

        bool operator==(const_iterator other) const {
            return (it == other.it);
        }

        bool operator!=(const_iterator other) const {
            return !((*this) == other);
        }
    };

    iterator begin() {
        return iterator(body.begin());
    }

    iterator end() {
        return iterator(body.end());
    }

    const_iterator begin() const {
        return const_iterator(body.begin());
    }

    const_iterator end() const {
        return const_iterator(body.end());
    }

    iterator find(const KeyType k) {
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k)
                return it;
        return end();
    }

    const_iterator find(const KeyType k) const {
        auto it = iters[hash(k)%HASH_MAP_SIZE];
        for (int t = 0; t < sizes[hash(k)%HASH_MAP_SIZE]; ++t, ++it)
            if (it->first == k)
                return const_iterator(it);
        return end();
    }

    void clear() {
        std::vector<int> hashes_to_clear;
        auto p = body.begin();
        while (p != body.end()) {
            hashes_to_clear.push_back(hash(p->first));
            p = body.erase(p);
        }
        for (int h : hashes_to_clear) {
            sizes[h%HASH_MAP_SIZE] = 0;
            iters[h%HASH_MAP_SIZE] = body.end();
        }
    }

    HashMap& operator=(const HashMap& other) {
        if (&other == this)
            return *this;
        clear();
        for (auto p : other)
            (*this)[p.first] = p.second;
        return *this;
    }
};
