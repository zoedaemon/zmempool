/**
* zMemPool, dinamic memory pool with reusable freed memory segments (segment : pointed heap memory pool)
*
* Copyright (c) 2016 Zoe Daemon (Roy A.) All rights reserved.
* Use of this source code is governed by the MIT License that can be
* found in the LICENSE.md file.
*
* @brief Library for fast memory alocation  coz this lib use memory pool that you can initialized at the first before
* your main logic program run, no need to malloc, calloc or realloc manualy that can slowdown your program, instead
* use this memory pool library for eficiency memory pointing without actual allocation.
*
* \todo: test :
*	1) Malloc, realloc and free in one single loop, 1000 iteration
*	2) Long string manipulation (test string algorithm, ex. porter stemming,
            levenstein distance, natural language processing, etc)
*	3) Make some data structure from this mempool (ex. pointer of array,
            pointer of pointer, multi pointer of pointer, etc)
* \todo: Apakah perlu mutex tuk [zMemPool]->current_end_pointer spaya gak corrupt
            pointer memorynya ? apa semaphore ? coba read-write locks !!
		tp kyknya bagus semaphore coz bisa transfer [zMemPool]->current_end_pointer
		ke next thread biar lebih cepat prosesnya tanpa menunggu
		current thread selesai memproses :)
		atau bisa coba ini https://www.cs.cf.ac.uk/Dave/C/node31.html
		sepertinya bagus pake "ret = pthread_cond_timedwait(&cv, &mp, &abstime);"
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "zMemPool.h"

#define __cdecl __attribute__((__cdecl__))

/**
* @brief Tiap alokasi memori akan membentuk segment baru, #segment adalah struktur
* data khusus yg menyimpan penunjuk (pointer) ke lokasi memori yg sudah
* dialokasikan oleh fungsi zMemPool_init sebelum pemanggilan fungsi khusus
* alokasi memory pool (zMemPool_malloc, zMemPool_calloc dan zMemPool_realloc)
**/
struct segment_header{
	// \todo: perlu cek tipe sistem x86_64 apa 32 bit system
	void *current_start_pointer;
	size_t reserved_size;
	//DONE tambah "bool freed;" tuk menandai segment sedang free (sudah difree sebelumsnya)
	unsigned short freed : 1;
	struct segment *next_segment;
};



/**
@brief
freed adalah double linked list yg perlu menshorting listnya berdasarkan #segment_size.
struktur data ini untuk reuse mempool segement yg d free (zMemPool_free), tiap
object dari tipe data ini memrepresentasikan nilai segment header (indexnya aza cuy)
\todo: malloc biasa apa masukin ke mempool jg ? SOLUSI : tuk sementara malloc biasa za
*/
struct segment_header_freed{
	void *left;
	void *right;
	//todo : harus explisit tipe data !!!
	void *segment_address; ///< pointer ke [struct segment_header *] yg di free
	//DONE tambah "bool freed;" tuk menandai segment sedang free (sudah difree sebelumsnya)
	size_t  segment_size;
};



/**
	@brief
	menyimpan struktur data utama, yaitu memory pool, segments akan grow atau
	bertambah jika ada alokasi baru
	\note: selisih address space antara zMemPool dengan start_pointer adalah
	56,472 bytes (size_t, Windows)
	\todo: lakukan test case _mempool->segments apabila membesar karena realloc
	apa akan berpengaruh ke zMemPool lokasi ? sepertinya aman2 za
*/
struct zMemPool {
	void *segments; ///< pointer ke [struct segment_header *] yg aktif (segment_header pertama
	unsigned int n_segment;///< jumlah segment saat ini
	void *start_pointer; ///< pointer ke memori awal alokasi malloc()
	void *current_end_pointer; ///< pointer ke akhir memori dari segments[n-1] (segment yg baru set)
	void *end_pointer;///< pointer ke memori akhir alokasi malloc()
	zMemPool_alloc_size_t total_size_t;///< total memori yg ditampung zMemPool
	size_t current_size;///< total memori yg ditampung zMemPool
	int gap; //jarak per segment WARNING : bisa mempengaruhi segment lain
	void *segment_header_start;
	void *segment_header_end;
};



/**
@brief menggunakan alokasi global, hnya satu object ini yg digunakan untuk
keseluruhan program
\todo: cek ukuran memori ZMEMPOOL_MAX_SIZE harus kurang dari memori yg dimiliki
            sistem, max 70%nya
\todo: harus thread safe, mutex vs semaphore
\todo: lakukan test pada major version, dimana _mempool adalah array of mempool
      jd bisa menggunakan multiple zMemPool untuk ukuran yg besar :), untuk
      mencegah bug:<17.54.13.05.16>
*/
struct zMemPool *_mempool;



