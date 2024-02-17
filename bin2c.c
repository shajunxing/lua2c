/*
The MIT License (MIT)

Copyright (C) 2024 shajunxing

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *in_fname, *dot, *in_fname_no_ext = 0, *out_fname;
    FILE *in = 0, *out = 0;
    int first, c;
    if (argc < 2) {
        printf("Usage: %s <input filename> [output filename]\n", argv[0]);
        printf("Convert binary file to C-style byte array representation.\n");
        printf("Output file name is optional, if not specified, will be output to stdout.\n");
        return -1;
    }
    in_fname = argv[1];
    in_fname_no_ext = (char *)calloc(strlen(in_fname) + 1, 1); // clean alloc会清零内存
    strcpy(in_fname_no_ext, in_fname);
    dot = strrchr(in_fname_no_ext, '.');
    if (dot != 0) {
        *dot = 0;
    }
    if (0 == (in = fopen(in_fname, "rb"))) {
        fprintf(stderr, "Cannot open input file \"%s\": %s\n", in_fname, strerror(errno));
        return -1;
    }
    if (argc == 2) {
        out = stdout;
    } else {
        out_fname = argv[2];
        if (0 == (out = fopen(out_fname, "w"))) {
            fprintf(stderr, "Cannot open output file \"%s\": %s\n", out_fname, strerror(errno));
            return -1;
        }
    }
    fprintf(out, "static const unsigned char %s[] = {", in_fname_no_ext);
    first = 1;
    while (EOF != (c = fgetc(in))) {
        if (first) {
            first = 0;
        } else {
            fprintf(out, ", ");
        }
        fprintf(out, "%d", c);
    }
    fprintf(out, "};\n");
    fclose(out); // 其它内存和句柄都不必释放，反正程序退出了
    return 0;
}