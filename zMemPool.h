#ifndef __zMEMPOOL_H__
#define __zMEMPOOL_H__

char *zMemPool_init(size_t size, int gap);

void *zMemPool_malloc(size_t size_of);

void *zMemPool_get_start_pointer(void);

char *zMemPool_print_all_field(void);

char *zMemPool_print_all_mem(int limit);

char *zMemPool_print_segment_header(void *start);
#endif
