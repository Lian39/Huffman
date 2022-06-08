#include "Huffman.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>

void buildFreqsTable(std::ifstream &file, std::multimap<int, char> &freqsTable)
{
    std::vector<int> freqs(0x100);

    while (!file.eof())
    {
        char ch;
        file.read(&ch, sizeof(ch));
        freqs[static_cast<unsigned char>(ch)]++;
    }

    for (size_t i = 0; i < freqs.size(); ++i)
    {
        if (freqs[i] > 0)
            freqsTable.insert(std::make_pair(freqs[i], (unsigned char)i));
    }
}

void buildTree(std::multimap<int, char> &freqsTable)
{
    std::priority_queue<Node *, std::vector<Node *>, comp> pq;

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

    std::cout << pq.top()->freq << "\n";
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

        std::multimap<char, std::string> codesTable;
        inputFiles.push_back("./test.txt");
        for (size_t i = 0; i < inputFiles.size(); ++i)
        {
            std::ifstream file(inputFiles[i], std::ios::binary);
            std::multimap<int, char> freqsTable;

            buildFreqsTable(file, freqsTable);
            buildTree(freqsTable);
        }
    }
    return 0;
}