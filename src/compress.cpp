#include "compress.h"
char magic[9] = "Danuvius";
static long long filesize;
using namespace std;


void compress_file_huffman(string inputFileName, string desFileName) {
    map<char, long long> charFreq = getCharFreq(inputFileName);
    Huffman huffman = Huffman(charFreq);
    Node *hTree = huffman.getHuffmanTree();
    vector<char> v;
    huffman.get_encoding(hTree, v);
    encodeFile(inputFileName, desFileName, huffman.enc, charFreq);
}

int extract_file_huffman(string inputFileName, string desFileName) {
    char verify[9];
    ifstream is(inputFileName, ios::binary);
    is.read(verify, sizeof(verify));
    is.close();
    if (strcmp(magic, verify) != 0) {
        cout << "!" << endl;
        return 1;
    }
    fileHead filehead = readFileHead(inputFileName);
    map<char, long long> charFreq = readFileHaffmanFreq(inputFileName, filehead.alphaVarity);
    decodeFile(inputFileName, desFileName, filehead, charFreq);
    return 0;
}

// #######################################################################################

map<char, long long> getCharFreq(string inputFileName) {
    //用二进制流传输
    ifstream fin(inputFileName, ios::binary);
    fin.seekg(0, ios::end);
    filesize = fin.tellg();
    fin.seekg(0, ios::beg);
    map<char, long long> charFreq;
    char buffer;
    long long charFreqArray[256] = {0L};
    //减少IO次数
    if (filesize == 0) {
        while (!fin.eof()) {
            //每次只接收一个字符,于是会有2^8种字符
            fin.read(&buffer, sizeof(char));
            charFreqArray[(int)buffer + 128]++;
        }
    } else {
        char bufferArray[64 * 1024];
        for (int i = 0; i < filesize / (64 * 1024); i++) {
            fin.read(bufferArray, 64 * 1024 * sizeof(char));
            for (int j = 0; j < 64 * 1024; j++) {
                charFreqArray[(int)bufferArray[j] + 128]++;
            }
        }
        //不满64K的直接读取
        fin.read(bufferArray, (filesize % (64 * 1024)) * sizeof(char));
        for (int j = 0; j < filesize % (64 * 1024); j++) {
            charFreqArray[(int)bufferArray[j] + 128]++;
        }
    }
    //将有数据的字符写进map
    for (int i = 0; i < 256; i++) {
        if (charFreqArray[i] != 0) {
            charFreq.insert(map<char, long long>::value_type((char)i - 128, charFreqArray[i]));
        }
    }
    fin.close();
    return charFreq;
}