/// *****  \todo: DECLARE PRIVATE FUNCTION HERE



/**
 @brief inisialisasi zMemPool, menggunakan calloc supaya alokasi memori langsung
 tersedia supaya pada proses zMemPool_malloc dan zMemPool_calloc tidak ada lagi
 alokasi memori tambahan
 @param size : ukuran maksimal memory pool
 @param gap : ukuran jarak (dalam bytes) antar alokasi. Bentuk umum memori pool
 yang diimplementasikan zMemPool adalah : header1-gap-datablock1-header2-gap-datablock2
 @return NULL jika tidak ada error terjadi, static string jika ada kesalahan, misal
 ALLOCATION_FAILED adalah string "~1: ALLOCATION FAILED", karena static JANGAN
 di-free
*/
char *zMemPool_init(zMemPool_alloc_size_t size, int gap)
{
      //absolute size (jika minus harus dipositifkan)
      //if ( size < 0)
	//	size -= (size + 100);
	if (gap < 0)
            gap = -(gap);

      if (size >= INT_MAX) {
            size = (unsigned long long int)(INT_MAX - 100) & 0xfffffffc;
            fprintf(stderr,"size : %ld\n", size);
      }
      long long int i_test_alloc = 0;
      long long int max_test_alloc = 3000000000UL;

	if ( (_mempool = (struct zMempool *)malloc( sizeof(struct zMemPool))) == NULL ) {
		return ALLOCATION_FAILED;
	}

	/**
		supaya gak lazy evaluation gunakan calloc(1, size) ?
		http://stackoverflow.com/questions/4383059/malloc-memory-questions
	*/
	if ( (_mempool->start_pointer = calloc(1, size)) == NULL ) {

            //insist to alloc :D
            //handle if size can overflow to negative :( need binary Operation...
            //...to delete negative flag !!!
            while ( (_mempool->start_pointer = calloc(1, size) ) == NULL
                        && i_test_alloc < max_test_alloc) {
                  i_test_alloc++;
                  //size = (size + i_test_alloc+3)& 0xfffffffc;
                  size = (unsigned long long int)(size + i_test_alloc + 3) & 0xfffffffc;
                  //fprintf(stderr,"size : %ld\n", size);
		}

            if (i_test_alloc >=  max_test_alloc || _mempool->start_pointer == NULL) {
                  free(_mempool);
                  return ALLOCATION_FAILED;
            }
	}

	fprintf(stderr,"actual size : %ld\n", size);

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
	_mempool->current_size = 0;

return NULL;
}



char *zMemPool_print_all_field(void)
{
	fprintf(stdout,"_mempool : %p\n", _mempool);
	fprintf(stdout,"_mempool->start_pointer : %p\n", _mempool->start_pointer);
	fprintf(stdout,"_mempool->total_size_t  : %ld\n", _mempool->total_size_t );
	fprintf(stdout,"_mempool->gap (segment gap) : %d\n", _mempool->gap);
	fprintf(stdout,"_mempool->n_segment : %d\n", _mempool->n_segment);
	fprintf(stdout,"_mempool->current_end_pointer : %p\n", _mempool->current_end_pointer);
	fprintf(stdout,"_mempool->end_pointer : %p\n", _mempool->end_pointer);
	fprintf(stdout,"_mempool->current_size : %d\n", _mempool->current_size);
	fprintf(stdout,"_mempool->segment_header_start : %p\n", _mempool->segment_header_start);
	fprintf(stdout,"_mempool->segment_header_end : %p\n", _mempool->segment_header_end);
return NULL;
}



/**
      \note: WANING if #limit exceed nonallocated memory it can causing
      segmentation fault
*/
char *zMemPool_print_all_mem(int limit)
{
	int i;
	fprintf(stdout,"segments data : \n");
	for (i=0; i < limit; i++ )
		fprintf(stdout,"%c", ((char *)_mempool->start_pointer)[i]);
	fprintf(stdout,"\n\n");

return NULL;
}



