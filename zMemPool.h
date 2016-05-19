#ifndef __zMEMPOOL_H__
#define __zMEMPOOL_H__



#define ZMEMPOOL_MAX_SIZE 10000000

#define SEGMENT_HEADER_GAP_TO_DATA 0

/// \todo:buat fungsi cek error dengan awalan tanda tilde, UPDATE : pake fungsi Throw aza  dr unity test framework
#define ALLOCATION_FAILED	"~01: ALLOCATION FAILED"


/**
\todo: max-nya adalah sepanjang2nya nilai yg bisa d tampung d tipe data ini, klo memori mencukupi tetap teralokasi,
		tp kita tidak tau apakah sudah max karena nilai size otomatis di set d ukuran maksimal tipe data yg digunakan
		Misal Memori sistem 32 Gb, qt set ukuran ZMEMPOOL_MAX_SIZE=100000000000000 akan dibulatkan jadi 27644723211
		*) THIS NEED VALIDATION :
*/
#define zMemPool_alloc_size_t 	unsigned long long int


char *zMemPool_init(zMemPool_alloc_size_t size, int gap);

void *zMemPool_malloc(size_t size_of);

void *zMemPool_calloc(size_t _NumOfElements,size_t _SizeOfElements);

void zMemPool_free(void *_Memory);

void *zMemPool_get_start_pointer(void);

char *zMemPool_print_all_field(void);

char *zMemPool_print_all_mem(int limit);

char *zMemPool_print_segment_header(void *start);
#endif
