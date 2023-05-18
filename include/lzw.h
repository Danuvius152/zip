#ifndef __LZW__
#define __LZW__

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <stdexcept>
using namespace std;

string int2BinaryString(int c, int cl);
int binaryString2Int(string p);
void binaryWrite(vector<int> &compressed, string filename);
vector<int> binaryRead(string filename);
char *BlockIO(string filename, int &size_out);

/* reference: https://rosettacode.org/wiki/LZW_compression */

template <typename Iterator>
Iterator compress(const string &uncompressed, Iterator result) {
    // Build the dictionary.
    int dictSize = 256;
    map<string, int> dictionary;
    for (int i = 0; i < 256; i++)
        dictionary[string(1, i)] = i;

    string w;
    for (auto it = uncompressed.begin(); it != uncompressed.end(); ++it) {
        char c = *it;
        string wc = w + c;
        if (dictionary.count(wc))
            w = wc;
        else {
            *result++ = dictionary[w];
            // Add wc to the dictionary. Assuming the size is 2^16 = 65536
            if (dictionary.size() < (1 << 16))
                dictionary[wc] = dictSize++;
            w = string(1, c);
        }
    }
    // Output the code for w.
    if (!w.empty())
        *result++ = dictionary[w];
    return result;
}

// "begin" and "end" must form a valid range of ints
template <typename Iterator>
string decompress(Iterator begin, Iterator end) {
    // Build the dictionary.
    int dictSize = 256;
    map<int, string> dictionary;
    for (int i = 0; i < 256; i++)
        dictionary[i] = string(1, i);

    string w(1, *begin++);
    string result = w;
    string entry;

    for (; begin != end; begin++) {
        int k = *begin;
        if (dictionary.count(k))
            entry = dictionary[k];
        else if (k == dictSize) {
            entry = w + w[0];
        } else {
            throw "Bad compressed k";
        }

        result += entry;

        // Add w+entry[0] to the dictionary.
        if (dictionary.size() < (1 << 16))
            dictionary[dictSize++] = w + entry[0];

        w = entry;
    }

    return result;
}

#endif