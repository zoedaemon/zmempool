#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zMemPool.h"
#include "rdtsc.h"

#include "zMemPool_Test.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */


void print_sequence(void *start, unsigned int size) {
	fprintf(stdout,"\n\nSEQUENCE DATA :\n");
	int i;
	for (i=0; i < size; i++ )
		fprintf(stdout,"%c", ((char *)start)[i]);
	fprintf(stdout,"\n");
}

void print_sequence_pointer(void *start, unsigned int size) {
	fprintf(stdout,"\n\nSEQUENCE DATA :\n");
	int i;
	for (i=0; i < size; i++ )
		fprintf(stdout,"%x:", ((char *)start)[i]);
	fprintf(stdout,"\n");
}

int main(int argc, char *argv[]) {

	//Test inisialisasi
	/*
	char *err = zMemPool_init(10000000, 1);

	if (err != NULL) {
		fprintf(stderr,"Terjadi kesalahan : %s", err);
		exit(1);
	}else
		fprintf(stdout,"INISIALISASI AWAL BERHASIL BROOOHhh : %x\n", zMemPool_get_start_pointer());

	//Test alokasi data 1
	char *copystring = zMemPool_malloc(sizeof(char) * 3);
	void *start = copystring;
	strcpy(copystring, "INI BUDI COYYYYYYYYYYYY"); //akan kepenggal oleh alokasi selanjunya coz zMemPool_malloc cuman 10 slot aza
	fprintf(stdout,"TEST ALOKASI 1 : %s\n\n", copystring);
	zMemPool_print_all_field();


	//Test alokasi data 2 : normal overlapping
	copystring = zMemPool_malloc( sizeof(char) * 10);
	strncpy(copystring, "DAN INI ANI", 10);
	fprintf(stdout,"\n\nTEST ALOKASI 2 : %s\n\n", copystring);
	zMemPool_print_all_field();


	print_sequence( start, strlen("INI BUDI COYY  DAN INI ANI++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++") );


	//Test alokasi data 3 : struct tipe data
	struct data {
		int count;
		char *string;
	};
	struct data *data = zMemPool_malloc( sizeof(struct data) );
	data->count = 9929;
	//NOTE: string di sini sebagai konstanta dan d tunjuk sebagai pointer, jd gak bakalan bisa ditampilkan karena gak ada d memori alokasi
	data->string = "HOolyyyy seeee iittt";
	start = data;
	fprintf(stdout,"\n\nTEST ALOKASI 3 : [%d  %s]\n\n", data->count, data->string);
	zMemPool_print_all_field();


	print_sequence_pointer( start, sizeof(struct data) + 50  );
	print_sequence( start, sizeof(struct data) + 50 );


	//Test alokasi data 4 : overlapping struct

	data = zMemPool_malloc( sizeof(struct data) );
	data->count = 1122345;
	data->string = zMemPool_malloc( strlen("HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt") );
	strncpy(data->string , "HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt", strlen("HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt"));
	//start = data;
	fprintf(stdout,"\n\nTEST ALOKASI 4 : [%d  %s]\n\n", data->count, data->string);
	zMemPool_print_all_field();


	print_sequence_pointer( data, sizeof(struct data) + strlen("HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt")   );
	print_sequence( data, sizeof(struct data)  + strlen("HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt")  );



	//Test alokasi data 5 : overlapping struct 2
	struct data_reverse {
		char *string;
		int count;
		char *reverse;
		struct data *data;
	};
	struct data_reverse *data_reverse = zMemPool_malloc( sizeof(struct data_reverse) );

	data_reverse->count = 9929;

	char *long_string = "HOolyyyy seeee iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt111111111111";
	data_reverse->string = zMemPool_malloc( strlen(long_string) );
	strncpy(data_reverse->string , long_string, strlen(long_string));

	long_string = "iittttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt111111111111  seeee  HOolyyyy";
	data_reverse->reverse = zMemPool_malloc( strlen(long_string) );
	strncpy(data_reverse->reverse, long_string, strlen(long_string));

	//start = data_reverse;
	fprintf(stdout,"\n\nTEST ALOKASI 5 : [%d  %s  ] => %s\n\n", data_reverse->count, data_reverse->string, data_reverse->reverse);

	zMemPool_print_all_field();

	/// \note apakah struct data kemungkinan bisa teroverlap apabila ada alokasi memori acak ???
	/// \note penambahan fungsi zMemPool_malloc_dynamic pakah bermanfaat ?

	data_reverse->data = data;
	fprintf(stdout,"\n\nTEST ALOKASI 5,5 : [%d  %s]\n\n", data_reverse->data->count, data_reverse->data->string);

	//print_sequence_pointer( start, sizeof(struct data_reverse)  );
	//print_sequence( start, sizeof(struct data_reverse)  );


	zMemPool_print_all_mem(10000);

	zMemPool_print_segment_header(start);
	zMemPool_print_segment_header(data);//karena data memiliki data string yg dinamis maka alamat data_reverse akan beda dengan data->next_segment
	zMemPool_print_segment_header(data_reverse);





	//// TEST DUMMYCPY
	char *string_nomempool, *withmempool, *text_to_battle = "I don't know who you, where you come from, I'll find you !!!''";
	unsigned long long a, b;
	int i, length, size=1000000, error_count=0;

	a = rdtsc();
	for	(i=0; i<size; i++) {
		length = strlen(text_to_battle);
		string_nomempool = (char *)malloc(sizeof(char) * length);
		memcpy(string_nomempool, text_to_battle, length);
		free(string_nomempool);
	}
///	fprintf(stdout,"\n\nresult %s\n\n", string_nomempool);//STACK CORRUPT

    b = rdtsc();
	fprintf(stderr,"\n\n\nWAKTU PEMBACAAN = %llu ns ( %lf micro second | %lf second)\n\n",
            b-a, (double)(b-a)/1000000, (double)(b-a)/1000000000);


	a = rdtsc();
	for	(i=0; i<size; i++) {
		length = strlen(text_to_battle);
		withmempool = (char *)zMemPool_malloc(sizeof(char) * length);

		//fprintf(stdout,"FUCKKKKKKKKK: %x %s %x %s => %d\n", withmempool, withmempool, ALLOCATION_FAILED, ALLOCATION_FAILED, strncmp(withmempool, ALLOCATION_FAILED, strlen(ALLOCATION_FAILED)) )	;
		if ( strncmp(withmempool, ALLOCATION_FAILED, strlen(ALLOCATION_FAILED) ) == 0 ){
			error_count = i;
			break;
		}else
			memcpy(withmempool, text_to_battle, length);
	}

    b = rdtsc();
	fprintf(stderr,"\n\n\nWAKTU PEMBACAAN = %llu ns ( %lf micro second | %lf second)\n\n",
            b-a, (double)(b-a)/1000000, (double)(b-a)/1000000000);


	fprintf(stdout,"\n\nerror_count %d\n\n", error_count);
	zMemPool_print_all_field();
*/

return zMemPool_Test_all(NULL);
}

