#include "evict_set.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>


#define CACHE_SAMPLES 10000
#define CACHE_THRESHOLD 90

static const int32_t pagesize = 4096;

uint64_t probe(void* address){
    register uint64_t start, end;
    start = timer_read();
    access_one_block(address);
    end = timer_read();
    return end - start;
}

uint64_t cache_miss_time (){
    uint64_t result = 0;
    register uint64_t start, end;
    void* dest1, *dest2, *buf;
    for (int i=0; i<CACHE_SAMPLES; i++) {
        buf = malloc(pagesize * 100);
        int page = rand() % 99;
        dest1 = buf + page * pagesize + (rand() % (pagesize / 2));
        access_one_block(dest1);
        dest2 = buf + page * pagesize + pagesize / 2 + (rand() % (pagesize / 2));
        start = timer_read();
        access_one_block(dest2);
        result += timer_read() - start;
        free(buf);
    }    
	result = result/CACHE_SAMPLES;
    printf("cache miss time: %llu\n", result);
    return result;
}

uint64_t cache_hit_time (){
    uint64_t result = 0;
    register uint64_t start, end;
    int page = rand() % 9;
    void* buf = malloc(pagesize * 10);
    void* dest = buf + page * pagesize;
    access_one_block(dest);
    for (int i=0; i<CACHE_SAMPLES; i++) {
        start = timer_read();
        access_one_block(dest);
        result += timer_read() - start;
    }    
    free(buf);
	result = result/CACHE_SAMPLES;
    printf("cache hit time: %llu\n", result);
    return result;
}


// basic configs
int cache_line_size = 64;
int evsets_size = 16;
int evmem_size = 40 * 1024 * 1024;
int rounds = 50;
int max_backtrack = 50;

uint64_t evsets_count = 0;
uint64_t* evsets_pages = NULL;
traverser_t traverser = NULL;
Elem** evsets = NULL;

static void traverse_list(Elem *arg)
{
	Elem *ptr = arg;
	while (ptr && ptr->next && ptr->next->next)
	{
		access_one_block (ptr);
		access_one_block (ptr->next);
		access_one_block (ptr->next->next);
		access_one_block (ptr);
		access_one_block (ptr->next);
		access_one_block (ptr->next->next);
		ptr = ptr->next;
	}
}

void get_threshold () {

}

static void initialize_list(char *start, uint64_t size, uint64_t offset)
{
	uint64_t off = 0;
	Elem* last = NULL;
	for (off = offset; off < size - sizeof(Elem); off += pagesize)
	{
		Elem* cur = (Elem*)(start + off);
		cur->set = -2;
		cur->delta = 0;
		cur->prev = last;
		cur->next = NULL;
		if(last){
		    last->next = cur;
		}
		last = cur;
	}
}

static Elem* initialize_list_with_offset(Elem* reference, uint64_t offset){
    // xor mask to turn offset of reference into offset of 
    uint64_t xor = ((uint64_t)reference & (pagesize - 1)) ^ offset;
    Elem* last = NULL;
    Elem* start = (Elem*)((uint64_t)reference ^ xor);
    while(reference){
        Elem* cur = (Elem*)((uint64_t)reference ^ xor);
        cur->set = -2;
        cur->delta = 0;
        cur->prev = last;
        cur->next = NULL;
        if(last){
            last->next = cur;
        } 
        last = cur;
        reference = reference->next;
    }
    
    return start;
}

int list_length(Elem *ptr)
{
	int l = 0;
	while (ptr)
	{
		l = l + 1;
		ptr = ptr->next;
	}
	return l;
}

void list_concat(Elem **ptr, Elem *chunk)
{
	Elem *tmp = (ptr) ? *ptr : NULL;
	if (!tmp)
	{
		*ptr = chunk;
		return;
	}
	while (tmp->next != NULL)
	{
		tmp = tmp->next;
	}
	tmp->next = chunk;
	if (chunk)
	{
		chunk->prev = tmp;
	}
}

void list_split(Elem *ptr, Elem **chunks, int n)
{
	if (!ptr)
	{
		return;
	}
	int len = list_length (ptr), k = len / n, i = 0, j = 0;
	while (j < n)
	{
		i = 0;
		chunks[j] = ptr;
		if (ptr)
		{
			ptr->prev = NULL;
		}
		while (ptr != NULL && ((++i < k) || (j == n-1)))
		{
			ptr = ptr->next;
		}
		if (ptr)
		{
			ptr = ptr->next;
			if (ptr && ptr->prev) {
				ptr->prev->next = NULL;
			}
		}
		j++;
	}
}

