#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

int created = 0;
int cache_create(int num_entries) {
  if (num_entries >= 2 && num_entries <= 4096 && created ==0)
    {
      cache = calloc(num_entries,sizeof(cache_entry_t));
    if (cache == NULL)
      {  
	printf("Error\n");
	return 0;
      }
    cache_size = num_entries;
    created = 1;
    return 1;
    }
  return -1;
    
}
  
int cache_destroy(void) {
  if (created == 1)
    {
    free(cache); 
    cache = NULL;
    created = 0;
    cache_size = 0;
    return 1;
    }
  return -1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if (created == 0 || cache->valid == false || buf == NULL || disk_num < 0 || disk_num > 16 || block_num < 0 || block_num > 256)
    {
      return -1;
    }
  num_queries += 1;
  for (int i = 0; i < cache_size; ++i) //iterates through cache and then iffound memcopy. 
    {
      if (cache[i].valid == true && cache[i].disk_num == disk_num && cache[i].block_num == block_num)
	{
	  memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE); //not making it to memcopy
	  num_hits += 1;
	  clock += 1;
	  cache[i].access_time = clock;
	  return 1;
	}
    }
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  for (int i = 0; i < cache_size; ++i) //iterates through cache and then iffound memcopy.                                                            
    {
      if (cache[i].valid == true && cache[i].disk_num == disk_num && cache[i].block_num == block_num)
        {
          memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE); //not making it to memcopy                                                                   
          num_hits += 1;
          clock += 1;
          cache[i].access_time = clock;
        }
    }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  if (created == 0 || buf == NULL || disk_num <0 || disk_num > 16 || block_num < 0 || block_num > 256)
    {
      return -1;
    }
  int smallest = 1000000; //just to make sure that the smallest access time is used
  int smallest_index = 0;

  for (int i = 0; i < cache_size; ++i)
    {
      if (cache[i].valid == false) //checks if there is an entry. if false then no entry. if valid is false, then that means there is no entry
	{
	  cache[i].valid = true;
	  cache[i].disk_num = disk_num;
	  cache[i].block_num = block_num;
	  clock += 1;
	  cache[i].access_time = clock;
	  memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
	  return 1;
	}
      else if (cache[i].valid == true && cache[i].disk_num == disk_num && cache[i].block_num == block_num) //checks to see if its already in cache entry in cache
	{
	  cache[i].access_time = clock;
	  memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
	  return -1;
	}
      else //checks if its full with with valid being true and diff block or disk num, so we have to evict an entry
	{//loop thorugh and find smallest  access time. if smaller access time, we take that index and we update the cache
	  if (cache[i].access_time < smallest)
	    {
	      smallest = cache[i].access_time;
	      smallest_index = i;
	    }
	}
    }
  cache[smallest_index].disk_num = disk_num;
  cache[smallest_index].block_num = block_num;
  clock +=1;
  cache[smallest_index].access_time = clock;
  memcpy(cache[smallest_index].block, buf, JBOD_BLOCK_SIZE);
  return 1;
}

bool cache_enabled(void) {
  if (cache_size != 0)
    return true;
  return false;
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
