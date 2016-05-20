#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dependency/unity.h"
#include "dependency/unity_internals.h"

#include "zMemPool_Test.h"
#include "zMemPool.h"

#define TEST_CAPTION 	"##########################################################################" \
						" \nTEST CASE : \n"
//#define ALLOC_SIZE	(zMemPool_alloc_size_t)100000000000000 //GAK OVERFLOW d windows with 3 Gb memory fisik :D
//#define ALLOC_SIZE	(zMemPool_alloc_size_t)1000000000000 //DONE ALLOCATION FAILED untuk size ini, di test zMemPool_Test_2 (WINDOWS)
#define ALLOC_SIZE	(zMemPool_alloc_size_t)1000000000///NOTE: safe max size


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
		fprintf(stdout,"<<<<<<<<<< INISIALISASI AWAL BERHASIL BROOOHhh : %p>>>>>>>>>>\n\n", zMemPool_get_start_pointer());

	TEST_ASSERT( err == NULL ); //this one will pass
	//**** END TEST
}


void zMemPool_Test_2(void)
{

	char *TestFunc = "zMemPool_Test_2";
	char *TestDetail = "test zMemPool_malloc dan lakukan copy string k alamat memori mempool yg diset";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST
	char *to_copy = "INI BUDI COYYYYYYYYYYYY";
	char *expected = "INI BUDI COYYYYYYYYYYYY";
	char *copystring = zMemPool_malloc(sizeof(char) * 3);//SIZEnya ?

	strcpy(copystring, to_copy); //akan kepenggal oleh alokasi selanjunya coz zMemPool_malloc cuman 10 slot aza
	fprintf(stdout,"String in MemPool : %s\n\n", copystring);

	if ( strcmp(copystring, expected) == 0) {
            fprintf(stdout,"<<<<<<<<<< Copy data berhasil >>>>>>>>>>\n");
            zMemPool_print_all_field();
            fprintf(stdout,"\n");
	}

	//TEST_ASSERT( strcmp(copystring, "INI BUDI COYYYYYYYYYYYY") == 0 ); //this one will pass
	TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, copystring,
                                    "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
	//**** END TEST
}



