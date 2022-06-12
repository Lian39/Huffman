/** @file compressor.cpp */

#include "huffman.hpp"
#include <streambuf>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <map>

/**
 * @brief Read file to encode
 * 
 * @param path file path to read
 * @return std::string 
 */
std::string readFile(const std::string &path)
{
    std::ifstream inputFile(path, std::ios::binary); /** file to read */
    
    std::string str((std::istreambuf_iterator<char>(inputFile)),
                    std::istreambuf_iterator<char>()); /** read data */
    
    return str;
}

/**
 * @brief Create a Frequencies Table object
 * 
 * @param data data on which the table is built
 * @return std::multimap<int, char> 
 */
std::multimap<int, char> createFreqsTable(const std::string &data)
{
    std::vector<int> freqs(0x100); /** vector of size 256 */
    std::multimap<int, char> freqsTable; /** char frequency table */

    for (auto ch : data)
    {
        freqs[static_cast<unsigned char>(ch)]++;
    }

    for (size_t i = 0; i < freqs.size(); ++i)
    {
        if (freqs[i] > 0)
            freqsTable.insert(std::make_pair(freqs[i], (unsigned char)i));
    }

    return freqsTable;
}

/**
 * @brief Build a tree
 * 
 * @param freqsTable char frequency table
 * @return Node 
 */
Node buildTree(const std::multimap<int, char> &freqsTable)
{
    std::priority_queue<Node, std::vector<Node>, comp> pq; /** priority queue with nodes */

    for (auto pair : freqsTable)
    {
        Node node = Node(pair.second, pair.first, nullptr, nullptr);
        pq.push(node);
    }

    while (pq.size() != 1)
    {
        Node left = pq.top();
        pq.pop();
        Node right = pq.top();
        pq.pop();

        int freqsSum = left.freq + right.freq;
        Node node = Node('\0', freqsSum, std::make_shared<Node>(left), std::make_shared<Node>(right));
        pq.push(node);
    }

    return pq.top();
}

/**
 * @brief Create a Codes Table object
 * 
 * @param root root node of the tree
 * @param code initial code (pass "0")
 * @param codesTable std::multimap<char, std::string>
 */
void createCodesTable(std::shared_ptr<Node> root, std::string code, std::multimap<char, std::string> &codesTable)
{
    if (root == nullptr)
        return;

    if (!root->left && !root->right)
        codesTable.insert(std::make_pair(root->ch, code));

    createCodesTable(root->left, code + "0", codesTable);
    createCodesTable(root->right, code + "1", codesTable);
}

/**
 * @brief Encode data
 * 
 * @param data data to encode
 * @param codesTable char codes table
 * @return std::vector<bool> 
 */
std::vector<bool> encode(const std::string &data, const std::multimap<char, std::string> &codesTable)
{
    std::vector<bool> encodedData; /** encoded data */

    for (auto ch : data)
    {
        for (auto bit : codesTable.find(ch)->second)
        {
            encodedData.push_back(bit == '0' ? 0 : 1);
        }
    }

    return encodedData;
}

/**
 * @brief Decode data
 * 
 * @param root root node of the tree
 * @param encodedData encoded data
 * @return std::string 
 */
std::string decode(std::shared_ptr<Node> root, const std::vector<bool> &encodedData)
{
    std::string decodedData; /** decoded data */
    std::shared_ptr<Node> node = root; /** root node of the tree */

    for (auto i : encodedData)
    {
        if (i)
            node = node->right;
        else
            node = node->left;
        if (node->left == nullptr && node->right == nullptr)
        {
            char ch = node->ch;
            decodedData += ch;
            node = root;
        }
    }

    return decodedData;
}

/**
 * @brief Serialize the tree into file
 * 
 * @param root root node of the tree
 * @param outFile output file
 */
void serializeTree(std::shared_ptr<Node> root, std::ofstream &outFile)
{
    if (root == nullptr)
        return;

    if (root->left == nullptr && root->right == nullptr)
    {
        char flag = '1';
        char ch = root->ch;
        outFile.write((char *)&flag, sizeof(flag));
        outFile.write((char *)&ch, sizeof(ch));
    }
    else
    {
        char flag = '0';
        outFile.write((char *)&flag, sizeof(flag));
    }

    serializeTree(root->left, outFile);
    serializeTree(root->right, outFile);
}

/**
 * @brief Deserialize tree from file
 * 
 * @param root root node of the tree
 * @param inputFile input file
 */
void deserializeTree(std::shared_ptr<Node> root, std::ifstream &inputFile)
{
    char ch; /** an empty char to read into it from file */

    if (!inputFile.read((char *)&ch, sizeof(ch)) || root == nullptr)
        return;

    if (ch == '0')
    {
        std::shared_ptr<Node> leftNode = std::make_shared<Node>('\0', 0, nullptr, nullptr);
        std::shared_ptr<Node> rightNode = std::make_shared<Node>('\0', 0, nullptr, nullptr);

        root->left = leftNode;
        root->right = rightNode;

        deserializeTree(leftNode, inputFile);
        deserializeTree(rightNode, inputFile);
    }
    else
    {
        inputFile.read((char *)&ch, sizeof(ch));
        root->ch = ch;
        return;
    }
}

/**
 * @brief Read file to decode
 * 
 * @param inputPath file path to read
 * @param root root node of the tree
 * @return std::vector<bool> 
 */
