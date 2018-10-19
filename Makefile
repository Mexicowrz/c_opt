stencil: stencil.c
	gcc -std=c99 -O3 -ftree-vectorize -ftree-vectorizer-verbose=5 -msse2 -Wall $^ -o $@
	#icc -std=c99 -Wall -fast $^ -o $@
