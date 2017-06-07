# http://oss.sgi.com/LDP/HOWTO/UPS-HOWTO-8.html

#DEBSRC=getchar_ne.c

all: passemu

passemu: aes_cbc_decrypt.c pwman.c powerd.c $(DEBSRC) passemu.c passemu.h
	$(CC) -O3 -Wall -o passemu powerd.c $(DEBSRC) pwman.c passemu.c aes_cbc_decrypt.c -lcrypto
	strip passemu

clean:
	rm -f *.o
	rm -f passemu
