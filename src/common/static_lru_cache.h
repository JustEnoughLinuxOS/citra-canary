// Modified version of: https://www.boost.org/doc/libs/1_79_0/boost/compute/detail/lru_cache.hpp
// Original license:
//
//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//
#pragma once

#include <array>
#include <list>
#include <tuple>
#include <utility>

#include <boost/optional.hpp>

namespace Common {

// a cache which evicts the least recently used item when it is full
// the cache elements are statically allocated.
template <class Key, class Value, size_t Size>
class static_lru_cache {
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef std::list<std::pair<Key, size_t>> list_type;
    typedef std::array<Value, Size> array_type;

    static_lru_cache() {}

    ~static_lru_cache() {}

    size_t size() const {
        return m_list.size();
    }

    constexpr size_t capacity() const {
        return m_array.size();
    }

    bool empty() const {
        return m_list.empty();
    }

    bool contains(const key_type& key) const {
        return find(key) != m_list.end();
    }

    // Requests an element from the cache. If it is not found,
    // the element is inserted using its key.
    // Returns wether the element was present in the cache
    // and a reference to the element itself.
    std::pair<bool, value_type&> request(const key_type& key) {
        // lookup value in the cache
        typename list_type::const_iterator i = find(key);
        if (i == m_list.cend()) {
            size_t next_index = size();
            // insert item into the cache, but first check if it is full
            if (next_index >= capacity()) {
                // cache is full, evict the least recently used item
                next_index = evict();
            }

            // insert the new item
            m_list.push_front(std::make_pair(key, next_index));
            return std::pair<bool, value_type&>(false, m_array[next_index]);
        }
        // return the value, but first update its place in the most
        // recently used list
        if (i != m_list.cbegin()) {
            // move item to the front of the most recently used list
            typename list_type::iterator::value_type backup = *i;
            m_list.erase(i);
            m_list.push_front(backup);

            // return the value
            return std::pair<bool, value_type&>(true, m_array[backup.second]);
        } else {
            // the item is already at the front of the most recently
            // used list so just return it
            return std::pair<bool, value_type&>(true, m_array[i->second]);
        }
    }

    void clear() {
        m_list.clear();
    }

private:
    typename list_type::const_iterator find(const key_type& key) const {
        return std::find_if(m_list.cbegin(), m_list.cend(),
                            [&key](const std::pair<Key, size_t>& el) { return el.first == key; });
    }

    size_t evict() {
        // evict item from the end of most recently used list
        typename list_type::iterator i = --m_list.end();
        size_t evicted_index = i->second;
        m_list.erase(i);
        return evicted_index;
    }

private:
    array_type m_array;
    list_type m_list;
};

} // namespace Common