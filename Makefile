stencil: stencil.c
	gcc -std=c99 -Ofast -O3 -fno-inline -DPERF -mfpmath=sse -ftree-vectorizer-verbose=1 -march=corei7 -Wall $^ -o $@

