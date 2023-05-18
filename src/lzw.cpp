#include "compress.h"
using namespace std;

string int2BinaryString(int c, int cl) {
    string p = ""; // a binary code string with code length = cl
    int code = c;

    // shift and encode c in binary
    while (c > 0) {
        if (c % 2 == 0)
            p = "0" + p;
        else
            p = "1" + p;
        c = c >> 1;
    }

    int zeros = cl - p.size();
    if (zeros < 0) {
        cout << "\nWarning: Overflow. code " << code << " is too big to be coded by " << cl << " bits!\n";
        p = p.substr(p.size() - cl);
    } else {
        for (int i = 0; i < zeros; i++) // pad 0s to left of the binary code if needed
            p = "0" + p;
    }
    return p;
}

int binaryString2Int(string p) {
    int code = 0;
    if (p.size() > 0) {
        if (p.at(0) == '1')
            code = 1;
        p = p.substr(1);
        while (p.size() > 0) {
            code = code << 1;
            if (p.at(0) == '1')
                code++;
            p = p.substr(1);
        }
    }
    return code;
}

void binaryWrite(vector<int> &compressed, string filename) {
    int bits = 9;          //# of bits to use for encoding to start
    int words_written = 0; // use this to decide when to switch word length
    string p;

    // String to store binary code for compressed sequence
    string bcode = "";
    for (auto it = compressed.begin(); it != compressed.end(); ++it) {
        // Find the least power of 2 that encodes the data

        // When we've used up all of our new table space, increase by one bit
        // if we're using 16 bits, stop
        if (words_written == (1 << (bits - 1)) && bits < 16) { // (1 << b) = 2^b
            ++bits;
            words_written = 0;
        }

        // Encode the value in binary
        p = int2BinaryString(*it, bits);
        bcode += p;
        ++words_written;
    }

    filename += ".lzw";
    ofstream outfile(filename, ios::binary);

    string zeros = "00000000";
    if (bcode.size() % 8 != 0) // make sure the length o the binary string is a multiple of 8bits = 1byte
        bcode += zeros.substr(0, 8 - bcode.size() % 8);

    int b;
    for (int i = 0; i < bcode.size(); i += 8) {
        b = 1;
        for (int j = 0; j < 8; ++j) {
            b = b << 1;
            if (bcode.at(i + j) == '1')
                b += 1;
        }
        char c = (char)(b & 255); // save the string byte by byte
        outfile.write(&c, 1);
    }
}

vector<int> binaryRead(string filename) {
    ifstream infile(filename, ios::binary);

    if (!infile.is_open())
        throw runtime_error("could not open file to decompress\n");

    string zeros = "00000000";

    struct stat filestatus;
    stat(filename.c_str(), &filestatus);
    long fsize = filestatus.st_size; // file size in bytes

    char c[fsize];
    infile.read(c, fsize);

    string s = "";  // full binary string file input
    long count = 0; // count how many bytes have been read

    while (count < fsize) {
        unsigned char uc = (unsigned char)c[count];

        // Convert a byte to a binary string
        string p = ""; // binary string
        for (int j = 0; j < 8 && uc > 0; ++j) {
            if (uc % 2 == 0)
                p = "0" + p;
            else
                p = "1" + p;
            uc = uc >> 1;
        }
        p = zeros.substr(0, 8 - p.size()) + p; // pad 0s to the left if needed
        s += p;
        count++;
    }

    // Have a string of binary digits as bytes
    // Need to divide into sets of n bits and decode
    // start reading with 9 bits
    int bits = 9;
    int segments = 0;

    vector<int> code;

    // A segment of n-bit length codes occurs at maximum 2^(n-1) times
    for (int current_bit = 0; current_bit < s.size(); current_bit += bits) {
        // check to see if we need to add a bit
        if (bits < 16 && segments == 1 << (bits - 1)) {
            ++bits;
            segments = 0;
        }

        // Check to see if we have a n-bit segment left to read
        // if not, we can just ignore the rest- it's padding
        if (s.size() - current_bit + 1 < bits) {
            break;
        }

        // Construct a string starting from the current bit and taking the appropriate number of bits
        string segment(&s[current_bit], bits);
        // Convert the segment to an integer and add it to the code
        code.push_back(binaryString2Int(segment));
        // Note that we've consumed a segment
        ++segments;
    }

    return code;
}

char *BlockIO(string filename, int &size_out) {
    // Read bytes
    ifstream myfile(filename.c_str(), ios::binary);

    if (!myfile.is_open())
        throw runtime_error("could not open file");

    streampos begin, end;
    begin = myfile.tellg();
    myfile.seekg(0, ios::end);
    end = myfile.tellg();
    streampos size = end - begin; // size of the file in bytes
    myfile.seekg(0, ios::beg);

    // Make sure to also return how many bytes were read
    // Otherwise, if the String constructer hits a null terminator,
    //  	it stops construction, causing bugs
    size_out = size;

    char *memblock = new char[size];
    myfile.read(memblock, size); // read the entire file
    memblock[size] = '\0';       // add a terminator
    myfile.close();

    return memblock;
}