char *zMemPool_print_segment_header(void *start)
{
      //TODO: blum dikurangin dengan SEGMENT_HEADER_GAP_TO_DATA
	struct segment_header *real_start = (struct segment_header *)zMemPool_get_header(start);

	fprintf(stdout,"segment_header : %p\n", real_start);
	fprintf(stdout,"segment_header->current_start_pointer : %p\n", real_start->current_start_pointer);
	fprintf(stdout,"segment_header->reserved_size : %d\n", real_start->reserved_size);
	fprintf(stdout,"segment_header->freed : %d\n", real_start->freed);
	fprintf(stdout,"segment_header->next_segment : %p\n", real_start->next_segment);
return NULL;
}



void *zMemPool_get_start_pointer(void)
{
	return _mempool->start_pointer;
}



void *zMemPool_malloc(size_t size_of)
{
      ///\todo: realloc jika zMemPool-size exausted

      void *end_pointer_reuse_or_not = NULL;
      int found = 0;
      /// release node
      //function helper
      struct segment_header_freed *__release (struct segment_header_freed *current_node)
      {
            if (current_node == NULL)
                  return NULL;
            struct segment_header_freed *old_node_right = current_node->right;
            struct segment_header_freed *old_node_left = current_node->left;

            //mengaitkan ulang node kanan
            if (old_node_right != NULL) {
                  old_node_right->left = old_node_left;
            }else{//terindikasi adalah awal node, wajib update _mempool->segment_header_end
                  _mempool->segment_header_end = current_node->left;
            }
            //mengaitkan ulang node kiri
            if (old_node_left != NULL) {
                  old_node_left->right = old_node_right;
            }else {//terindikasi adalah awal node, wajib update _mempool->segment_header_start
                  _mempool->segment_header_start = current_node->right;
            }

            return current_node;
      }


      struct segment_header * _copy_and_free (struct segment_header_freed *current_node)
      {
            if (current_node == NULL)
                  return NULL;
            //if (*current_node == NULL)
            //      return NULL;

            //keluarkan node yg ditunjuk *current_node dari deretan link list :)
            __release(current_node);//__release(&(*current_node));

            struct segment_header *temp = (current_node)->segment_address;//ambil segment header
            //free(current_node);//bebaskan node link list \todo perlu efisiensi jika ada sisa di nodenya
            found = 1;/// \note released == NULL gak kebaca di gcc windows, is BUG ???
            //if (temp != NULL) //NOTE: sudah ada set flag ini setelah fungsi _reuse
            //      temp->freed = 0;//tandai sudah bisa dipakai kembali
      return temp;
      }

	/// \todo: reuse dari link list
      void *_reuse(void)
      {
            if ( _mempool->segment_header_start == NULL ||
                _mempool->segment_header_end == NULL) {
                        return NULL;
            }

            struct segment_header *released = NULL;
            struct segment_header_freed *old_node_right = _mempool->segment_header_end;
            struct segment_header_freed *old_node_left = _mempool->segment_header_start;

            if (old_node_right->segment_size < size_of ||
                old_node_left->segment_size > size_of)
                  return NULL;

            /// \todo: test 1 node freed tuk reuse
            while (old_node_right->segment_size >= old_node_left->segment_size) {
                  /// \todo: optimize this ==, can use we <= or >= ???
                  if (old_node_right->segment_size == size_of)
                        released = _copy_and_free( old_node_right );
                  else if (old_node_left->segment_size == size_of)
                        released = _copy_and_free( old_node_left);
                  else {
                        /*
                        ambil node di kanannya jika current old_right node
                        sudah kurang dari size_of tuk alokasi
                        */
                        if (old_node_right->segment_size < size_of &&
                            old_node_right->right != NULL)
                              released = _copy_and_free(
                                                (struct segment_header_freed *)
                                                      old_node_right->right
                                                );
                        /*
                        ambil node sekarang jika current old_left node
                        sudah lebih dari size_of tuk alokasi
                        */
                        if (old_node_left->segment_size > size_of)
                              released = _copy_and_free( old_node_right );
                  }

                  if (found == 1)
                        break;

                  old_node_left = old_node_left->right;
                  old_node_right = old_node_right->left;
            }

            if (old_node_left->segment_size >= size_of && found == 0)
                  released = _copy_and_free( old_node_left );

      return (void *)released;
      }

      //cek jika ada reuse segment gunakan segment tsb jika NULL ambil pointer
      //yg d tunjuk segment terakhir
      if ( (end_pointer_reuse_or_not = _reuse()) == NULL  )
            end_pointer_reuse_or_not = _mempool->current_end_pointer;

	struct segment_header *pointing = end_pointer_reuse_or_not;
      int gap = _mempool->gap;// ((_mempool->n_segment - 1)==0? 0 : _mempool->gap);

	//sediakan jarak untuk penempatan data
	/// \bug:<17.54.13.05.16> segmentation fault coz memory out of boundary,...
	//    ...pdahal cuman cek aza tp dah error; (zMemPool_alloc_size_t)1000000000000000
	if ((end_pointer_reuse_or_not + sizeof(struct segment_header) + gap)
		>= _mempool->end_pointer) {
		return ALLOCATION_FAILED;
	}
	pointing->current_start_pointer = end_pointer_reuse_or_not +
                                          sizeof(struct segment_header) + gap;

	//OLD
	//pointing->current_end_pointer = _mempool->current_end_pointer + sizeof(struct segment_header) + size_of +
	//		((_mempool->n_segment - 1)==0? 0 : _mempool->gap);

	pointing->reserved_size = size_of;
	pointing->freed = 0x0;//tandai segment blum pernah di-free

	/*
	kalkulasikan next address dengan menghitung address memori yg ditunjuk
	pointing->current_start_pointer jumlahkan dengan ukuran struktur data
	[strucct segment_header] dan gap to data dan ukuran data agar pointer
	pindah ke next address penjumlahan dengan gap dilakukan di sini
	*/
	void *end = pointing->current_start_pointer + sizeof(struct segment_header) +
                  gap + size_of ;


      //////////// NO SEGMENT REUSE
      if (end_pointer_reuse_or_not == _mempool->current_end_pointer) {
            //Fail to check
            if ((char*)end >= (char*)_mempool->end_pointer) {
                  return ALLOCATION_FAILED;
            }else
                  _mempool->current_end_pointer = end;

            //next segment, bisa untuk iterator menelusuri pointer2/segment2 yg aktif
            pointing->next_segment = _mempool->current_end_pointer;

            _mempool->n_segment++;
            _mempool->current_size += sizeof(struct segment_header) + gap + size_of ;
      }

return pointing->current_start_pointer;
}



