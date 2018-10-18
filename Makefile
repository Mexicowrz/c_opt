stencil: stencil.c
	#gcc -std=c99 -Ofast -O3 -fno-inline -DPERF -mfpmath=sse -ftree-vectorizer-verbose=1 -march=corei7 -Wall $^ -o $@
        icc -fast -O3 -fp-model fast=2 -no-prec-div -ipo -xHost -std=c99 $^ -o $@
