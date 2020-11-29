#include <stdio.h>
#include "sqlite3.h"

const char* SQL = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO FOO VALUES(1,2,3); SELECT * FROM FOO;";


const char* SQL_CREATE = "CREATE TABLE IF NOT EXISTS Persons (Idn INTEGER PRIMARY KEY ASC, Fio varchar(127) NOT NULL, Psw varchar(31), 'Sec' varchar(63), 'Sig' varchar(127) );";

const char* SQL_ADD1 = "INSERT INTO Persons (Fio, Psw) VALUES ('Ivanov','12345$678');";
const char* SQL_ADD2 = "INSERT INTO Persons (Fio, Psw) VALUES ('Sidorov','1a2b3&c4d5e');";
const char* SQL_ADD3 = "INSERT INTO Persons (Fio, Psw) VALUES ('Saharov','1A3*B3C');";
const char* SQL_ADD4 = "INSERT INTO Persons (Fio, Psw) VALUES ('Bobrov','ABC_DEF');";
const char* SQL_ADD5 = "INSERT INTO Persons (Fio, Psw) VALUES ('Volkov','abc(a)bc');";

const char* SQL_SEL1 = "SELECT * FROM Persons;";
const char* SQL_SEL2 = "SELECT Fio, Psw, Sec, Sig FROM Persons WHERE Idn=2;";
const char* SQL_SEL3 = "SELECT Idn, Psw, Sec, Sig FROM Persons WHERE Fio='Saharov';";

const char* SQL_UPD1 = "UPDATE Persons SET Sec = 'AABBCCDDEEFF' WHERE Idn = 2;";
const char* SQL_UPD2 = "UPDATE Persons SET Sig = 'A0B1C2E3' WHERE Idn = 2;";


//---------------------------------------------------------------------------


int sqlcb(void* arg, int n, char** data ,char** name)
{
 short i;

 for(i=0;i<n;i++)
 {
  printf("%s = %s ;", name[i], data[i]);
 }
 printf("\r\n");


 return 0;
}


int sql_main(void)
{
       sqlite3 *db = 0;
char *err = 0;
int ret;

if( sqlite3_open("registrator.db3", &db) )
fprintf(stderr, "error of open/create DB: %s\n", sqlite3_errmsg(db));
//
else
{

 ret=sqlite3_exec(db, SQL_CREATE, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 /*

 ret=sqlite3_exec(db, SQL_ADD1, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 ret=sqlite3_exec(db, SQL_ADD2, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 ret=sqlite3_exec(db, SQL_ADD3, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 ret=sqlite3_exec(db, SQL_ADD4, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 ret=sqlite3_exec(db, SQL_ADD5, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }




 */

 printf("--------------SEL1-----------------\r\n");

 ret=sqlite3_exec(db, SQL_SEL1, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

  printf("--------------SEL2-----------------\r\n");

  ret=sqlite3_exec(db, SQL_SEL2, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

 printf("--------------SEL3-----------------\r\n");

  ret=sqlite3_exec(db, SQL_SEL3, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }


 printf("--------------UPD1-----------------\r\n");

  ret=sqlite3_exec(db, SQL_UPD1, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }


 printf("--------------UPD2-----------------\r\n");

  ret=sqlite3_exec(db, SQL_UPD2, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }


 printf("--------------SEL1-----------------\r\n");

 ret=sqlite3_exec(db, SQL_SEL1, sqlcb, 0, &err);
 if (ret)
 {
  fprintf(stderr, "Error SQL: %sn", err);
  sqlite3_free(err);
 }

}




// 
sqlite3_close(db);



        return 0;
}