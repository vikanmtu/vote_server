#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amath.h"
#include "packets.h" //data  fields
#include "shake.h"
#include "trng.h"
#include "b64.h"
#include "sqlite3.h"
#include "bls.h"
#include "whereami.h"
#include "server.h"
#include "srv_verb.h"
#include "srv_idnt.h"
#include "srv_regs.h"
#include "convert.h"

//registrator's keys
typedef struct
{
 fp_t x; //secret key[32]
 ecpoint_fp X; //Q1 public key [68]
 ecpoint_fp2 XX; //Q2 public key [132]
 ecpoint_fp2 YY; //voter's Q2 public key [132]
 fp_t p;  //compressed X
} reg_keys;

//client's data from DB
typedef struct
{
 int n;  //count
 int Idn;  //ID
 char Fio[128]; //FIO
 char Inn[16];  //INN
 char Psw[32];  //Pasword
 char Sec[64];  //Shared secret
 char Sig[128]; //Blind signature
}  reg_data;


extern reg_keys* rkeys;
extern reg_data* rdata;
extern sqlite3 *reg_db;

short srv_inir(void);  //initialize registrator's keys
short srv_get_voter_pubkey(void); //load voter's public key
short srv_set_test(void); //insert test dat to DB