//gunakan operasi _NumOfElements * size_t _SizeOfElements
void *__cdecl zMemPool_calloc(size_t num_of_elm,size_t size_of_elm)
{
      //if _NumOfElements == 0 ??? look some linux man page :D
      return zMemPool_malloc( num_of_elm * size_of_elm);
}



//GROW: free segment lama dan reservasi blog memory lebih besar d akhir (zMemPool*)->end_pointer
//SHRINK: free bagian segment lama yg berkurang dan segment tetap pada blog memory yg sama
void *__cdecl zMemPool_realloc(void *_Memory,size_t _NewSize);



//gunakan operasi _NumOfElements * size_t _SizeOfElements
//TODO: private ?
void *zMemPool_get_header(void *ptr)
{
      return (struct segment_header *) (ptr - sizeof(struct segment_header) - _mempool->gap);
}



/**
 @brief cari alamat pointer data_ptr dengan ukuran data size_of_elm apakah
 menunjuk ke salah satu segment yang ada di dalam memory pool zMemPool yang
 teralokasi sebelumnya
 @param data_ptr pointer yang ingin dicek
 @param size_of_elm ukuran data yg disimpan di pointer yg ingin dibandingkan
 @param retval 1 berarti alamat pointer sama, 2 berarti alamat pointer sama dan data di dalamnya sama
 @return nilai alamat yang ditemukan, jika tidak ditemukan akan mengembalikan nilai NULL
*/
void *zMemPool_is_allocated(const void *data_ptr, size_t size_of_elm, int *retval)
{
      struct segment_header *iterator = (struct segment_header *) _mempool->start_pointer;
      struct segment_header *iterator_next = (struct segment_header *) iterator->next_segment;
      struct segment_header *curr_end_pointer = (struct segment_header *) _mempool->current_end_pointer;
      //fprintf(stderr,"\ndata_ptr: %p (%d)", data_ptr, size_of_elm);

      //iterasi dari awal start pointer
      while (iterator < curr_end_pointer) {
            //fprintf(stderr,"\ncurrent_data_ptr: %p", iterator->current_start_pointer);
            if (iterator->current_start_pointer == data_ptr) {
                  if (memcmp(iterator->current_start_pointer, data_ptr, size_of_elm) == 0) {
                        if (retval != NULL)
                              *retval = 2;
                        return iterator->current_start_pointer;
                  }
                  if (retval != NULL)
                        *retval = 1;
                  return iterator->current_start_pointer;
            }
            //zMemPool_print_segment_header(iterator);
            //goto next segment
            iterator = iterator_next;
            iterator_next = (struct segment_header *)iterator->next_segment;
      }

      return NULL;
}



