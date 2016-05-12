/**
* Library for fast memory alocation  coz this lib use memory pool that you can initialized at the first before 
* your main logic program run, no need to malloc, calloc or realloc manualy that can slowdown your program, instead
* use this memory pool library for eficiency memory pointing without actual allocation.
* MIT
* 2016 zoe daemon
* \todo: test :
*	1) Malloc, realloc and free in one single loop, 1000 iteration
*	2) Long string manipulation (test string algorithm, ex. porter stemming, levenstein distance, natural language processing, etc)
*	3) Make some data structure from this mempool (ex. pointer of array, pointer of pointer, multi pointer of pointer, etc)
**/

#include <stdlib.h>
#include <stdio.h>
#include "zMemPool.h"


#define ZMEMPOOL_MAX_SIZE 10000000

#define SEGMENT_HEADER_GAP_TO_DATA 0

/// \todo:buat fungsi cek error dengan awalan tanda tilde
#define ALLOCATION_FAILED	"~01: ALLOCATION FAILED"


/**
	@brief 
	Tiap alokasi memori akan membentuk segment baru, #segment adalah struktur data khusus 
	yg menyimpan penunjuk (pointer) ke lokasi memori yg sudah dialokasikan oleh fungsi zMemPool_init 
	sebelum pemanggilan fungsi khusus alokasi memory pool (zMemPool_malloc, zMemPool_calloc dan zMemPool_realloc)
*/
struct segment_header{
	// \todo: perlu cek tipe sistem x86_64 apa 32 bit system 
	void *current_start_pointer;
	size_t reserved_size;
	//DONE tambah "bool freed;" tuk menandai segment sedang free (sudah difree sebelumsnya)
	short freed : 1;
	struct segment *next_segment;
};


/**
freed adalah double linked list yg perlu menshorting listnya berdasarkan #segment_size.
struktur data ini untuk reuse mempool segement yg d free (zMemPool_free), tiap object dari tipe 
data ini memrepresentasikan nilai segment header (indexnya aza cuy)

\todo: malloc biasa apa masukin ke mempool jg ? SOLUSI : tuk sementara malloc biasa za
*/
struct segment_header_freed{
	void *left;
	void *right;
	void *segment_address; ///< pointer ke [struct segment_header *] yg di free
	//DONE tambah "bool freed;" tuk menandai segment sedang free (sudah difree sebelumsnya)
	size_t  segment_size;
};

/**
	@brief 
	menyimpan struktur data utama, yaitu memory pool, segments akan grow atau bertambah jika ada alokasi baru
	\note: selisih address space antara zMemPool dengan start_pointer adalah 56,472 (size_t, Windows)
	\todo: lakukan test case _mempool->segments apabila membesar karena realloc apa akan berpengaruh ke zMemPool lokasi ? sepertinya aman2 za
*/
struct zMemPool {
	void *segments; ///< pointer ke [struct segment_header *] yg aktif (segment_header pertama
	unsigned int n_segment;///< jumlah segment saat ini
	void *start_pointer; ///< pointer ke memori awal alokasi malloc()
	void *current_end_pointer; ///< pointer ke akhir memori dari segments[n-1] (segment yg baru set)
	void *end_pointer;///< pointer ke memori akhir alokasi malloc()
	size_t total_size_t;///< total memori yg ditampung zMemPool 
	int gap; //jarak per segment WARNING : bisa mempengaruhi segment lain	
	void *segment_header_start;
	void *segment_header_end;
};


/**
@brief menggunakan alokasi global, hnya satu object ini yg digunakan untuk keseluruhan program
\todo: cek ukuran memori ZMEMPOOL_MAX_SIZE harus kurang dari memori yg dimiliki sistem, max 70%nya
\todo: harus thread safe, mutex vs semaphore
*/
struct zMemPool *_mempool;




char *zMemPool_init(size_t size, int gap)
{
	if ( (_mempool = (struct zMempool *)malloc( sizeof(struct zMemPool))) == NULL )
		return ALLOCATION_FAILED;
	if ( (_mempool->start_pointer = malloc(size)) == NULL ) {
		free(_mempool);
		return ALLOCATION_FAILED;
	}
	if (gap < 0)
		gap = 1;
	
	_mempool->start_pointer = memset(_mempool->start_pointer, '\0', size);
	
	_mempool->segments = NULL;
	_mempool->total_size_t = size;
	_mempool->gap = gap;
	_mempool->n_segment = 1;
	_mempool->current_end_pointer = _mempool->start_pointer;
	_mempool->end_pointer = _mempool->start_pointer + size;
	_mempool->segment_header_start = NULL;
	_mempool->segment_header_end  = NULL;
return NULL;
}



