/** @file compressor.hpp */

#include <memory>

/**
 * @brief Node structure
 *
 */
struct Node
{
    char ch;                           /** Char of the node */
    int freq;                          /** Summary frquency of the node */
    std::shared_ptr<Node> left, right; /** Shared pointers to the left and right nodes */

    /**
     * @brief Construct a new Node object
     *
     * @param symbol        Char of the node
     * @param frequency     Summary frquency of the node
     * @param leftNode      Shared pointers to the left node
     * @param rightNode     Shared pointers to the right node
     */
    Node(char symbol, int frequency, std::shared_ptr<Node> leftNode, std::shared_ptr<Node> rightNode) : ch(symbol),
                                                                                                        freq(frequency),
                                                                                                        left(std::move(leftNode)),
                                                                                                        right(std::move(rightNode)){};

    Node(Node &&) = default;

    Node(const Node &other) = default;

    Node &operator=(const Node &other) = default;
};

/**
 * @brief Comparison structure
 *
 */
struct comp
{
    /**
     * @brief Comparison operator for priority queue
     *
     */
    bool operator()(Node &left, Node &right)
    {
        return left.freq > right.freq;
    }
};