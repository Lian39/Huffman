#include "Huffman.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>

void readFile(std::string path, std::string &data)
{
    std::ifstream inputFile(path, std::ios::binary);

    while (!inputFile.eof())
    {
        char ch;

        inputFile.read(&ch, sizeof(ch));
        data += ch;
    }

    data.pop_back();
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

void encode(std::string &data, std::multimap<char, std::string> &codesTable, std::vector<bool> &encodedData)
{
    for (auto ch : data)
    {
        for (auto bit : codesTable.find(ch)->second)
        {
            encodedData.push_back(bit == '0' ? 0 : 1);
        }
    }
}

void writeFile(std::string path, std::vector<bool> &encodedData)
{
    std::ofstream outFile(path, std::ios::binary);

    for (size_t i = 0; i <= encodedData.size() / 8; ++i)
    {
        unsigned char ch = 0;

        for (int j = 0; j < 8; ++j)
        {
            if (encodedData[i])
                ch |= 1 << j;
        }
        outFile.write((char *)&ch, sizeof(ch));
    }
}

void compress(std::string inputPath, std::string outPath)
{
    std::string data;
    readFile(inputPath, data);

    std::multimap<int, char> freqsTable;
    createFreqsTable(data, freqsTable);

    std::priority_queue<Node *, std::vector<Node *>, comp> pq;
    buildTree(pq, freqsTable);
    Node *root = pq.top();

    std::multimap<char, std::string> codesTable;
    createCodesTable(root, "", codesTable);

    std::vector<bool> encodedData;
    encode(data, codesTable, encodedData);

    // for (auto pair : codesTable)
    //     std::cout << pair.first << "\t" << pair.second << "\n";

    // for (auto i : encodedData)
    //     std::cout << i;

    writeFile(outPath, encodedData);
}

void decompress(std::string inputPath, std::string outPath)
{
    std::string data;
    readFile(inputPath, data);
}

int main(int argv, char *argc[])
{

    // if (argv > 1)
    // {
    //     bool inputFlag = false, outFlag = false;
    //     std::string inputPath = "";
    //     std::string outPath = "";
    //     std::string type;

    //     for (int i = 1; i < argv; i++)
    //     {
    //         if (strcmp(argc[i], "--help") || strcmp(argc[i], "-h"))
    //         {
    //             std::cout << ""
    //                       << "\n";
    //         }
    //         if (strcmp(argc[i], "--pack") == 0)
    //         {
    //             type = "pack";
    //             inputFlag = outFlag = false;
    //         }
    //         if (strcmp(argc[i], "--unpack") == 0)
    //         {
    //             type = "unpack";
    //             inputFlag = outFlag = false;
    //         }
    //         if (strcmp(argc[i], "--out") == 0)
    //         {
    //             outFlag = true;
    //             inputFlag = false;
    //             continue;
    //         }
    //         if (strcmp(argc[i], "--files") == 0)
    //         {
    //             inputFlag = true;
    //             outFlag = false;
    //             continue;
    //         }
    //         if (outFlag)
    //         {
    //             outPath.assign(argc[i]);
    //             outPath = argc[i];
    //         }
    //         if (inputFlag)
    //             inputPath = std::string(argc[i]);
    //     }
    //     outPath = "./test.huff";
    //     inputPath = "./test.txt";
    //     compress(inputPath, outPath);
    // }

    std::string outPath = "./test.huff";
    std::string inputPath = "./test.txt";
    compress(inputPath, outPath);

    outPath = "./test.txt";
    inputPath = "./test.huff";
    decompress(inputPath, outPath);

    return 0;
}