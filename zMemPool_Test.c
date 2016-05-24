#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dependency/unity.h"
#include "dependency/unity_internals.h"

#include "zMemPool_Test.h"
#include "zMemPool.h"

#define TEST_CAPTION 	"##########################################################################" \
						" \nTEST CASE : \n"

/// \bug:perlu realloc jika max size sudah tercapai !!!
/*
PROPOSED :
      ubah object zMemPool utama "struct zMemPool *_mempool;" jadi pointer to pointer
      "struct zMemPool **_mempool;" hal ini bertujuan jika max size sudah tercapai
      maka index bisa naik ke slot berikutnya yang masih kosong. Efeknya apakah
      semua logic yg ada di semua fungsi harus berubah ? kyknya cuman perlu index
      khusus untuk menunjukkan _mempool mana yang sedang aktif (_mempool[i]) dan
      gunakan flags deprecated tuk fungsi lama yang menggunakan single _mempool
      (misalnya zMemPool_malloc_deprecated)
TODO :
      lakukan perubahan ini untuk versi major 1.0.0, untuk pengembangan versi 0.*.*
      dan diatasnya (versi 0.1.0 < 1.0.0) bisa tetap terus dilanjutkan parallel
      dengan pengembangan versi 1.*.*
*/
//#define ALLOC_SIZE	(zMemPool_alloc_size_t)1000


//#define ALLOC_SIZE	(zMemPool_alloc_size_t)100000000000000 //GAK OVERFLOW d windows with 3 Gb memory fisik :D
//#define ALLOC_SIZE	(zMemPool_alloc_size_t)1000000000000000 //bug:<17.54.13.05.16> harus dihapus pada commit selanjutnya
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
	      char *str;
	      int uniqid;
	};

      void **arr_ptr_dynamic = zMemPool_calloc(10, sizeof(struct test_obj *));
      void *to_copy_arr_ptr[] = {"akddjfkl", "kskalssad", "doasip[asdop", "poaihskdbas",
                                    "oiasd0-39o", "aslkdjja0-pe", "daposihdausi", "e392ueids[s'", "a7yweuhjsd", "82ioiwasol"};
      int to_copy_arr_int[] = {2334, 9822, 182, 299, 1100, 99932, 312321, 111111, 444444, 345656};
      void *expected_arr_ptr[] = {"akddjfkl", "kskalssad", "doasip[asdop", "poaihskdbas",
                                    "oiasd0-39o", "aslkdjja0-pe", "daposihdausi", "e392ueids[s'", "a7yweuhjsd", "82ioiwasol"};
      int expected_arr_int[] = {2334, 9822, 182, 299, 1100, 99932, 312321, 111111, 444444, 345656};

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
            fprintf(stdout,"%d)", ((struct test_obj *)arr_ptr_dynamic[i])->uniqid );
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