int zMemPool_is_freed(void *ptr)
{
      if (ptr == NULL)
            return INVALID_MEMORY_ADDRESS_INT;
      struct segment_header *segment_header = (struct segment_header *) zMemPool_get_header(ptr);
return segment_header->freed;
}



void *zMemPool_destroy(void)
{
      free(_mempool);
return NULL;
}



/**
 @brief cetak segment yang sudah di free sebelumnya
 @param data_ptr pointer yang ingin dicek
 @param size_of_elm ukuran data yg disimpan di pointer yg ingin dibandingkan
 @param retval 1 berarti alamat pointer sama, 2 berarti alamat pointer sama dan data di dalamnya sama
 @return nilai alamat yang ditemukan, jika tidak ditemukan akan mengembalikan nilai NULL
 \todo: buat pake callback function biar dinamis user bisa print dengan caranya sendiri
 tapi jika callback function sama dengan NULL panggil fungsi default untuk mencetaknya
*/
char *zMemPool_print_free_segments(void)
{
      if (_mempool->segment_header_start == NULL) {
            return NULL_POINTER;
      }
      struct segment_header_freed *iterator = (struct segment_header_freed *) _mempool->segment_header_start;
      struct segment_header_freed *iterator_next = iterator->right;
      fprintf(stderr,"\nFree Nodes : \n");

      //iterasi dari awal start pointer
      while (iterator != NULL) {

            //print some pointer's pointed address
            fprintf(stderr,"\n(segment_header_freed *): %p", iterator);
            fprintf(stderr,"\n(segment_header_freed *)->segment_size: %d", iterator->segment_size);
            fprintf(stderr,"\n(segment_header_freed *)->right: %p",
                    ((struct segment_header *)iterator->right) );
            fprintf(stderr,"\n(segment_header_freed *)->left: %p",
                    ((struct segment_header *)iterator->left) );
            fprintf(stderr,"\n(segment_header_freed *)->segment_address->current_start_pointer : %p\n",
                    ((struct segment_header *)iterator->segment_address)->current_start_pointer);
            //print segments
            zMemPool_print_segment_header(((struct segment_header *)iterator->segment_address)->current_start_pointer);
            //goto next segment
            iterator = iterator_next;
            if (iterator != NULL)
                  iterator_next = iterator->right;
      }

return NULL;
}



