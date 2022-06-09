#include "Huffman.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>

void readFile(const std::string path, std::string &data)
{
    std::ifstream inputFile(path, std::ios::binary);

    while (!inputFile.eof())
    {
        char ch;
        inputFile.read(&ch, sizeof(ch));
        data += ch;
    }
}

void createFreqsTable(const std::string data, std::multimap<int, char> &freqsTable)
{
    std::vector<int> freqs(0x100);

    for (auto ch : data)
    {
        freqs[static_cast<unsigned char>(ch)]++;
    }

    for (size_t i = 0; i < freqs.size(); ++i)
    {
        if (freqs[i] > 0)
            freqsTable.insert(std::make_pair(freqs[i], (unsigned char)i));
    }
}

void buildTree(std::priority_queue<Node *, std::vector<Node *>, comp> &pq, const std::multimap<int, char> freqsTable)
{
    for (auto pair : freqsTable)
    {
        Node *node = new Node(pair.second, pair.first, nullptr, nullptr);
        pq.push(node);
    }

    while (pq.size() != 1)
    {
        Node *left = pq.top();
        pq.pop();
        Node *right = pq.top();
        pq.pop();

        int freqsSum = left->freq + right->freq;
        Node *node = new Node('\0', freqsSum, left, right);
        pq.push(node);
    }
}

void createCodesTable(Node *node, std::string code, std::multimap<char, std::string> &codesTable)
{
    if (node == nullptr)
        return;

    if (!node->left && !node->right)
        codesTable.insert(std::make_pair(node->ch, code));

    createCodesTable(node->left, code + "0", codesTable);
    createCodesTable(node->right, code + "1", codesTable);
}

void encode(std::string &data, const std::multimap<char, std::string> codesTable, std::vector<bool> &encodedData)
{
    for (auto ch : data)
    {
        for (auto bit : codesTable.find(ch)->second)
        {
            encodedData.push_back(bit == '0' ? 0 : 1);
        }
    }
}

void decode(Node *root, const std::vector<bool> encodedData, std::string &decodedData)
{
    Node *node = root;

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
}

void serializeTree(Node *root, std::ofstream &outFile)
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

void deserializeTree(Node *root, std::ifstream &inputFile)
{
    char ch;

    if (!inputFile.read((char *)&ch, sizeof(ch)) || root == nullptr)
        return;

    if (ch == '0')
    {
        Node *leftNode = new Node('\0', 0, nullptr, nullptr);
        Node *rightNode = new Node('\0', 0, nullptr, nullptr);

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

void readFile(std::string path, std::vector<bool> &encodedData, Node *root)
{
    std::ifstream inputFile(path, std::ios::binary);

    deserializeTree(root, inputFile);

    while (!inputFile.eof())
    {
        char ch;
        inputFile.read(&ch, sizeof(ch));
        for (int i = 0; i < 8; ++i)
        {
            encodedData.push_back((ch >> i) & 1);
        }
    }
}

void writeFile(const std::string outPath, const std::vector<bool> encodedData, Node *root)
{
    std::ofstream outFile = std::ofstream(outPath, std::ios::binary);

    serializeTree(root, outFile);

    for (size_t i = 0; i <= encodedData.size() / 8; ++i)
    {
        unsigned char ch = 0;

        for (int j = 0; j < 8; ++j)
        {
            if (encodedData[i * 8 + j])
                ch |= (1 << j);
        }
        outFile.write((char *)&ch, sizeof(ch));
    }
}

void writeFile(const std::string outPath, const std::string decodedData)
{
    std::ofstream outFile(outPath, std::ios::binary);

    for (auto i : decodedData)
    {
        outFile.write((char *)&i, sizeof(i));
    }
}

void compress(const std::string inputPath, const std::string outPath)
{
    std::string data;
    readFile(inputPath, data);
    std::multimap<int, char> freqsTable;
    std::priority_queue<Node *, std::vector<Node *>, comp> pq;
    std::vector<bool> encodedData;
    std::multimap<char, std::string> codesTable;

    createFreqsTable(data, freqsTable);
    buildTree(pq, freqsTable);
    Node *root = pq.top();
    createCodesTable(root, "", codesTable);
    encode(data, codesTable, encodedData);
    writeFile(outPath, encodedData, root);
}

void decompress(std::string inputPath, std::string outPath)
{
    std::vector<bool> encodedData;
    std::string decodedData;
    Node *root = new Node('\0', 0, nullptr, nullptr);

    readFile(inputPath, encodedData, root);
    decode(root, encodedData, decodedData);
    writeFile(outPath, decodedData);
}

int main(int argv, char *argc[])
{

    if (argv > 1)
    {
        bool inputFlag = false, outFlag = false;
        std::string inputPath = "";
        std::string outPath = "";
        std::string type;

        for (int i = 1; i < argv; i++)
        {
            if (strcmp(argc[i], "--help") || strcmp(argc[i], "-h"))
            {
                std::cout << ""
                          << "\n";
            }
            if (strcmp(argc[i], "--pack") == 0)
            {
                type = "pack";
                inputFlag = outFlag = false;
            }
            if (strcmp(argc[i], "--unpack") == 0)
            {
                type = "unpack";
                inputFlag = outFlag = false;
            }
            if (strcmp(argc[i], "--out") == 0)
            {
                outFlag = true;
                inputFlag = false;
                continue;
            }
            if (strcmp(argc[i], "--file") == 0)
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

    return 0;
}