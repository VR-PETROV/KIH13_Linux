PATH_L=/usr/lib
PATH_H=/usr/include

kih13l: kih13l.c
	gcc -I$(PATH_H) -I$(PATH_L) kih13l.c -o kih13l.out
