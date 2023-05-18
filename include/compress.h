#ifndef __COMPRESS__
#define __COMPRESS__
#include "huffman.h"
#include "lzw.h"
using namespace std;
void compress_file_huffman(string sourceFilename, string desFilename);
int extract_file_huffman(string sourceFilename, string desFilename);

void compress_file_lzw(string sourceFilename);
void extract_file_lzw(string sourceFilename);

int lz77_encode(FILE *in, FILE *out);
int lz77_decode(FILE *in, FILE *out);

int lzss_encode(FILE *in, FILE *out);
int lzss_decode(FILE *in, FILE *out);
#endif
