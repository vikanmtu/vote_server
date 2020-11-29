#system depened libs
ifdef SYSTEMROOT
#Win32    
TFLIBS= -lcomctl32 -lws2_32
else
   ifeq ($(shell uname), Linux)
#Linux     
TFLIBS= -lm -pthread -ldl
   endif
endif

#compiller
CC = gcc -Wall -O2
#compiller flags (not used yet)
CFLAGS = -Ipair -Ipair/include -Ipair/demo -Ipair/test -Ipair/src -Ipair/src/gss -Ipair/src/hash -Ipair/src/ibe -Ipair/src/base/std/bigint -Ipair/src/base/std/ec -Ipair/src/base/std/fp -Ipair/base/src/std/pbc -Irnd -Ishake -Isql -Isrv -Itcp -Itools -Iecc -Iweb
#-fomit-frame-pointer -ffast-math -funroll-loops
#including pathes
TFPATH = -Ipair -Ipair/include -Ipair/demo -Ipair/test -Ipair/src -Ipair/src/gss -Ipair/src/hash -Ipair/src/ibe -Ipair/src/base/std/bigint -Ipair/src/base/std/ec -Ipair/src/base/std/fp -Ipair/base/src/std/pbc -Irnd -Ishake -Isql -Isrv -Itcp -Itools -Iecc -Iweb

#============================Source files===============================
#pairing utils
PAIRUTILS = pair/src/param.c pair/src/rand.c pair/src/bench_util.c pair/src/myassert.c pair/src/myutil.c 
#Pairing bigint 
PAIRBI = pair/src/base/std/bigint/bi_const.c pair/src/base/std/bigint/bi.c
#Pairing fields
PAIRFP = pair/src/base/std/fp/fp2.c pair/src/base/std/fp/fp4.c pair/src/base/std/fp/fp12.c pair/src/base/std/fp/fp.c
#Pairing curve
PAIREC = pair/src/base/std/ec/ecfp2.c pair/src/base/std/ec/ecfp.c pair/src/base/std/ec/compress.c pair/src/base/std/pbc/pbc_bn.c
#BLS and blind signature
PAIRBLS = pair/src/hash/Keccak-compact.c pair/src/hash/sha1.c pair/src/hash/hashing.c pair/src/gss/bls.c
#ellipic curve X25519
ECCFILES = ecc/add.c ecc/copy.c ecc/cswap.c ecc/from_bytes.c ecc/invert.c ecc/mul.c ecc/mul121665.c  ecc/sqr.c ecc/sub.c ecc/to_bytes.c ecc/scalarmult.c ecc/r2p.c 
#hashing and PRNG
RNDFILES = shake/KeccakP800.c shake/shake.c rnd/timing.c rnd/havege.c rnd/trng.c 
#tools
TOOLS = tools/amath.c tools/b64.c tools/base32.c tools/srv_verb.c tools/cli_verb.c tools/convert.c tools/whereami.c 
#SQL, tcp, web 
WEBSQL = sql/sqlite3.c web/webtcp.c web/websrv.c web/websrv_ru.c tcp/trr.c 
#Calculator
CALCULATOR = srv/srv_iniv.c srv/srv_gets.c srv/srv_join.c srv/srv_vote.c srv/srv_open.c srv/srv_webv.c 
#Registrator
REGISTRATOR = srv/srv_inir.c srv/srv_idnt.c srv/srv_regs.c srv/srv_webr.c srv/server.c


#build the binary.
all: vote

vote: $(PAIRUTILS) $(PAIRBI) $(PAIRFP) $(PAIREC) $(PAIRBLS) $(ECCFILES) $(RNDFILES) $(TOOLS) $(WEBSQL) $(CALCULATOR) $(REGISTRATOR)
	$(CC) $(TFPATH) -o $@ $@.c $(PAIRUTILS) $(PAIRBI) $(PAIRFP) $(PAIREC) $(PAIRBLS) $(ECCFILES) $(RNDFILES) $(TOOLS) $(WEBSQL) $(CALCULATOR) $(REGISTRATOR) $(TFLIBS)
	
clean:
	rm -f *.o *.exe *.a vote
	rm -- **/*.o
	rm -- **/**/*.o
	rm -- **/**/**/*.o
	rm -- **/**/**/**/*.o
	
