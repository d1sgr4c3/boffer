obj-m := vuln.o

all:
	make -C ../ M=./vuln

clean:
	make -C ../ M=./vuln clean
	rm -f *.o *.ko *.mod.c *.mod.o *.order