void zMemPool_Test_3(void)
{

	char *TestFunc = "zMemPool_Test_3";
	char *TestDetail = "test alokasi dengan fungsi zMemPool_calloc untuk array pointer dan pointer of pointer";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST

	//**BEGIN TEST 1
	int arr_int_static[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int *arr_int_dynamic = zMemPool_calloc(10, sizeof(int));
	int i;

	fprintf(stdout,"init : ");
	for (i=0; i < 10; i++) {
            arr_int_dynamic[i] = i;
            fprintf(stdout,"%d, ", arr_int_dynamic[i]);
            //if (arr_int_dynamic[i] == 8)
            //      arr_int_dynamic[i] = 100;
	}
	fprintf(stdout,"\n");

	fprintf(stdout,"compare : \n");
	for (i=0; i < 10; i++) {
            if ( arr_int_dynamic[i] == arr_int_static[i]) {
                  fprintf(stdout,"%d : ", arr_int_dynamic[i]);
                  fprintf(stdout,"<<<<<<<<<< Copy data berhasil >>>>>>>>>>\n");
            }

            TEST_ASSERT_EQUAL_INT_MESSAGE(arr_int_static[i], arr_int_dynamic[i],
                                          "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
            //TEST_ASSERT( arr_int_dynamic[i] == arr_int_static[i] );
	}
	zMemPool_print_all_field();
	fprintf(stdout,"\n");

      //** END TEST 1



      //**BEGIN TEST 2
      void **arr_ptr_dynamic = zMemPool_calloc(10, sizeof(void *));
      void *to_copy_arr_ptr[] = {"aab", "aba", "baa", "abc", "acb", "cba", "abcd", "abdc", "adbc", "dabc"};
      void *expected_arr_ptr[] = {"aab", "aba", "baa", "abc", "acb", "cba", "abcd", "abdc", "adbc", "dabc"};

	fprintf(stdout,"init : ");
	for (i=0; i < 10; i++) {
            int len = strlen(to_copy_arr_ptr[i]);
            arr_ptr_dynamic[i] = zMemPool_malloc( sizeof(char) * len);
            strcpy(arr_ptr_dynamic[i], to_copy_arr_ptr[i]);
            fprintf(stdout,"%s, ", (char *) arr_ptr_dynamic[i]);
            //if (arr_int_dynamic[i] == 8)
            //      arr_int_dynamic[i] = 100;
	}
	fprintf(stdout,"\n");

	fprintf(stdout,"compare : \n");
	for (i=0; i < 10; i++) {
            if ( strcmp(arr_ptr_dynamic[i], expected_arr_ptr[i]) == 0) {
                  fprintf(stdout,"<<<<<<<<<< Copy data berhasil >>>>>>>>>>\n");
            }
            //TEST_ASSERT( strcmp(copystring, "INI BUDI COYYYYYYYYYYYY") == 0 ); //this one will pass
            TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_arr_ptr[i], arr_ptr_dynamic[i],
                                          "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
            int len = strlen(expected_arr_ptr[i]);
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_arr_ptr[i], arr_ptr_dynamic[i], len,
                                             "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
	}
	zMemPool_print_all_field();
	fprintf(stdout,"\n");
	//** END TEST 2


	//**** END TEST
}




void zMemPool_Test_4(void)
{
	char *TestFunc = "zMemPool_Test_4";
	char *TestDetail = "test alokasi dengan fungsi zMemPool_calloc untuk pointers of struct";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST
	int i, retval=0;
	void *return_ptr = NULL;
	struct test_obj {
	      int str;
	      int uniqid;
	};

      void **arr_ptr_dynamic = zMemPool_calloc(10, sizeof(struct test_obj *));
      void *to_copy_arr_ptr[] = {"akddjfkl", "kskalssad", "doasip[asdop", "poaihskdbas",
                                    "oiasd0-39o", "aslkdjja0-pe", "daposihdausi", "e392ueids[s'", "a7yweuhjsd", "82ioiwasol"};
      int *to_copy_arr_int[] = {2334, 9822, 182, 299, 1100, 99932, 312321, 111111, 444444, 345656};
      void *expected_arr_ptr[] = {"akddjfkl", "kskalssad", "doasip[asdop", "poaihskdbas",
                                    "oiasd0-39o", "aslkdjja0-pe", "daposihdausi", "e392ueids[s'", "a7yweuhjsd", "82ioiwasol"};
      int *expected_arr_int[] = {2334, 9822, 182, 299, 1100, 99932, 312321, 111111, 444444, 345656};

	fprintf(stdout,"init : ");
	for (i=0; i < 10; i++) {
            //alokasi main struct
            arr_ptr_dynamic[i] = zMemPool_malloc( sizeof(struct test_obj) );
            //alokasi main struct->string
            int len = strlen(to_copy_arr_ptr[i]);
            ((struct test_obj *)arr_ptr_dynamic[i])->str = zMemPool_malloc( sizeof(char) * len );

            //copy string
            strcpy(((struct test_obj *)arr_ptr_dynamic[i])->str, to_copy_arr_ptr[i]);
            //copy integer
            ((struct test_obj *)arr_ptr_dynamic[i])->uniqid = to_copy_arr_int[i];
            //show values
            fprintf(stdout,"(%s, ", (char *) ((struct test_obj *)arr_ptr_dynamic[i])->str );
            fprintf(stdout,"%d)", (char *) ((struct test_obj *)arr_ptr_dynamic[i])->uniqid );
            //if (arr_int_dynamic[i] == 8)
            //      arr_int_dynamic[i] = 100;
	}
	fprintf(stdout,"\n");

      //test main pointer, is valid pointer (has valid allocated memory)
      return_ptr = zMemPool_is_allocated(arr_ptr_dynamic, sizeof(struct test_obj), &retval);
      fprintf(stderr,"\nzMemPool_is_allocated: %p (%d)\n", return_ptr, retval);
      TEST_ASSERT_EQUAL_MEMORY_MESSAGE(return_ptr, arr_ptr_dynamic, sizeof(struct test_obj),
                                       "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");

	fprintf(stdout,"compare data : \n");
	for (i=0; i < 10; i++) {
            if ( strcmp(((struct test_obj *)arr_ptr_dynamic[i])->str, expected_arr_ptr[i]) == 0) {
                  fprintf(stdout,"<<<<<<<<<< Copy data berhasil >>>>>>>>>>\n");
            }
            //cek apakah data string sama semuanya
            TEST_ASSERT_EQUAL_STRING_MESSAGE( expected_arr_ptr[i], ((struct test_obj *)arr_ptr_dynamic[i])->str,
                                          "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
            //cek apakah data integer sama
            TEST_ASSERT_EQUAL_INT_MESSAGE( expected_arr_int[i], ((struct test_obj *)arr_ptr_dynamic[i])->uniqid,
                                          "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");

            //test apakah data pointer sama ukurannya
            return_ptr = zMemPool_is_allocated(arr_ptr_dynamic[i], sizeof(struct test_obj), &retval);
            fprintf(stderr,"zMemPool_is_allocated: %p (%d)\n",return_ptr, retval);
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(return_ptr, arr_ptr_dynamic[i], sizeof(struct test_obj),
                                             "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");

	}



	zMemPool_print_all_field();
	fprintf(stdout,"\n");
	//**** END TEST
}

int zMemPool_Test_all(void *parms)
{
	UnityBegin("zMemPool_Test.c");
	RUN_TEST(zMemPool_Test_1);
	RUN_TEST(zMemPool_Test_2);
      RUN_TEST(zMemPool_Test_3);
      RUN_TEST(zMemPool_Test_4);

	/**
	\note : UnityEnd akan mengembalikan nilai kembalian yg sama dengan jumlah test yg gagal
			tampaknya dia pake fungsi exit(<num_failed_test>)
 	*/
	 return (UnityEnd());
}

