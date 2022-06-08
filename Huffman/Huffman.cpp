#include "Huffman.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>

void readFile(std::string &path, std::string &data)
{
    std::ifstream inputFile(path, std::ifstream::binary);
    
    while (!inputFile.eof())
    {
        char ch;
        inputFile.read(&ch, sizeof(ch));
        data += ch;
    }
}

void createFreqsTable(std::string &data, std::multimap<int, char> &freqsTable)
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

void buildTree(std::priority_queue<Node *, std::vector<Node *>, comp> &pq, std::multimap<int, char> &freqsTable)
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
        Node *newNode = new Node('\0', freqsSum, left, right);
        pq.push(newNode);
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

void encode(std::string &data, std::multimap<char, std::string> &codesTable, std::vector<bool> &compressedData)
{
    for (auto ch : data)
    {
        
    }
}

int main(int argv, char *argc[])
{

    if (argv > 1)
    {
        bool filesFlag = false, outFlag = false;
        std::vector<std::string> inputFiles;
        std::string outFile = "";
        std::string type;

        for (int i = 1; i < argv; i++)
        {
            if (strcmp(argc[i], "--pack") == 0)
            {
                type = "pack";
                filesFlag = outFlag = false;
            }
            if (strcmp(argc[i], "--unpack") == 0)
            {
                type = "unpack";
                filesFlag = outFlag = false;
            }
            if (strcmp(argc[i], "--out") == 0)
            {
                outFlag = true;
                filesFlag = false;
                continue;
            }
            if (strcmp(argc[i], "--files") == 0)
            {
                filesFlag = true;
                outFlag = false;
                continue;
            }
            if (outFlag)
            {
                outFile.assign(argc[i]);
                outFile = argc[i];
            }
            if (filesFlag)
                inputFiles.push_back(std::string(argc[i]));
        }

        for (size_t i = 0; i < inputFiles.size(); ++i)
        {
            std::string data;
            readFile(inputFiles[i], data);

            std::multimap<int, char> freqsTable;
            createFreqsTable(data, freqsTable);

            std::priority_queue<Node *, std::vector<Node *>, comp> pq;
            buildTree(pq, freqsTable);
            Node *root = pq.top();

            std::multimap<char, std::string> codesTable;
            createCodesTable(root, "", codesTable);

            std::vector<bool> compressedData;
            encode(data, codesTable, compressedData);
        }
    }
    return 0;
}