void list_from_chunks(Elem **ptr, Elem **chunks, int avoid, int len)
{
	int next = (avoid + 1) % len;
	if (!(*ptr) || !chunks || !chunks[next])
	{
		return;
	}
	// Disconnect avoided chunk
	Elem *tmp = chunks[avoid];
	if (tmp) {
		tmp->prev = NULL;
	}
	while (tmp && tmp->next != NULL && tmp->next != chunks[next])
	{
		tmp = tmp->next;
	}
	if (tmp)
	{
		tmp->next = NULL;
	}
	// Link rest starting from next
	tmp = *ptr = chunks[next];
	if (tmp)
	{
		tmp->prev = NULL;
	}
	while (next != avoid && chunks[next] != NULL)
	{
		next = (next + 1) % len;
		while (tmp && tmp->next != NULL && tmp->next != chunks[next])
		{
			if (tmp->next)
			{
				tmp->next->prev = tmp;
			}
			tmp = tmp->next;
		}
		if (tmp)
		{
			tmp->next = chunks[next];
		}
		if (chunks[next])
		{
			chunks[next]->prev = tmp;
		}
	}
	if (tmp)
	{
		tmp->next = NULL;
	}
}

void shuffle(int *array, size_t n)
{
	size_t i;
	if (n > 1)
	{
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

int test_set(Elem *ptr, char *victim)
{
        // page walk
	access_one_block (victim);
	access_one_block (victim);

	traverser(ptr);
        
	return probe(victim);
}

int tests_avg(Elem *ptr, char *victim, int rep)
{
	int i = 0, ret =0;
	unsigned int delta = 0;
	unsigned int delta_d = 0;
	for (i=0; i < rep; i++)
	{
		delta = test_set (ptr, victim);
		if (delta < 800) {
			delta_d += delta;
		}
	}
	ret = (float)delta_d / rep;
	#if DEBUG == 1
	printf("ret: %d\n", ret);
	#endif
	return ret > CACHE_THRESHOLD;
}

int gt_eviction(Elem **ptr, Elem **can, char *victim)
{
	// Random chunk selection
	Elem **chunks = (Elem**) calloc (evsets_size + 1, sizeof (Elem*));
	if (!chunks)
	{
	        printf("Could not allocate chunks!\n");
		return 1;
	}
	int *ichunks = (int*) calloc (evsets_size + 1, sizeof (int)), i;
	if (!ichunks)
	{
		printf("Could not allocate ichunks!\n");
		free (chunks);
		return 1;
	}

	int len = list_length(*ptr);
	int cans = 0;

	// Calculate length: h = log(a/(a+1), a/n)
	double sz = (double)evsets_size / len;
	double rate = (double)evsets_size / (evsets_size + 1);
	int h = ceil(log(sz) / log(rate)), l = 0;

	// Backtrack record
	Elem **back = (Elem**) calloc (h * 2, sizeof (Elem*));
	if (!back)
	{
		printf("Could not allocate back (%d, %f, %f, %d)!\n", h, sz, rate, list_length(*ptr));
		free (chunks);
		free (ichunks);
		return 1;
	}

	int repeat = 0;
	do {

		for (i=0; i < evsets_size + 1; i++)
		{
			ichunks[i] = i;
		}
		shuffle (ichunks, evsets_size + 1);

		// Reduce
		while (len > evsets_size)
		{

			list_split (*ptr, chunks, evsets_size + 1);
			int n = 0, ret = 0;

			// Try paths
			do
			{
				list_from_chunks (ptr, chunks, ichunks[n], evsets_size + 1);
				n = n + 1;
				ret = tests_avg (*ptr, victim, rounds);
			}
			while (!ret && (n < evsets_size + 1));

			// If find smaller eviction set remove chunk
			if (ret && n <= evsets_size)
			{
				back[l] = chunks[ichunks[n-1]]; // store ptr to discarded chunk
				cans += list_length (back[l]); // add length of removed chunk
				len = list_length (*ptr);
				l = l + 1; // go to next lvl
			}
			// Else, re-add last removed chunk and try again
			else if (l > 0)
			{
				list_concat (ptr, chunks[ichunks[n-1]]); // recover last case
				l = l - 1;
				cans -= list_length (back[l]);
				list_concat (ptr, back[l]);
				back[l] = NULL;
				len = list_length (*ptr);
				goto mycont;
			}
			else
			{
				list_concat (ptr, chunks[ichunks[n-1]]); // recover last case
				break;
			}
		}

		break;
		mycont:
		;

	} while (l > 0 && repeat++ < max_backtrack);

	// recover discarded elements
	for (i = 0; i < h * 2; i++)
	{
		list_concat (can, back[i]);
	}

	free (chunks);
	free (ichunks);
	free (back);

    int ret = 0;
    
    ret = tests_avg (*ptr, victim, rounds);
    
    if (ret)
	{
		if (len > evsets_size)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}

	return 0;
}

void evset_find(void* addr){

    uint64_t offset = (pagesize - 1) & (uint64_t)addr & ~(cache_line_size - 1);

    // check whether eviction set on same page exists
    int same_page = -1;
    uint64_t page = (uint64_t)addr / pagesize;
    for(int i = 0; i < evsets_count - 1; i++){
        if(evsets_pages[i] == page){
            same_page = i;
            break;
        }
    }
    
    Elem* start;
    
    if(same_page != -1){
        // eviction set on same page (same_page) found!
        start = initialize_list_with_offset(evsets[same_page], offset);
       
    } else {
        // no eviction set on same page found
    
        // map memory
        char* memory_chunk = (char*) mmap (NULL, evmem_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
    
        if(!memory_chunk){
            printf("MMAP FAILED\n");
        }
    
        // initialize list
        start = (Elem*)(void*)(memory_chunk + offset);
        initialize_list(memory_chunk, evmem_size, offset);
    
    
        // reduce list
        Elem* can = NULL;
        int retries = 0;
        while(gt_eviction(&start, &can, addr)){
            // printf("finding eviction set failed!\n");
            if(retries > 20){
                // printf("max retries exceeded!\n");
                break;
            }
            // unmap memory chunk
            munmap(memory_chunk, evmem_size);
            // map new memory chunk
            memory_chunk = (char*) mmap (NULL, evmem_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
            start = (Elem*)(void*)(memory_chunk + offset);
            initialize_list(memory_chunk, evmem_size, offset);
            can = NULL;
            retries ++;
        }
        if(retries < 20){
            // printf("Eviction set found!\n");
        }
    
        while(can){
    	    Elem* next = can->next;
            if(munmap((void*)((uint64_t)can & (~(pagesize-1))), pagesize)){
                printf("munmap failed!\n");
            }
            can = next;
        }
 
    }
    // magic!
    evsets[evsets_count - 1] = start;
    evsets_pages[evsets_count - 1] = page;
}

void evset_init(traverser_t traverser_in) {
    srand(time(NULL));        
    if(!traverser){
        traverser = traverser_in;
    }
    
    if(!evsets){
        evsets = calloc(1, sizeof(Elem*));
    }
    
    if(!evsets_pages){
        evsets_pages = calloc(1, sizeof(char*));
    }    
}

void evsets_flush(cache_ctx_t ctx){
    unsigned int evset = (unsigned int)(uint64_t)ctx;
    traverser(evsets[evset]);
}

void print_evsets(cache_ctx_t ctx){
    unsigned int evset = (unsigned int)(uint64_t)ctx;
	printf("%d\n", evset);
	Elem *ptr = evsets[evset];
	int i = 0;
	while (ptr && ptr->next && ptr->next->next)
	{
		printf("%d, ptr: %p\n", i, ptr);
		i++;
		ptr = ptr->next;
	}
	printf("%d, ptr: %p\n", i, ptr);
	printf("%d, ptr: %p\n", ++i, ptr->next);
}

cache_ctx_t evsets_prepare(char* addr){
    if(cache_line_size < sizeof(Elem)){
        printf("ERROR: Entries are bigger than cache line!\n");
    }
    if(!evsets){
        get_threshold();
        evset_init(traverse_list);
    }
        
    uint64_t time;
    for(uint64_t i = 0; i < (uint64_t)evsets_count; i++){
        access_one_block(addr);
        access_one_block(addr);
        // evict
        traverser(evsets[i]);
        traverser(evsets[i]);

        if(probe(addr) > CACHE_THRESHOLD){
            return (void*)i;
        }
    }
    
    // if not, make space for one (if required) and find one
    uint64_t id = evsets_count ++;
    if(evsets_count % 8 == 1){
        evsets = realloc(evsets, sizeof(Elem*) * (evsets_count + 8));
        evsets_pages = realloc(evsets_pages, sizeof(uint64_t) * (evsets_count + 8));
    }
    evset_find(addr);
    return (void*)id;
    
}

static uint8_t mem1[4096];
static uint8_t mem2[4096];

static inline void flush_with_evsets(void* addr) {
    static cache_ctx_t ctx;
    ctx = evsets_prepare(addr);
    evsets_flush(ctx);
}

// int main() {
//     mem1[0] = 5;
//     uint64_t time_value = probe(mem1);
//     printf("First time = %ld\n", time_value);
//     flush_with_evsets(mem1);
//     time_value = probe(mem1);
//     printf("Second time = %ld\n", time_value);
//     // cache_miss_time();
//     // cache_hit_time();
//     return 0;
// }