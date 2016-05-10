#include <stdlib.h>
#include <stdio.h>
#include "zMemPool.h"


#define ZMEMPOOL_MAX_SIZE 10000000

/// \todo:buat fungsi cek error dengan awalan tanda tilde
#define ALLOCATION_FAILED	"~01: ALLOCATION FAILED"

struct segment{
	// \todo: perlu cek tipe sistem x86_64 apa 32 bit system 
	void *current_start_pointer;
	void *current_end_pointer;
	size_t reserved_size;
	//TODO tambah "bool flag;" tuk menandai segment sedang free (sudah difree sebelumsnya)
};


/**
	\note: selisih address space antara zMemPool dengan start_pointer adalah 56,472 (size_t)
	\todo: lakukan test case _mempool->segments apabila membesar karena realloc apa akan berpengaruh ke zMemPool lokasi ? sepertinya aman2 za
*/
struct zMemPool {
	struct segment **segments;
	unsigned int n_segment;
	void *start_pointer;
	void *current_end_pointer;
	void *end_pointer;
	size_t total_size_t;
	int gap; //jarak per segment WARNING : bisa mempengaruhi segment lain	
};



struct zMemPool *_mempool;




char *zMemPool_init(size_t size, int gap)
{
	if ( (_mempool = (struct zMempool *)malloc( sizeof(struct zMemPool))) == NULL )
		return ALLOCATION_FAILED;
	if ( (_mempool->start_pointer = malloc(size)) == NULL )
		return ALLOCATION_FAILED;
	
	_mempool->start_pointer = memset(_mempool->start_pointer, '\0', size);
	
	if ( (_mempool->segments = (struct segment **)calloc(1, sizeof(struct segment))) == NULL )
		return ALLOCATION_FAILED;
	if (gap < 0)
		gap = 1;
	_mempool->total_size_t = size;
	_mempool->gap = gap;
	_mempool->n_segment = 1;
	_mempool->current_end_pointer = _mempool->start_pointer;
	_mempool->end_pointer = _mempool->start_pointer + size;
return NULL;
}



char *zMemPool_print_all_field(void)
{
	
	fprintf(stdout,"_mempool : %x\n", _mempool);
	fprintf(stdout,"_mempool->start_pointer : %x\n", _mempool->start_pointer);
	fprintf(stdout,"_mempool->segments : %x\n", _mempool->segments);
	fprintf(stdout,"_mempool->total_size_t  : %d\n", _mempool->total_size_t );
	fprintf(stdout,"_mempool->n : %d\n", _mempool->gap);
	fprintf(stdout,"_mempool->n_segment : %d\n", _mempool->n_segment);
	fprintf(stdout,"_mempool->current_end_pointer : %x\n", _mempool->current_end_pointer);
	fprintf(stdout,"_mempool->end_pointer : %x\n", _mempool->end_pointer);
	
return NULL;
}

void *zMemPool_get_start_pointer(void)
{
	return _mempool->start_pointer;
}

void *zMemPool_malloc(size_t size_of)
{
	//if (_mempool == NULL) ??? gak konsisten kyknya
	
	if ( (_mempool->segments = (
				struct segment *)realloc(_mempool->segments, sizeof(struct segment ) *  (_mempool->n_segment+1))) 
		== NULL )
		return (void*)ALLOCATION_FAILED;
	
	if ( (_mempool->segments[_mempool->n_segment - 1] = (struct segment *)malloc(sizeof(struct segment))) == NULL )
		return (void*)ALLOCATION_FAILED;
	
	struct segment *pointing = _mempool->segments[_mempool->n_segment - 1];
	
	pointing->current_start_pointer = _mempool->current_end_pointer + 
			((_mempool->n_segment - 1)==0? 0 : _mempool->gap);
	
	pointing->current_end_pointer = _mempool->current_end_pointer + size_of + 
			((_mempool->n_segment - 1)==0? 0 : _mempool->gap);
	
	pointing->reserved_size = size_of;
	
	_mempool->current_end_pointer = pointing->current_end_pointer;
			
	_mempool->n_segment++;

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
  
