#ifndef ORDERED_SET_HPP
#define ORDERED_SET_HPP

#include <vector>
#include <unordered_set>
#include <iostream> // For print

template <typename T>
class OrderedSet
{
private:
    std::vector<T> order;            // To keep insertion order
    std::unordered_set<T> uniqueSet; // To track uniqueness

public:
    void insert(const T &value);      // Insert value if it's not already in the set
    void clear();                     // Clear the OrderedSet
    size_t size() const;              // Get the size of the OrderedSet

    typename std::vector<T>::iterator begin();             // Iterator support
    typename std::vector<T>::iterator end();
    typename std::vector<T>::const_iterator begin() const; // Const Iterator support
    typename std::vector<T>::const_iterator end() const;

    void print() const; // Print elements
};

// ---------- Implementation ----------

// Insert method
template<typename T>
void OrderedSet<T>::insert(const T& value)
{
    if (uniqueSet.find(value) == uniqueSet.end()) {
        order.push_back(value);
        uniqueSet.insert(value);
    }
}

// Clear method
template<typename T>
void OrderedSet<T>::clear()
{
    order.clear();
    uniqueSet.clear();
}

// Size method
template<typename T>
size_t OrderedSet<T>::size() const
{
    return order.size();
}

// Iterators
template<typename T>
typename std::vector<T>::iterator OrderedSet<T>::begin()
{
    return order.begin();
}

template<typename T>
typename std::vector<T>::iterator OrderedSet<T>::end()
{
    return order.end();
}

template<typename T>
typename std::vector<T>::const_iterator OrderedSet<T>::begin() const
{
    return order.begin();
}

template<typename T>
typename std::vector<T>::const_iterator OrderedSet<T>::end() const
{
    return order.end();
}

// Print method
template<typename T>
void OrderedSet<T>::print() const
{
    for (const auto& elem : order) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

#endif // ORDERED_SET_HPP
