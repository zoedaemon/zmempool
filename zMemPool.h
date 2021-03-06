#ifndef __zMEMPOOL_H__
#define __zMEMPOOL_H__


/*
#ifndef UINT_MAX
#define UINT_MAX     (sizeof(unsigned int) * 256 - 1)
#endif

#ifndef ULONG_MAX
#define ULONG_MAX    (sizeof(unsigned long) * 256 - 1)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX    (sizeof(unsigned long long) * 256 - 1)
#endif
*/

#define ZMEMPOOL_MAX_SIZE 10000000

#define SEGMENT_HEADER_GAP_TO_DATA 0

/// \todo:buat fungsi cek error dengan awalan tanda tilde, UPDATE : pake fungsi Throw aza  dr unity test framework
#define ALLOCATION_FAILED	"~-1: ALLOCATION FAILED"
#define ALLOCATION_FAILED_INT	-1
#define INVALID_MEMORY_ADDRESS	"~-2: INVALID MEMORY ADDRESS"
#define INVALID_MEMORY_ADDRESS_INT	-2
#define NULL_POINTER          "~-3: NULL POINTER"

/**
\todo: max-nya adalah sepanjang2nya nilai yg bisa d tampung d tipe data ini, klo memori mencukupi tetap teralokasi,
		tp kita tidak tau apakah sudah max karena nilai size otomatis di set d ukuran maksimal tipe data yg digunakan
		Misal Memori sistem 32 Gb, qt set ukuran ZMEMPOOL_MAX_SIZE=100000000000000 akan dibulatkan jadi 27644723211
		*) THIS NEED VALIDATION :
		TODO : search method for maximal page can allocate for new realloc if
		current size exceed max allocation set in
*/
#define zMemPool_alloc_size_t 	unsigned long long int

char *zMemPool_init(zMemPool_alloc_size_t size, int gap);

void *zMemPool_malloc(size_t size_of);

void *zMemPool_calloc(size_t num_of_elm,size_t size_of_elm);

void *zMemPool_free(void *memory_ptr);

void *zMemPool_get_start_pointer(void);

void *zMemPool_get_header(void *ptr);

void *zMemPool_is_allocated(const void *data_ptr, size_t size_of_elm, int *retval);

int zMemPool_is_freed(void *ptr);

char *zMemPool_print_all_field(void);

char *zMemPool_print_all_mem(int limit);

char *zMemPool_print_segment_header(void *start);

char *zMemPool_print_free_segments(void);

#endif
