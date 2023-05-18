#include "compress.h"
#include "huffman.h"

using namespace std;
Node *Huffman::getHuffmanTree() {
    while (node_queue.size() > 1) {
        Node *tmp1 = node_queue.top();
        node_queue.pop();
        Node *tmp2 = node_queue.top();
        node_queue.pop();
        Node *root = new Node('\0', tmp1->freq + tmp2->freq);
        root->left = tmp1;
        root->right = tmp2;
        node_queue.push(root);
    }
    return node_queue.top();
}

void Huffman::delete_node(Node *t) {
    if (t == nullptr)
        return;
    delete_node(t->left);
    delete_node(t->right);
    delete t;
}

/*
获取编码表
root: 当前的节点
state: 从根节点到当前节点的路径产生的比特串
*/
void Huffman::get_encoding(Node *root, vector<char> &state) {
    if (root != nullptr) {
        if (root->left == nullptr && root->right == nullptr) {
            string tmp = "";
            for (int i = 0; i < state.size(); i++)
                tmp += state[i];
            enc[root->val] = tmp;
        } else {
            if (root->left != nullptr) {
                state.push_back('0');
                get_encoding(root->left, state);
                state.pop_back();
            }
            if (root->right != nullptr) {
                state.push_back('1');
                get_encoding(root->right, state);
                state.pop_back();
            }
        }
    }
}
