CC = gcc
CFLAGS += -O3 -march=native -mtune=native -pg -w -mavx2
SOURCES= main.c mq_ntt.c sample.c poly.c pke.c fips202.c code.c kem.c randombytes.c
HEADERS= mq_ntt.h mq_ntt_param.h sample.h poly.h pke.h fips202.h code.h kem.h randombytes.h


main : $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

clean:
	rm -f main


          
