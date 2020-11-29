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
#include "srv_gets.h"
#include "srv_join.h"
#include "srv_vote.h"
#include "srv_open.h"
#include "convert.h"


//voter's keys
typedef struct
{
 fp_t x;  //secret key[32]
 ecpoint_fp X; //Q1 public key [68]
 ecpoint_fp2 XX; //Q2 public key [132]
 ecpoint_fp2 YY; //registrator's Q2 public key [132]
 fp_t rps; //own signature of YY
 fp_t p;  //compressed X
 char cnd[12*16]; //candidates
 char onion[32]; //server's onion address
} vot_keys;


typedef struct
{
 int n; //count
 int Idn; //ID
 char Key[64]; //hash[16] of client's public key
 char Vot[63]; //vote
 char Mac[63]; //mac
 int Tmr; //voting time
} vot_data;

extern vot_keys* vkeys;
extern vot_data* vdata;
extern sqlite3 *vot_db;

short srv_iniv(void);
short srv_get_reg_pubkey(void);
