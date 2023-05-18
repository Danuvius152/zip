#ifndef __Huffman__
#define __Huffman__

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>
using namespace std;

class Node {
public:
    Node *left;     // 左子节点
    Node *right;    // 右子节点
    long long freq; // 频率
    char val;       // 权值
    Node(char val, long long freq) {
        this->val = val;
        this->freq = freq;
        this->left = nullptr;
        this->right = nullptr;
    }
    Node(char val, long long freq, Node *left, Node *right) {
        this->val = val;
        this->freq = freq;
        this->left = left;
        this->right = right;
    }
};

struct comp {
    bool operator()(Node *a, Node *b) {
        if (a->freq != b->freq)
            return a->freq > b->freq;
        else
            return a->val > b->val;
    }
};

class Huffman {
public:
    priority_queue<Node *, vector<Node *>, comp> node_queue;
    map<char, string> enc;
    Huffman(map<char, long long> freq) {
        for (auto i : freq) {
            Node *node = new Node(i.first, i.second);
            node_queue.push(node);
        }
    }
    static bool isLeaf(Node *node) {
        return node->left == nullptr && node->right == nullptr;
    }
    Node *getHuffmanTree();
    void get_encoding(Node *root, vector<char> &state);
    ~Huffman() {
        Node *node = node_queue.top();
        delete_node(node);
    }

private:
    void delete_node(Node *node);
};

struct fileHead {
    //字符种类
    int alphaVarity;
    //源文件的字节数
    long long originchars;
};
//字母及其频度
struct alphaCode {
    char alpha;
    //频度
    long long freq;
    alphaCode() {
    }
    alphaCode(const pair<char, long long> &x) {
        this->alpha = x.first;
        this->freq = x.second;
    }
};

map<char, long long> getCharFreq(string inputFileName);
void encodeFile(string inputFileName, string desFileName, map<char, string> charCode, map<char, long long> charFreq);
fileHead readFileHead(string inputFileName);
map<char, long long> readFileHaffmanFreq(string inputFileName, int alphaVariety);
void decodeFile(string inputFileName, string desFileName, fileHead filehead, map<char, long long> charFreq);

#endif