void zMemPool_Test_5(void)
{
	char *TestFunc = "zMemPool_Test_5";
	char *TestDetail = "test dealokasi dengan fungsi zMemPool_free :D";
	fprintf(stdout,"\n\n%s%s :> %s \nPROCESSING...\n\n", TEST_CAPTION, TestFunc, TestDetail);

	//****BEGIN TEST
	int i, j, max_data = 10, max_selectif = 6;
      void **arr_ptr_dynamic = zMemPool_calloc(10, sizeof(void *));
      void *to_copy_arr_ptr[] = {"1234", "123456", "123", "12345678",
                              "1", "123", "1234567890", "123456789012345", "12", "123456"};
      void *expected_arr_ptr[] = {"1234", "123456", "123", "12345678",
                              "1", "123", "1234567890", "123456789012345", "12", "123456"};

      /**
      \todo: perlu pengecekan apakah data yang di-free terurut node2nya (automatic sort n check)
      */
      //start from i+1 (i==1) TODO : cek memset berhasil tuk menset nilai null tuk slot
      //    dengan index berikut ini (untuk memeriksa apakah fungsi zMemPool_free berhasil)
      //int index_arr_to_free[] = {2, 6, 7, 9, 10, 1, 3, 5, 4, 8};
      int index_arr_to_free[] = {2, 6, 7, 9, 10, 8};

      //data pengganti yang udah d free sebelumnya dan diisikan lg dengan zMemPool_malloc
      //RESULT : harus semua arr_replace mengisi kembali slot array yang di-free
      void *arr_replace[] = {"abc", "abcdef", "abcdefghij", "ab", "abcdef", "abcdefgh"};

	fprintf(stdout,"\ninit : ");
	for (i=0; i < max_data; i++) {
            int len = strlen(to_copy_arr_ptr[i]);
            arr_ptr_dynamic[i] = zMemPool_malloc( sizeof(char) * len);
            strcpy(arr_ptr_dynamic[i], to_copy_arr_ptr[i]);
            fprintf(stdout,"%s, ", (char *) arr_ptr_dynamic[i]);
            //zMemPool_print_segment_header(arr_ptr_dynamic[i]);
	}
	fprintf(stdout,"\n");

	fprintf(stdout,"\nvalidate copied data : \n");
	for (i=0; i < max_data; i++) {
            //TEST_ASSERT( strcmp(copystring, "INI BUDI COYYYYYYYYYYYY") == 0 ); //this one will pass
            TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_arr_ptr[i], arr_ptr_dynamic[i],
                                          "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
            int len = strlen(expected_arr_ptr[i]);
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_arr_ptr[i], arr_ptr_dynamic[i], len,
                                             "XXXXXXXXXX Copy data TIDAK berhasil XXXXXXXXXX\n");
	}
	fprintf(stdout,"....VALID\n\n");
	//zMemPool_print_all_field();

      //char *err = zMemPool_print_free_segments();
      //fprintf(stderr,"\n\nXXXXXXXXXX %s XXXXXXXXXX ", err);

      //FREEING AND DIRECT TEST
      fprintf(stdout,"\nfreeing selectif data : \n");
      for (j=0; j < max_selectif; j++) {
            //print dulu sebelum difree
            fprintf(stdout,"freeing : %s\n", (char *) arr_ptr_dynamic[(index_arr_to_free[j]-1)] );
            //set flag free dan delete data (memset '\0')
            zMemPool_free( arr_ptr_dynamic[(index_arr_to_free[j]-1)] );
            //zMemPool_print_segment_header(arr_ptr_dynamic[(index_arr_to_free[j]-1)]);
//            zMemPool_print_free_segments();

            TEST_ASSERT_EQUAL_INT_MESSAGE(1,
                        zMemPool_is_freed(arr_ptr_dynamic[(index_arr_to_free[j]-1)]),
                        "XXXXXXXXXX flag != 1 XXXXXXXXXX\n");
            //cek pointernya harus NULL jika sudah difree
            //TEST_ASSERT_NULL_MESSAGE( arr_ptr_dynamic[(index_arr_to_free[j]-1)],
            //      "XXXXXXXXXX block data != NULL XXXXXXXXXX\n");
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE("\0",
                  ((char *)arr_ptr_dynamic[(index_arr_to_free[j]-1)]), 1,
                  "XXXXXXXXXX block data != NULL XXXXXXXXXX\n");
      }

      zMemPool_print_free_segments();

      //VERBOSE TEST
      fprintf(stdout,"\n\nvalidate success freed zMemPool segments : \n");
	j = 0;
	for (i=0; i < max_data; i++) {
            if (i == (index_arr_to_free[j]-1) ) {
                  //cek flagnya, harus memiliki nilai kembalian 1
                  TEST_ASSERT_EQUAL_INT_MESSAGE(1,
                        zMemPool_is_freed(arr_ptr_dynamic[i]),
                        "XXXXXXXXXX flag != 1 XXXXXXXXXX\n");
                  //fprintf(stdout,"\n---validate arr_ptr_dynamic[%d] : %d\n", i,
                  //        zMemPool_is_freed(arr_ptr_dynamic[i]));
                  //cek pointernya harus NULL jika sudah difree
                  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE("\0",
                        ((char *)arr_ptr_dynamic[(index_arr_to_free[j]-1)]), 1,
                        "XXXXXXXXXX block data != NULL XXXXXXXXXX\n");
                  j++;
            }

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
      RUN_TEST(zMemPool_Test_5);

	/**
	\note : UnityEnd akan mengembalikan nilai kembalian yg sama dengan jumlah test yg gagal
			tampaknya dia pake fungsi exit(<num_failed_test>)
 	*/
	 return (UnityEnd());
}

