#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dependency/unity.h"
#include "dependency/unity_internals.h"

#include "zMemPool_Test.h"
#include "zMemPool.h"

#define TEST_CAPTION 	"##########################################################################" \
						" \nTEST CASE : \n"
#define ALLOC_SIZE	(zMemPool_alloc_size_t)100000000000000 //GAK OVERFLOW d windows with 3 Gb memory fisik :D

void zMemPool_Test_1(void)
{
	char *TestFunc = "zMemPool_Test_1";
	char *TestDetail = "inisialisasi memory pool zMemPool";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST
	char *err = zMemPool_init(ALLOC_SIZE, 1);

	if (err != NULL) {
		fprintf(stderr,"XXXXXXXXXX Alokasi Gagal : %s XXXXXXXXXX", err);
	}else
		fprintf(stdout,"<<<<<<<<<< INISIALISASI AWAL BERHASIL BROOOHhh : %p>>>>>>>>>>\n", zMemPool_get_start_pointer());

	TEST_ASSERT( err == NULL ); //this one will pass
	//**** END TEST
}


void zMemPool_Test_2(void)
{

	char *TestFunc = "zMemPool_Test_2";
	char *TestDetail = "test zMemPool_malloc dan lakukan copy string k alamat memori mempool yg diset";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST
	char *copystring = zMemPool_malloc(sizeof(char) * 3);//SIZEnya ?
	void *start = copystring;
	strcpy(copystring, "INI BUDI COYYYYYYYYYYYY"); //akan kepenggal oleh alokasi selanjunya coz zMemPool_malloc cuman 10 slot aza
	fprintf(stdout,"String in MemPool : %s\n\n", copystring);
	zMemPool_print_all_field();


	if ( strcmp(copystring, "INI BUDI COYYYYYYYYYYYY") == 0)
		fprintf(stdout,"<<<<<<<<<< Copy data berhasil >>>>>>>>>>\n");
	else
		fprintf(stderr,"XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");

	TEST_ASSERT( strcmp(copystring, "INI BUDI COYYYYYYYYYYYY") == 0 ); //this one will pass
	//**** END TEST
}

int zMemPool_Test_all(void *parms)
{
	UnityBegin("zMemPool_Test.c");
	RUN_TEST(zMemPool_Test_1);
	RUN_TEST(zMemPool_Test_2);

	/**
	\note : UnityEnd akan mengembalikan nilai kembalian yg sama dengan jumlah test yg gagal
			tampaknya dia pake fungsi exit(<num_failed_test>)
 	*/
	 return (UnityEnd());
}

