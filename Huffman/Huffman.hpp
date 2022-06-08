struct Node
{
    char ch;
    int freq;
    Node *left, *right;

    Node(char symbol, int frequency, Node *left_Node, Node *right_Node) : ch(symbol),
                                                                          freq(frequency),
                                                                          left(left_Node),
                                                                          right(right_Node){};
};

struct comp
{
    bool operator()(Node *left, Node *right)
    {
        return left->freq > right->freq;
    }
};