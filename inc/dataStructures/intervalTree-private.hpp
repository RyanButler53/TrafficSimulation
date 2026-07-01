#include "intervalTree.hpp"
using namespace std;

template <typename T>
IntervalTree<T>::IntervalTree():
root_{new Node(numeric_limits<T>::min(), numeric_limits<T>::max())}, size_{0}
{
}

template <typename T>
IntervalTree<T>::~IntervalTree()
{
    destroyTree(root_);
}

template <typename T>
void IntervalTree<T>::destroyTree(Node* n){
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

template <typename T>
void IntervalTree<T>::insert(const T &low, const T &high){
    // First check if both endpoints are in the tree before adding them
    if (!allEndpoints_.contains(low)){
        insertNode(low, root_);
        allEndpoints_.insert(low);
    }
    if (!allEndpoints_.contains(high)){
        insertNode(high, root_);
        allEndpoints_.insert(high);
    }
    allIntervals_.push_back(Interval{low, high});
    // Insert the INDEX of the interval into each node
    insertInterval(size_, root_);
    ++size_;
    return;
}

template <typename T>
void IntervalTree<T>::insert(const Interval& i){
    insert(i.low_, i.high_);
    return;
}

template <typename T>
void IntervalTree<T>::insertNode(const T& value, Node*&node){
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

template <typename T>
void IntervalTree<T>::insertInterval(size_t intervalInd, Node*&node){
    const Interval &i = allIntervals_[intervalInd];
    // Case where the node's subtree is fully contained in the interval
    if (node->min_ >= i.low_ and node->max_ <= i.high_)
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
        if (i.low_ < node->value_){
            insertInterval(intervalInd, node->left_);
        } 
        if (node->value_ < i.high_){
            insertInterval(intervalInd, node->right_);
        }
    }
}

template <typename T>
void IntervalTree<T>::query(const T& queryPoint,unordered_set<size_t>& intervals, Node* n) const{
    intervals.insert(n->intervals_.begin(), n->intervals_.end());
    if (n->isLeaf_) {
        return;
    }  
    // if queryPoint == n->value_, recurse on both sides.
    if (queryPoint <= n->value_) {
        query(queryPoint, intervals, n->left_);
    } 
    if (queryPoint >= n->value_) {
        query(queryPoint, intervals, n->right_);
    } 
}

template <typename T>
vector<pair<T, T>> IntervalTree<T>::findOverlaps(const T& queryPoint){
    unordered_set<size_t> overlapInds;
    query(queryPoint, overlapInds,root_);
    vector<pair<T, T>> overlapIntervals;
    for (size_t intervalInd : overlapInds)
    {
        const Interval &i = allIntervals_[intervalInd];
        overlapIntervals.push_back({i.low_, i.high_});
    }
    return overlapIntervals;
}

template <typename T>
vector<pair<T, T>> IntervalTree<T>::findSupersets(const T &low, const T &high){
    unordered_set<size_t> overlapLow;
    unordered_set<size_t> overlapHigh;
    query(low, overlapLow,root_);
    query(high, overlapHigh,root_);
    vector<pair<T, T>> overlapIntervals;
    // Find Intersection of sets
    for (size_t intervalInd : overlapHigh){
        // Both contain this interval
        if (overlapLow.contains(intervalInd)){
            const Interval &i = allIntervals_[intervalInd];
            overlapIntervals.push_back({i.low_, i.high_});
        }
    }
    return overlapIntervals;
}

// Struct Functions

template <typename T>
IntervalTree<T>::Interval::Interval(T low, T high):
    low_{low}, high_{high}{
        // Nothing here
}

template<typename T>
IntervalTree<T>::Node::Node(T min, T max):
    left_{nullptr}, 
    right_{nullptr}, 
    min_{min},
    max_{max}, 
    isLeaf_{true}{
        // Nothing to initialize
}