/**
 * @file intervalTree.hpp
 * @author Ryan Butler (rbutler@g.hmc.edu)
 * @brief Implementation of non-balancing interval tree
 * @version 0.1
 * @date 2023-07-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef INTERVAL_TREE_HPP_INCLUDED
#define INTERVAL_TREE_HPP_INCLUDED

#include <vector>
#include <tuple>
#include <unordered_set>
#include <numeric>


template <typename T>
class IntervalTree {
  public:
      struct Interval;

  private:
      // Interval Tree Node Struct
      struct Node
      {
          std::vector<size_t> intervals_; // Vector indexes of stored intervals
          Node *left_;
          Node *right_;
          T value_;
          T min_;       // Minimum value in subtree
          T max_;       // Maximum value in subtree
          bool isLeaf_; // Accessing value_ in leaf nodes is underfined

          // Constructors
          Node(T min, T max);
    };

    // Tree Data
    std::unordered_set<T> allEndpoints_; // Check if endpoint already exists
    std::vector<Interval> allIntervals_; // Store all intervals here
    Node *root_;
    size_t size_;

    // Helper Functions:

    /**
     * @brief Helper function for destructor
     * 
     * @param node Node to destroy
     */
    void destroyTree(Node *node);

    /**
     * @brief Inserts a new node into the tree representing an endpoint
     * 
     * @param value Value to put in the node
     * @param node Current node 
     */
    void insertNode(const T& value, Node *&node);

    /**
     * @brief Inserts an interval into the tree
     * 
     * @param intervalInd Index of this interval in allIntervals_ vector
     * @param node Current node
     */
    void insertInterval(size_t intervalInd, Node*& node);

    /**
     * @brief Helper function for finding overlapping intervals
     * 
     * @param queryPoint Point to find overlaps for
     * @param intervals Set of interval indexes. Modified over time
     * @param n Current node
     */
    void query(const T &queryPoint, std::unordered_set<size_t>& intervals, Node* n) const;

public:

    // Constructors
    IntervalTree();
    ~IntervalTree();

    // Interface Functions:

    /**
     * @brief Inserts an interval into the interval tree
     * 
     * @param low Lower bound of interval
     * @param high Upper bound of interval
     */
    void insert(const T &low, const T &high);

    /**
     * @brief Overloaded insert function
     * 
     * @param i IntervalTree::Interval object to insert
     */
    void insert(const Interval &i);

    /**
     * @brief Finds all intervals that overlap with a given query point
     * 
     * @param queryPoint Value that overlaps with every interval returned
     * @return std::vector<std::tuple<T, T>> Vector with low and high bounds
     * of all intervals that contain queryPoint param
     */
    std::vector<std::pair<T, T>> findOverlaps(const T &queryPoint);

    /**
     * @brief Finds all intervals that fully contain the low and high bound
     * 
     * @param low lower bound of interval
     * @param high upper bound 
     * @return std::vector<std::tuple<T, T>> Vector of all intervals that
     * fully contain the interval with lower bound low and upper bound high
     */
    std::vector<std::pair<T, T>> findSupersets(const T &low, const T &high);

    // Public Interval Struct

    /**
     * @brief Mostly for bookkeeping in implementation, but is public and can 
     * be constructed and inserted into the interval tree
     * 
     */
    struct Interval
    {
        // Data
        T low_;
        T high_;
    
        // Constructors / Destructor
        Interval(T low, T high);
        Interval(const Interval &other) = default;
        ~Interval() = default;
    };
};

#include "intervalTree-private.hpp"

#endif // INTERVAL_TREE_HPP_INCLUDED