void encodeFile(string inputFileName, string desFileName, map<char, string> charCode, map<char, long long> charFreq) {
    ofstream fout;
    fout = ofstream(desFileName, ios::binary);
    fout.write(magic, sizeof(magic));
    //先将文件的头信息写好
    fileHead filehead;
    // 获取字符的种类,写头信息
    filehead.originchars = filesize;
    filehead.alphaVarity = charCode.size();
    fout.write((char *)&filehead, sizeof(filehead));
    //写字符的频度等等
    for (auto i : charFreq) {
        alphaCode af(i);
        fout.write((char *)&af, sizeof(af));
    }
    //将哈夫曼码放到一个结构数组里
    struct CharInfo {
        CharInfo() :
            code("") {
        }
        string code;
    } charInfoArray[256];
    //字母及其哈夫曼码
    map<char, string>::reverse_iterator iter;
    for (iter = charCode.rbegin(); iter != charCode.rend(); iter++) {
        charInfoArray[(int)iter->first + 128].code = iter->second;
    }
    //写文件的内容
    ifstream fin(inputFileName, ios::binary);
    int bufferLength = 0;
    //每次只接收一个字符,于是会有2^8种字符
    unsigned char bufferbit = 0;
    char readChars[512 * 1024];
    char bufferArray[512 * 1024];
    int bufferArrayIndex = 0;
    char buffer;
    if (filesize == 0) {
        while (!fin.eof()) {
            fin.read(&buffer, sizeof(char));
            for (int strIdx = 0; strIdx < charInfoArray[(int)buffer + 128].code.length(); strIdx++) {
                bufferbit <<= 1;
                bufferbit |= (charInfoArray[(int)buffer + 128].code[strIdx] == '1');
                bufferLength++;
                //当有一个字符的时候就放进待写数组里
                if (bufferLength == 8) {
                    bufferArray[bufferArrayIndex] = bufferbit;
                    bufferArrayIndex++;
                    bufferbit = 0;
                    bufferLength = 0;
                }
                //如果buffer数组满了,就写入文件
                if (bufferArrayIndex == 1024) {
                    fout.write(bufferArray, sizeof(char) * 1024);
                    bufferArrayIndex = 0;
                }
            }
        }
    } else {
        for (int i = 0; i < (filesize / (512 * 1024)); i++) {
            fin.read(readChars, (512 * 1024) * sizeof(char));
            for (int j = 0; j < (512 * 1024); j++) {
                for (int strIdx = 0; strIdx < charInfoArray[(int)readChars[j] + 128].code.length(); strIdx++) {
                    bufferbit <<= 1;
                    bufferbit |= (charInfoArray[(int)readChars[j] + 128].code[strIdx] == '1');
                    bufferLength++;
                    //当有一个字符的时候就放进待写数组里
                    if (bufferLength == 8) {
                        bufferArray[bufferArrayIndex] = bufferbit;
                        bufferArrayIndex++;
                        bufferbit = 0;
                        bufferLength = 0;
                    }
                    //如果buffer数组满了,就写入文件
                    if (bufferArrayIndex == 512 * 1024) {
                        fout.write(bufferArray, sizeof(char) * 512 * 1024);
                        bufferArrayIndex = 0;
                    }
                }
            }
        }
        fin.read(readChars, (filesize % (512 * 1024)) * sizeof(char));
        for (int j = 0; j < (filesize % (512 * 1024)); j++) {
            for (int strIdx = 0; strIdx < charInfoArray[(int)readChars[j] + 128].code.length(); strIdx++) {
                bufferbit <<= 1;
                bufferbit |= (charInfoArray[(int)readChars[j] + 128].code[strIdx] == '1');
                bufferLength++;
                //当有一个字符的时候就放进待写数组里
                if (bufferLength == 8) {
                    bufferArray[bufferArrayIndex] = bufferbit;
                    bufferArrayIndex++;
                    bufferbit = 0;
                    bufferLength = 0;
                }
                //如果buffer数组满了,就写入文件
                if (bufferArrayIndex == 512 * 1024) {
                    fout.write(bufferArray, sizeof(char) * 512 * 1024);
                    bufferArrayIndex = 0;
                }
            }
        }
    }
    //将最后不足8位的bit补全并写入
    if (bufferLength != 0) {
        while (bufferLength != 8) {
            bufferbit <<= 1;
            bufferLength++;
        }
        bufferArray[bufferArrayIndex] = bufferbit;
        bufferArrayIndex++;
    }
    //写数组剩下的东西
    if (bufferArrayIndex != 0) {
        fout.write(bufferArray, bufferArrayIndex * sizeof(char));
    }
    fout.close();
}

