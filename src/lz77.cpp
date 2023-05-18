#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
#define OFFSETBITS 12
#define LENGTHBITS 4
#define WINDOWSIZE (1 << OFFSETBITS)

#define get_filesize(f, p)                                                     \
  {                                                                            \
    fseek(f, 0, SEEK_END);                                                     \
    p = ftell(f);                                                              \
    fseek(f, 0, SEEK_SET);                                                     \
    p -= ftell(f);                                                             \
  }


int lz77_decode(FILE *in, FILE *out) {
  unsigned char *nbuf = 0, *ori_buf = 0, *head = 0, *tail = 0;
  long ori_size = 0, nsize = 0;

  if (!in || !out)
    return 1;

  get_filesize(in, nsize); // get file size.
  if (nsize <= 0)
    return 1;

  ori_size = (fgetc(in) & 0xff) << 24;
  ori_size += (fgetc(in) & 0xff) << 16;
  ori_size += (fgetc(in) & 0xff) << 8;
  ori_size += (fgetc(in) & 0xff) << 0;
  if (ori_size <= 0)
    return 1;

  nbuf = (unsigned char *)malloc(nsize * sizeof(unsigned char));
  if (!nbuf)
    return 1;

  ori_buf = (unsigned char *)malloc(ori_size * sizeof(unsigned char));
  if (ori_buf) {
    head = tail = ori_buf; // store head and tail of slide-window.

    if (fread(nbuf, sizeof(unsigned char), nsize, in) == (nsize - 4)) {
      unsigned char *pt = nbuf;

      for (;;) {
        if (pt >= (nbuf + nsize - 4))
          break;
        if (!pt[0] && !pt[1]) { // \0\0\?
          tail[0] = pt[2];
          tail++;
          pt += 3;
        } else { // \?\?
          unsigned int offset = 0, length = 0, i;
          offset = pt[0] << LENGTHBITS;
          offset += (pt[1] >> LENGTHBITS) & 0x0f;
          length = pt[1] & 0x0f;
          // fprintf(stderr,"{%d - %d}",offset,length);
          for (i = 0; i < length; i++) {
            tail[i] = ((unsigned char *)(tail - offset))[i];
          }
          tail += length;
          pt += 2;
        }
      }
      fwrite(ori_buf, sizeof(unsigned char), ori_size, out);
    }

    free(ori_buf);
  }

  free(nbuf);
  return 0;
}

/*
 * head | ----------{pt,tmp}|{tail}--------- | end<EOF>
 */
static unsigned int search_maxlen_p(unsigned char **pt, unsigned char *head,
                             unsigned char *tail, unsigned char *end) {
  unsigned char *tmp = *pt;
  unsigned int leng = 0, i;

  for (; tmp >= head; tmp--) {
    if (tmp[0] == tail[0]) {
      for (i = 0; (i < ((1 << LENGTHBITS) - 1)) && (&tmp[i] > head) &&
                  (&tmp[i] < tail) && (&tail[i] < end) && (tmp[i] == tail[i]);
           i++)
        ;
      if (i > leng) {
        leng = i;
        *pt = tmp;
      }
    }
  }
  return leng;
}

int lz77_encode(FILE *in, FILE *out) {
  unsigned char *buf = 0, *head = 0, *tail = 0;
  long size = 0;

  if (!in || !out)
    return 1;

  get_filesize(in, size); // get file size.
  if (size <= 0)
    return 1;

  buf =
      (unsigned char *)malloc(size * sizeof(unsigned char)); // allocate memory.
  if (!buf)
    return 1;

  if (fread(buf, sizeof(unsigned char), size, in) == size) {
    head = tail = buf;
    fputc((size >> 24) & 0xff, out);
    fputc((size >> 16) & 0xff, out);
    fputc((size >> 8) & 0xff, out);
    fputc((size >> 0) & 0xff, out);

    for (;;) { // loop block.
      head = tail - WINDOWSIZE;
      if (buf > head)
        head = buf;
      if (tail > buf + size - 1)
        break;
      if (tail > head) {
        unsigned char *pt = tail - 1;
        unsigned int bytes = 0;
        bytes = search_maxlen_p(&pt, head, tail, buf + size);
        // search maximal matched bytes and position.
        if (bytes > 0) {
          unsigned char chars[2] = {0};
          chars[0] = (unsigned char)(((tail - pt) >> LENGTHBITS) & 0xff);
          chars[1] =
              (unsigned char)((((tail - pt) << LENGTHBITS) + bytes) & 0xff);
          fputc(chars[0], out);
          fputc(chars[1], out);
          // fprintf(stderr,"{%d - %d}",tail - pt,bytes);
          tail += bytes;
        } else {
          fputc(0, out);
          fputc(0, out);
          fputc(*tail, out);
          tail++;
        }
      } else {
        fputc(0, out);
        fputc(0, out);
        fputc(*tail, out);
        tail++;
      }
    }
  }
  free(buf);
  return 0;
}