#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

#define OFFSETBITS 12
#define LENGTHBITS 4
#define WINDOWSIZE (1 << OFFSETBITS)
#define OFFLENFLAG 1

#define get_filesize(f, s)                                                     \
  {                                                                            \
    fseek(f, 0, SEEK_END);                                                     \
    s = ftell(f);                                                              \
    fseek(f, 0, SEEK_SET);                                                     \
    s -= ftell(f);                                                             \
  }

#define put_bytes(p, n, o)                                                     \
  { fwrite(p, sizeof(unsigned char), n, o); }

/*
 * head | ----------{pt,tmp}|{tail}--------- | end<EOF>
 */

static unsigned int search_maxlen_p(unsigned char **pt, unsigned char *head,
                             unsigned char *tail, unsigned char *end) {
  unsigned char *tmp = *pt;
  unsigned int leng = 0, i;

  for (; tmp >= head; tmp--) {
    if (tmp[0] == tail[0]) {
      for (i = 0; (i < ((1 << LENGTHBITS) + 1)) && (&tmp[i] > head) &&
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
int lzss_encode(FILE *in, FILE *out) {
  unsigned char *buf = 0, *head = 0, *tail = 0;
  long size = 0;

  if (!in || !out)
    return 1;

  get_filesize(in, size);
  if (size <= 0)
    return 1;

  buf = (unsigned char *)malloc(size * sizeof(unsigned char));
  if (!buf)
    return 1;

  if (fread(buf, sizeof(unsigned char), size, in) == size) {
    unsigned int np = 1, control_count = 0;
    unsigned char suf[17] = {0}; // 2*8 + 1

    fputc((size >> 24) & 0xff, out);
    fputc((size >> 16) & 0xff, out);
    fputc((size >> 8) & 0xff, out);
    fputc((size >> 0) & 0xff, out);

    head = tail = buf;

    for (;;) {
      head = tail - WINDOWSIZE;
      if (head < buf)
        head = buf;

      if (control_count >= 8) {
        put_bytes(suf, np, out);
        memset(suf, 0, sizeof(suf));
        np = 1;
        control_count = 1;
      } else {
        control_count++;
      }

      if (tail > buf + size - 1)
        break;

      if (head < tail) {
        unsigned int bytes = 0;
        unsigned char *pt = tail - 1;
        bytes = search_maxlen_p(&pt, head, tail, buf + size);
        if (bytes >
            2) { // if find a pattern more than 2 bytes, save its offset-length.
          suf[0] <<= 1;
          suf[0] |= OFFLENFLAG;
          suf[np++] = (unsigned char)(((tail - pt) >> LENGTHBITS) & 0xff);
          suf[np++] =
              (unsigned char)((((tail - pt) << LENGTHBITS) + bytes - 2) & 0xff);
          tail += bytes;
        } else {
          suf[0] <<= 1;
          suf[np++] = tail[0];
          tail++;
        }
      } else {
        suf[0] <<= 1;
        suf[np++] = tail[0];
        tail++;
      }
    }

    for (; control_count <= 8; control_count++)
      suf[0] <<= 1; // must keep highest bit is aligned to first byte.
    put_bytes(suf, np, out);
  }

  free(buf);

  return 0;
}
int lzss_decode(FILE *in, FILE *out) {
  unsigned char *in_buffer = 0, *out_buffer = 0, *head = 0, *tail = 0;
  long in_size = 0, out_size = 0;

  if (!in || !out)
    return 1;

  get_filesize(in, in_size);
  if (in_size <= 0)
    return 1;

  out_size = (fgetc(in) & 0xff) << 24;
  out_size += (fgetc(in) & 0xff) << 16;
  out_size += (fgetc(in) & 0xff) << 8;
  out_size += (fgetc(in) & 0xff) << 0;
  if (out_size <= 0)
    return 1;

  in_buffer = (unsigned char *)malloc(sizeof(unsigned char) * in_size);
  if (!in_buffer)
    return 1;

  out_buffer = (unsigned char *)malloc(sizeof(unsigned char) * out_size);
  if (out_buffer) {
    if (fread(in_buffer, sizeof(unsigned char), in_size, in) == in_size - 4) {
      unsigned int control_count = 0;
      unsigned char control_flags = 0;
      unsigned char *pt = (unsigned char *)in_buffer;

      head = tail = out_buffer;
      control_flags = pt[0];
      pt++;

      for (;;) {
        if (pt >= (in_buffer + in_size - 4))
          break;

        if (control_count >= 8) {
          control_flags = pt[0];
          pt++;
          control_count = 1;
        } else {
          control_count++;
        }

        if (control_flags >> 7) { // offset-length.
          unsigned int offset = 0, length = 0, i;
          offset = pt[0] << LENGTHBITS;
          offset += (pt[1] >> LENGTHBITS) & 0x0f;
          length = (pt[1] & 0x0f) + 2;
          for (i = 0; i < length; i++) {
            tail[i] = ((unsigned char *)(tail - offset))[i];
          }
          tail += length;
          pt += 2;
          control_flags <<= 1;
        } else { // literal.
          tail[0] = pt[0];
          tail++;
          pt++;
          control_flags <<= 1;
        }
      }
      fwrite(out_buffer, sizeof(unsigned char), out_size, out);
    }
    free(out_buffer);
  }

  free(in_buffer);

  return 0;
}