fileHead readFileHead(string inputFileName) {
    fileHead filehead;
    ifstream is(inputFileName, ios::binary);
    is.seekg(sizeof(magic));
    is.read((char *)&filehead, sizeof(filehead));
    is.close();
    return filehead;
}
map<char, long long> readFileHaffmanFreq(string inputFileName, int alphaVariety) {
    ifstream is(inputFileName, ios::binary);
    map<char, long long> codeFreq;
    //定位在头信息后
    is.seekg(sizeof(magic) + sizeof(fileHead));
    //将各个字符的频度获取
    for (int i = 0; i < alphaVariety; i++) {
        alphaCode af;
        is.read((char *)&af, sizeof(af));
        codeFreq.insert(make_pair(af.alpha, af.freq));
    }
    is.close();
    return codeFreq;
}
void decodeFile(string inputFileName, string desFileName, fileHead filehead, map<char, long long> charFreq) {
    ifstream is(inputFileName, ios::binary);
    ofstream out(desFileName, ios::binary);
    //空文件直接return
    if (charFreq.size() == 0)
        return;
    is.seekg(0, ios::end);
    long long x = is.tellg();
    //恢复哈夫曼树
    Huffman huffman = Huffman(charFreq);
    Node root = *huffman.getHuffmanTree();
    Node temp = root;
    //定位到存储文件的位置
    is.seekg(sizeof(magic) + sizeof(filehead) + filehead.alphaVarity * sizeof(alphaCode), ios::beg);
    //开始读取
    char readBuf;
    long long writedchars = 0;
    char readChars[512 * 1024];
    char writeBufferArray[512 * 1024];
    int writeBufferArrayIndex = 0;
    long long filesize = x - sizeof(filehead) + filehead.alphaVarity * sizeof(alphaCode);
    for (int i = 0; i < filesize / (512 * 1024); i++) {
        is.read(readChars, (512 * 1024) * sizeof(char));
        for (int j = 0; j < (512 * 1024); j++) {
            for (int k = 7; k >= 0; k--) {
                if (readChars[j] & (1 << k))
                    temp = *temp.right;
                else
                    temp = *temp.left;
                if (huffman.isLeaf(&temp)) {
                    //该字符放到缓存数组里
                    writeBufferArray[writeBufferArrayIndex] = temp.val;
                    //缓存指针加一
                    writeBufferArrayIndex++;
                    temp = root;
                    writedchars++;
                }
                //缓存数组满,写入文件
                if (writeBufferArrayIndex == 512 * 1024) {
                    out.write(writeBufferArray, 512 * 1024 * sizeof(char));
                    writeBufferArrayIndex = 0;
                }
                if (writedchars >= filehead.originchars) {
                    goto finish;
                }
            }
        }
    }
    is.read(readChars, (filesize % (512 * 1024)) * sizeof(char));
    for (int j = 0; j < (filesize % (512 * 1024)); j++) {
        for (int k = 7; k >= 0; k--) {
            if (readChars[j] & (1 << k)) {
                if (temp.right)
                    temp = *temp.right;
            } else {
                if (temp.left)
                    temp = *temp.left;
            }
            if (huffman.isLeaf(&temp)) {
                //该字符放到缓存数组里
                writeBufferArray[writeBufferArrayIndex] = temp.val;
                //缓存指针加一
                writeBufferArrayIndex++;
                temp = root;
                writedchars++;
            }
            //缓存数组满,写入文件
            if (writeBufferArrayIndex == 512 * 1024) {
                out.write(writeBufferArray, 512 * 1024 * sizeof(char));
                writeBufferArrayIndex = 0;
            }
            if (writedchars >= filehead.originchars) {
                goto finish;
            }
        }
    }
//将残留的数据写进去
finish:;
    out.write(writeBufferArray, writeBufferArrayIndex * sizeof(char));
    out.close();
}


void compress_file_lzw(string filename) {
    int size;
    char *content = BlockIO(filename, size);
    vector<int> compressed;
    compress(string(content, size), back_inserter(compressed));
    binaryWrite(compressed, filename);
    delete content;
}

void extract_file_lzw(string filename) {
    vector<int> code = binaryRead(filename);
    string decompressed = decompress(code.begin(), code.end());

    // chop off the .lzw
    filename = filename.substr(0, filename.find_last_of("."));
    // insert a 2M
    filename.insert(filename.find_first_of("."), "2M");

    ofstream outfile(filename, ios::binary);
    outfile.write(decompressed.c_str(), decompressed.size());
}