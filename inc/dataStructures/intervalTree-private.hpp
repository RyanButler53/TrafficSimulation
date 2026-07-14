#include "intervalTree.hpp"
#include <stdexcept>

template <Interval I>
IntervalTree<I>::IntervalTree():
root_{new Node(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())}, size_{0}
{
}

template <Interval I>
IntervalTree<I>::~IntervalTree()
{
    destroyTree(root_);
}

template <Interval I>
void IntervalTree<I>::destroyTree(Node* n){
    if (n){
        if (n->left_){
            destroyTree(n->left_);
        }
        if (n->right_){
            destroyTree(n->right_);
        }
        delete n;
    }
}

template <Interval I>
void IntervalTree<I>::clear(){
    destroyTree(root_);
    root_ = new Node(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    allEndpoints_.clear();
    allIntervals_.clear();
    size_ = 0;
}

template <Interval I>
void IntervalTree<I>::insert(const T &low, const T &high){
    insert(I{low, high});
}

template <Interval I>
void IntervalTree<I>::insert(const std::pair<T,T>& i){
    insert(I{i.first, i.second});
}

template <Interval I>
void IntervalTree<I>::insert(const I& i){
// First check if both endpoints are in the tree before adding them
    const T& low = i.low();
    const T& high = i.high();
    if (!allEndpoints_.contains(low)){
        insertNode(low, root_);
        allEndpoints_.insert(low);
    }
    if (!allEndpoints_.contains(high)){
        insertNode(high, root_);
        allEndpoints_.insert(high);
    }
    allIntervals_.push_back(i);
    // Insert the INDEX of the interval into each node
    insertInterval(size_, root_);
    ++size_;
    return;
}

template <Interval I>
void IntervalTree<I>::insertNode(const T& value, Node*&node){
    if (node->isLeaf_){
        node->isLeaf_ = false;
        node->value_ = value;
        node->left_ = new Node(node->min_, value);
        node->right_ = new Node(value, node->max_);
    } else if (value > node->value_){
        insertNode(value, node->right_);
    } else {
        insertNode(value, node->left_);
    }
}

template <Interval I>
void IntervalTree<I>::insertInterval(size_t intervalInd, Node*&node){
    const I &i = allIntervals_[intervalInd];
    // Case where the node's subtree is fully contained in the interval
    if (node->min_ >= i.low() and node->max_ <= i.high())
    {
        node->intervals_.push_back(intervalInd);
    }
    else
    {
        // Going to be using node->value_
        // assert(!node->isLeaf_);
        if (node->isLeaf_){
            throw std::logic_error("Node cannot be a leaf here");
        }
        if (i.low() < node->value_){
            insertInterval(intervalInd, node->left_);
        } 
        if (node->value_ < i.high()){
            insertInterval(intervalInd, node->right_);
        }
    }
}

template <Interval I>
void IntervalTree<I>::query(const I& interval,std::unordered_set<size_t>& intervals, Node* n) const{
    intervals.insert(n->intervals_.begin(), n->intervals_.end());
    if (n->isLeaf_) {
        return;
    }
    // If query upper bound is less than current node value, intervals will be in left subtree
    if (interval.high() < n->value_){
        query(interval, intervals, n->left_);
    }
    // If query lower bound is greater than node value, intervals will be in right subtree
    if (interval.low() > n->value_){
        query(interval, intervals, n->right_);
    }

    // If node value is INSIDE the interval or on the endpoints, recurse on both sides
    if (interval.low() <= n->value_ and interval.high() >= n->value_){
        query(interval, intervals, n->left_);
        query(interval, intervals, n->right_);
    }
}

template <Interval I>
std::vector<I> IntervalTree<I>::findOverlaps(const T& queryPoint){
    std::unordered_set<size_t> overlapInds;
    query({queryPoint, queryPoint}, overlapInds, root_);

    std::vector<I> overlapIntervals;
    for (size_t intervalInd : overlapInds)
    {
        overlapIntervals.push_back(allIntervals_[intervalInd]);
    }
    return overlapIntervals;
}

template <Interval I>
std::vector<I> IntervalTree<I>::findOverlaps(const std::pair<T,T>& interval){
    return findOverlaps(I(interval.first, interval.second));
}

template <Interval I>
std::vector<I> IntervalTree<I>::findOverlaps(const I& interval){
    std::unordered_set<size_t> overlapInds;
    query(interval, overlapInds,root_);
    std::vector<I> overlapIntervals;
    for (size_t intervalInd : overlapInds)
    {
        overlapIntervals.push_back(allIntervals_[intervalInd]);
    }
    return overlapIntervals;
}

template <Interval I>
std::vector<I> IntervalTree<I>::findSupersets(const T &low, const T &high){
    std::unordered_set<size_t> overlapLow;
    std::unordered_set<size_t> overlapHigh;
    query({low, low}, overlapLow, root_);
    query({high, high}, overlapHigh, root_);
    std::vector<I> overlapIntervals;
    // Find Intersection of sets
    for (size_t intervalInd : overlapHigh){
        // Both contain this interval
        if (overlapLow.contains(intervalInd)){
            overlapIntervals.push_back(allIntervals_[intervalInd]);
        }
    }
    return overlapIntervals;
}

// Struct Functions

template <typename T>
SimpleInterval<T>::SimpleInterval(T low, T high):
    low_{low}, high_{high}{
        // Nothing here
}

template <typename T>
bool SimpleInterval<T>::operator==(const SimpleInterval<T>& other) const {
    return (low_ == other.low_) && (high_ == other.high_);
}

template <Interval I>
IntervalTree<I>::Node::Node(T min, T max):
    left_{nullptr}, 
    right_{nullptr}, 
    min_{min},
    max_{max}, 
    isLeaf_{true}{
        // Nothing to initialize
}