char *zMemPool_print_all_field(void)
{
	fprintf(stdout,"_mempool : %x\n", _mempool);
	fprintf(stdout,"_mempool->start_pointer : %x\n", _mempool->start_pointer);
	fprintf(stdout,"_mempool->total_size_t  : %d\n", _mempool->total_size_t );
	fprintf(stdout,"_mempool->n : %d\n", _mempool->gap);
	fprintf(stdout,"_mempool->n_segment : %d\n", _mempool->n_segment);
	fprintf(stdout,"_mempool->current_end_pointer : %x\n", _mempool->current_end_pointer);
	fprintf(stdout,"_mempool->end_pointer : %x\n", _mempool->end_pointer);
	
return NULL;
}

char *zMemPool_print_all_mem(int limit)
{
	int i;
	fprintf(stdout,"segments data : \n", _mempool);
	for (i=0; i < limit; i++ )
		fprintf(stdout,"%c", ((char *)_mempool->start_pointer)[i]);
	fprintf(stdout,"\n\n");
	
return NULL;
}


char *zMemPool_print_segment_header(void *start)
{
	struct segment_header *real_start = (struct segment_header *) (start - sizeof(struct segment_header));

	fprintf(stdout,"segment_header : %x\n", real_start);
	fprintf(stdout,"segment_header->current_start_pointer : %x\n", real_start->current_start_pointer);
	fprintf(stdout,"segment_header->reserved_size : %d\n", real_start->reserved_size);
	fprintf(stdout,"segment_header->freed : %d\n", real_start->freed);
	fprintf(stdout,"segment_header->next_segment : %x\n", real_start->next_segment);
return NULL;
}

void *zMemPool_get_start_pointer(void)
{
	return _mempool->start_pointer;
}

void *zMemPool_malloc(size_t size_of)
{	
	/// \todo reuse dari link list
	struct segment_header *pointing = _mempool->current_end_pointer;
	
	
	pointing->current_start_pointer = _mempool->current_end_pointer + sizeof(struct segment_header) + 
									SEGMENT_HEADER_GAP_TO_DATA;
	
	//OLD
	//pointing->current_end_pointer = _mempool->current_end_pointer + sizeof(struct segment_header) + size_of +
	//		((_mempool->n_segment - 1)==0? 0 : _mempool->gap);
	
	pointing->reserved_size = size_of;
	pointing->freed = 0x0;//tandai segment blum pernah di-free
	
	//kalkulasikan next address dengan menghitung address memori yg ditunjuk pointing->current_start_pointer
	//jumlahkan dengan ukuran struktur data [strucct segment_header] dan gap to data dan ukuran data agar pointer
	//pindah ke next address
	//penjumlahan dengan gap d lakukan d sini
	_mempool->current_end_pointer = pointing->current_start_pointer + sizeof(struct segment_header) + 
									SEGMENT_HEADER_GAP_TO_DATA + size_of + 
										((_mempool->n_segment - 1)==0? 0 : _mempool->gap);
	
	//next segment, bisa untuk iterator menelusuri pointer2/segment2 yg aktif
	pointing->next_segment = _mempool->current_end_pointer;
	
	_mempool->n_segment++;
	
	///\todo: realloc jika zMemPool-size exausted

return pointing->current_start_pointer;
}

 

/*
cara 1 :
hapus data pd segment yg d free, kosongkan pointer dan tandai sedang kosong, slot kosong benar2 dihapus dan realloc zMemPool 
increment kan sesuai dengan size segment yg d free. yakin realloc nambah alamat baru di akhir gaknya mengubah alamat pointer lama ????

cara 2 :
buat index hash table untuk segment2 yg sudah d free, jika ada penambahan segment baru tinggal akses hash table dan copy data k segment tsb
tp kendalanya klo size segment baru lebih kecil dr data yg d alokasikan gmana solusinya ??? apa ada fungsi tuk mengkaitkan antar alamat memmory ???


cara 3 :
buat index link list untuk segment2 yg sudah d free, jika ada penambahan segment baru tinggal akses hash table dan copy data k segment tsbasalkan 
ukuran memori yg d inginkan sama ato kurang dr yg d free seeblumnya, jika lebih dari itu buat segment baru 
di current_end_pointer :D 

NOTE: hapus link list head jika proses berhasil merecycle segment yg d hapus
struct LinkListFreedMem {
 	void *ptr;
 	unsigned int reserved_size;//fixed gak bisa diubah lg; 
 	void *next;//node list belakang
 	void *prev;//node list didepan
}
NOTE : apa perlu thread terpisah tuk mengurutkan link list ? ato tambah operasi link list prepend append yg terurut, perlu (TODO) test case
	zMemPool_malloc	dan zMemPool_free bersamaan d dalam loop, jika pake threads apakah aman d dlm loop ?
*/
void __cdecl zMemPool_free(void *_Memory);


//gunakan operasi _NumOfElements * size_t _SizeOfElements
void *__cdecl zMemPool_calloc(size_t _NumOfElements,size_t _SizeOfElements);
  
//GROW: free segment lama dan reservasi blog memory lebih besar d akhir (zMemPool*)->end_pointer
//SHRINK: free bagian segment lama yg berkurang dan segment tetap pada blog memory yg sama
void *__cdecl zMemPool_realloc(void *_Memory,size_t _NewSize);
  