/////////////// zMemPool_free implementation
/*
cara 1 :
hapus data pd segment yg d free, kosongkan pointer dan tandai sedang kosong,
slot kosong benar2 dihapus dan realloc zMemPool increment kan sesuai dengan size
segment yg d free. yakin realloc nambah alamat baru di akhir gaknya mengubah
alamat pointer lama ????

cara 2 :
buat index hash table untuk segment2 yg sudah d free, jika ada penambahan segment
baru tinggal akses hash table dan copy data k segment tsb tp kendalanya klo size
segment baru lebih kecil dr data yg d alokasikan gmana solusinya ??? apa ada
fungsi tuk mengkaitkan antar alamat memmory ???

cara 3 :
buat index link list untuk segment2 yg sudah d free, jika ada penambahan segment
baru tinggal akses hash table dan copy data k segment tsbasalkan ukuran memori
yg d inginkan sama ato kurang dr yg d free seeblumnya, jika lebih dari itu buat segment baru
di current_end_pointer :D

NOTE: hapus link list head jika proses berhasil merecycle segment yg d hapus
struct LinkListFreedMem {
 	void *ptr;
 	unsigned int reserved_size;//fixed gak bisa diubah lg;
 	void *next;//node list belakang
 	void *prev;//node list didepan
}
NOTE : apa perlu thread terpisah tuk mengurutkan link list ? ato tambah operasi
      link list prepend append yg terurut, perlu (TODO) test case zMemPool_malloc
      dan zMemPool_free bersamaan d dalam loop, jika pake threads apakah aman d dlm loop ?

\todo : reusable node, gak perlu allocate node jika masih ada node kosong ada
*/
void *__cdecl zMemPool_free(void *memory_ptr)
{
      struct segment_header *segment_header = (struct segment_header *) zMemPool_get_header(memory_ptr);
      struct segment_header_freed *new_node;

      if (memory_ptr == NULL)
            return INVALID_MEMORY_ADDRESS;

      if (segment_header == NULL)
            return INVALID_MEMORY_ADDRESS;

      if ( (new_node = malloc(sizeof(struct segment_header_freed))) == NULL )
            return ALLOCATION_FAILED;

      //tandai segment SEDANG di-free
      segment_header->freed = 0x1;
      //hapus konten lama
      memset(segment_header->current_start_pointer, '\0', segment_header->reserved_size);

      //simpan informasi segment
      new_node->segment_address = segment_header;
      new_node->segment_size = segment_header->reserved_size;

      if ( _mempool->segment_header_start == NULL &&
          _mempool->segment_header_end == NULL) {

            _mempool->segment_header_start = new_node;
            _mempool->segment_header_end = new_node;
            new_node->left = NULL;
            new_node->right = NULL;

      }else {
            /*
            DIRECT ADD TO END (append)
            struct segment_header_freed *old_node = _mempool->segment_header_end;
            old_node->right = new_node;
            new_node->left = old_node;
            new_node->right = NULL;
            _mempool->segment_header_end = new_node;
            */

            /*
            SORTED : start from Start and End (dual iterator)
            todo : need mutex
            */
            //function helper
            void _append (struct segment_header_freed **new_node,
                         struct segment_header_freed **old_node,
                         struct segment_header_freed **prev)
            {
                  fprintf(stdout,"\n\n...APPEND = %d...\n\n", (*new_node)->segment_size);
                  (*old_node)->right = *new_node;
                  (*new_node)->left = *old_node;
                  (*new_node)->right = *prev;
                  if (*prev != NULL)
                        (*prev)->left = *new_node;
            }


            void _prepend (struct segment_header_freed **new_node,
                         struct segment_header_freed **old_node,
                         struct segment_header_freed **prev)
            {
                  fprintf(stdout,"\n\n...PREPEND = %d...\n\n", (*new_node)->segment_size);
                  (*old_node)->left = *new_node;
                  (*new_node)->right = *old_node;
                  (*new_node)->left = *prev;
                  if (*prev != NULL)
                        (*prev)->right = *new_node;
            }


            int success = 0;
            struct segment_header_freed *old_node_right = _mempool->segment_header_end;
            struct segment_header_freed *old_node_left = _mempool->segment_header_start;
            struct segment_header_freed *old_node_right_prev = old_node_right->right;
            struct segment_header_freed *old_node_left_prev = old_node_right->left;

            //TODO : pass by reference tuk mencegah duplikasi statement panjang di bawah ini
            //int do_node_manipulation (void)
            //{
            //}

            while (old_node_right->segment_size >= old_node_left->segment_size) {

                  if (new_node->segment_size >= old_node_right->segment_size) {
                        //DO APPEND
                        _append (&new_node, &old_node_right, &old_node_right_prev);
                        if (old_node_right == _mempool->segment_header_end)
                              _mempool->segment_header_end = new_node;//pointing to new end
                        success = 1;
                        break;
                  }else {
                        old_node_right_prev = old_node_right;
                        old_node_right = old_node_right->left; //move to the left
                  }

                  if (new_node->segment_size <= old_node_left->segment_size) {
                        //DO PREPEND
                        _prepend (&new_node, &old_node_left, &old_node_left_prev);
                        if (old_node_left == _mempool->segment_header_start)
                              _mempool->segment_header_start = new_node;//pointing to new end
                        success = 1;
                        break;
                  }else {
                        old_node_left_prev = old_node_left;
                        old_node_left = old_node_left->right; //move to the right
                  }

            }


            /**
            \todo:test bagian ini cek node-node keluarannya
            */
            if (success != 1) {
                  if (new_node->segment_size >= old_node_right->segment_size) {
                        //DO APPEND
                        _append (&new_node, &old_node_right, &old_node_right_prev);
                        if (old_node_right == _mempool->segment_header_end)
                              _mempool->segment_header_end = new_node;//pointing to new end
                  }else {
                        old_node_right_prev = old_node_right;
                        old_node_right = old_node_right->left; //move to the left
                  }

                  if (new_node->segment_size <= old_node_left->segment_size) {
                        //DO PREPEND
                        _prepend (&new_node, &old_node_left, &old_node_left_prev);
                        if (old_node_left == _mempool->segment_header_start)
                              _mempool->segment_header_start = new_node;//pointing to new end
                  }else {
                        old_node_left_prev = old_node_left;
                        old_node_left = old_node_left->right; //move to the right
                  }
            }
      }
return NULL;
}
