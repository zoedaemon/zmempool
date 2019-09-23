# zmempool
Memory pool that use naming convention follow the standard stdlib.h (malloc, calloc, realloc & free) 
and dynamic allocation with no more extra allocation (in malloc & calloc, except free and realloc there 
is need little allocation for link list for pointing free memory address in mempool)

my old repo : https://bitbucket.org/zoedaemon/zmempool