std::vector<bool> readFile(const std::string &inputPath, std::shared_ptr<Node> root)
{
    std::vector<bool> encodedData; /** encoded data */
    std::ifstream inputFile(inputPath, std::ios::binary); /** input file */

    deserializeTree(root, inputFile);

    char ch; /** an empty char to read into it from a file */

    while (inputFile.read(&ch, sizeof(ch)))
    {
        for (int i = 0; i < CHAR_BIT; i++)
        {
            encodedData.push_back(((ch >> i) & 1) != 0);
        }
    }
    
    return encodedData;
}

/**
 * @brief Write encoded data to file
 * 
 * @param outPath file path to write encoded data
 * @param encodedData encoded data
 * @param root root node of the tree
 */
void writeFile(const std::string &outPath, const std::vector<bool> &encodedData, std::shared_ptr<Node> root)
{
    std::ofstream outFile = std::ofstream(outPath, std::ios::binary); /** output file */

    serializeTree(root, outFile);

    for (size_t i = 0; i <= encodedData.size() / CHAR_BIT; ++i)
    {
        unsigned char ch = 0;

        for (int j = 0; j < CHAR_BIT; ++j)
        {
            if (encodedData[i * CHAR_BIT + j])
                ch |= (1 << j);
        }
        outFile.write((char *)&ch, sizeof(ch));
    }
}

/**
 * @brief Write decoded data to file
 * 
 * @param outPath file path to write decoded data
 * @param decodedData decoded data
 */
void writeFile(const std::string &outPath, const std::string &decodedData)
{
    std::ofstream outFile(outPath, std::ios::binary); /** output file */

    for (auto i : decodedData)
    {
        outFile.write((char *)&i, sizeof(i));
    }
}

/**
 * @brief Compress the file
 * 
 * @param inputPath file path to compress
 * @param outPath file path to write compressed data
 */
void compress(const std::string &inputPath, const std::string &outPath)
{
    std::string data = readFile(inputPath); /** data read from file */
    std::multimap<int, char> freqsTable = createFreqsTable(data); /** char frequency table */
    Node rootNode = buildTree(freqsTable); /** root node of the tree */
    std::shared_ptr<Node> rootPtr = std::make_shared<Node>(rootNode); /** shared pointer to the root node */
    std::multimap<char, std::string> codesTable; /** char codes table */
    createCodesTable(rootPtr, "", codesTable);
    std::vector<bool> encodedData = encode(data, codesTable); /** encoded data */
    writeFile(outPath, encodedData, rootPtr);
}

/**
 * @brief Decompress
 * 
 * @param inputPath file path to decompress
 * @param outPath file path to write decompressed data
 */
void decompress(const std::string &inputPath, const std::string &outPath)
{
    std::shared_ptr<Node> root = std::make_shared<Node>('\0', 0, nullptr, nullptr); /** empty shared pointer to the root node */

    std::vector<bool> encodedData = readFile(inputPath, root); /** encoded data */
    std::string decodedData = decode(root, encodedData); /** decoded data */
    writeFile(outPath, decodedData);
}

int main(int argv, char *argc[])
{
    if (argv <= 1)
        return 0;

    bool inputFlag = false, outFlag = false, helpFlag = false;
    std::string inputPath = "";
    std::string outPath = "";
    std::string type;

    for (int i = 1; i < argv; i++)
    {
        if (strcmp(argc[i], "--help") == 0 || strcmp(argc[i], "-h") == 0)
        {
            helpFlag = true;
            std::cout << "options:"
                      << "\n"
                      << "-h, --help"
                      << "\t\t\t"
                      << "show this help message and exit"
                      << "\n"
                      << "-c, --compress"
                      << "\t\t\t"
                      << "compress file"
                      << "\n"
                      << "-d, --decompress"
                      << "\t\t\t"
                      << "decompress file"
                      << "\n"
                      << "-i, --input [INPUT_FILE]"
                      << "\t"
                      << "path to file to (de)compress"
                      << "\n"
                      << "-o, --output [OUTPUT_FILE]"
                      << "\t"
                      << "path to (de)compressed file"
                      << "\n";
            break;
        }
        if (strcmp(argc[i], "--compress") == 0 || strcmp(argc[i], "-c") == 0)
        {
            type = "pack";
            inputFlag = outFlag = false;
        }
        if (strcmp(argc[i], "--decompress") == 0 || strcmp(argc[i], "-d") == 0)
        {
            type = "unpack";
            inputFlag = outFlag = false;
        }
        if (strcmp(argc[i], "--output") == 0 || strcmp(argc[i], "-o") == 0)
        {
            outFlag = true;
            inputFlag = false;
            continue;
        }
        if (strcmp(argc[i], "--input") == 0 || strcmp(argc[i], "-i") == 0)
        {
            inputFlag = true;
            outFlag = false;
            continue;
        }
        if (outFlag)
        {
            outPath.assign(argc[i]);
            outPath = argc[i];
        }
        if (inputFlag)
            inputPath = std::string(argc[i]);
    }
    if (type == "pack")
    {
        compress(inputPath, outPath);
    }
    else if (type == "unpack")
    {
        decompress(inputPath, outPath);
    }
    else
    {
        std::cout << "Invalid type" << "\n";
    }
}