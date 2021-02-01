/*
 * Harbin Institute of Technology - Qiuhao Li
 *
 * Anyone can modify this code and then redistribute it to help others.
 *
 * -std=c99
 */

#include "cachelab.h"
#include <stdio.h>	/* fopen freopen perror */
#include <stdint.h>	/* uintN_t */
#include <unistd.h> /* getopt */
#include <getopt.h> /* getopt -std=c99 POSIX macros defined in <features.h> prevents <unistd.h> from including <getopt.h>*/
#include <stdlib.h> /* atol exit*/
#include <errno.h>  /* errno */

#define false 0
#define true 1

typedef struct
{
    _Bool valid;    /* flag whether this line/block is valid, zero at first*/
    uint64_t tag;   /* identifier to choose line/block */
    uint64_t time_counter;  /* LRU strategy counter, we should evict the block who has the min time_counter, zero at first */
    /* We don't need to simulate the block, since we just requested to count hit/miss/eviction */
}line;
typedef line *entry_of_lines;
typedef entry_of_lines *entry_of_sets;

/*
Data structure:

                                                             +-----+
                                                +-----+   +-->Valid|
                                           +---->line0+---+  +-----+
                                           |    +-----+   |
                         +---------------+ |              |  +---+
                         | set0          | |    +-----+   +-->Tag|
                      +--> entry_of_lines+------>line1|   |  +---+
                      |  +---------------+ |    +-----+   |
                      |                    |              |  +-------+
                      |  +---------------+ |    +-----+   +-->Counter|
                      |  | set1          | +---->line2|      +-------+
                      +--> entry_of_lines| |    +-----+
+--------------+      |  +---------------+ |
| cache0       +------+                    |    +-----+
| entry_of_sets|      |  +---------------+ +---->lineX|
+--------------+      |  | set2          |      +-----+
                      +--> entry_of_lines|
                      |  +---------------+
                      |
                      |  +---------------+
                      |  | setX          |
                      +--> entry_of_lines|
                         +---------------+


*/

typedef struct
{
    int hit;
    int miss;
    int eviction;
}result;

entry_of_sets InitializeCache(uint64_t S, uint64_t E);

result HitMissEviction(entry_of_lines search_line, result Result, uint64_t E, uint64_t tag, _Bool verbose);

result ReadAndTest(FILE *tracefile, entry_of_sets cache, uint64_t S, uint64_t E, uint64_t s, uint64_t b, _Bool verbose);

void RealseMemory(entry_of_sets cache, uint64_t S, uint64_t E);

int main(int argc, char * const argv[])
{
    result Result = {0, 0, 0};
	const char *help_message = "Usage: \"Your complied program\" [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n" \
	                     "<s> <E> <b> should all above zero and below 64.\n" \
                         "Complied with std=c99\n";
    const char *command_options = "hvs:E:b:t:";

    FILE* tracefile = NULL;

    entry_of_sets cache = NULL;

	_Bool verbose = false;	/* flag whether switch to verbose mode, zero for default */
	uint64_t s = 0;	/* number of sets ndex's bits */
	uint64_t b = 0;	/* number of blocks index's bits */
	uint64_t S = 0; /* number of sets */
	uint64_t E = 0;	/* number of lines */

	char ch;    /* command options */

	while((ch = getopt(argc, argv, command_options)) != -1)
	{
		switch(ch)
		{
			case 'h':
			{
				printf("%s", help_message);
				exit(EXIT_SUCCESS);
			}

			case 'v':
			{
				verbose = true;
				break;
			}

			case 's':
			{

				if (atol(optarg) <= 0)	/* We assume that there are at least two sets */
				{
					printf("%s", help_message);
					exit(EXIT_FAILURE);
				}
				s = atol(optarg);
				S = 1 << s;
				break;
			}

			case 'E':
			{
				if (atol(optarg) <= 0)
				{
					printf("%s", help_message);
					exit(EXIT_FAILURE);
				}
				E = atol(optarg);
				break;
			}

			case 'b':
			{
				if (atol(optarg) <= 0)	/* We assume that there are at least two sets */
				{
					printf("%s", help_message);
					exit(EXIT_FAILURE);
				}
				b = atol(optarg);
				break;
			}

			case 't':
			{
				if ((tracefile = fopen(optarg, "r")) == NULL)
				{
					perror("Failed to open tracefile");
					exit(EXIT_FAILURE);
				}
				break;
			}

			default:
			{
				printf("%s", help_message);
				exit(EXIT_FAILURE);
			}
		}
	}

    if (s == 0 || b ==0 || E == 0 || tracefile == NULL)
    {
        printf("%s", help_message);
        exit(EXIT_FAILURE);
    }

    cache = InitializeCache(S, E);
    Result = ReadAndTest(tracefile, cache, S, E, s, b, verbose);
    RealseMemory(cache, S, E);   /* Don't forget this in C/C++, and do not double release which causes security problem */
    //printf("hits:%d misses:%d evictions:%d\n", Result.hit, Result.miss, Result.eviction);
    printSummary(Result.hit, Result.miss, Result.eviction);
	return 0;
}

entry_of_sets InitializeCache(uint64_t S, uint64_t E)
{
    // entry_of_sets cache;
    entry_of_lines *cache;

    /* use calloc instead of malloc to match the default situation we designed */

    if ((cache = calloc(S, sizeof(entry_of_lines))) == NULL) /* initialize the sets */
    {
        perror("Failed to calloc entry_of_sets");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < S; ++i)  /* initialize the lines in set */
    {
        if ((cache[i] = calloc(E, sizeof(line))) == NULL)
        {
            perror("Failed to calloc line in sets");
        }
    }

    return cache;
}

result HitMissEviction(entry_of_lines search_line, result Result, uint64_t E, uint64_t tag, _Bool verbose)
{
    uint64_t oldest_time = UINT64_MAX;
    uint64_t youngest_time = 0;
    uint64_t oldest_block = UINT64_MAX;
    _Bool hit_flag = false;

    for (uint64_t i = 0; i < E; ++ i)
    {
        if (search_line[i].tag == tag && search_line[i].valid) /* hit */
        {
            if (verbose)    printf("hit\n");
            hit_flag = true;
            ++Result.hit;
            ++search_line[i].time_counter; /* update the time counter */
            break;
        }
    }

    if (!hit_flag)  /* miss */
    {
        if (verbose)    printf("miss");
        ++Result.miss;
        uint64_t i;
        for (i = 0; i < E; ++i)    /* search for the oldest modified block (invalid blocks are oldest as we designed) */
        {
            if (search_line[i].time_counter < oldest_time)
            {
                oldest_time = search_line[i].time_counter;
                oldest_block = i;
            }
            if (search_line[i].time_counter > youngest_time)    /* search for the youngest modified block to update the new block's time counter */
            {
                youngest_time = search_line[i].time_counter;
            }
        }

        search_line[oldest_block].time_counter = youngest_time + 1;
        search_line[oldest_block].tag = tag;

        if (search_line[oldest_block].valid) /* It's a valid block, ++eviction */
        {
            if (verbose)    printf(" and eviction\n");
            ++Result.eviction;
        }
        else
        {
            if (verbose)    printf("\n");
            search_line[oldest_block].valid = true;
        }
    }
    return Result;
}

result ReadAndTest(FILE *tracefile, entry_of_sets cache, uint64_t S, uint64_t E, uint64_t s, uint64_t b, _Bool verbose)
{
    result Result = {0, 0, 0};
    char ch;
    uint64_t address;
    while((fscanf(tracefile, " %c %lx%*[^\n]", &ch, &address)) == 2)    /* read instruction and address from tracefile and ignore the size */
                                                                        /* address is represented by hexadecimal, use %lx instead of %lu */
    {
        if (ch == 'I')
        {
            continue; /* we don't care about 'I' */
        }
        else
        {
            uint64_t set_index_mask = (1 << s) - 1;
            uint64_t set_index = (address >> b) & set_index_mask;
            uint64_t tag = (address >> b) >> s;
            entry_of_lines search_line = cache[set_index];


            if (ch == 'L' || ch == 'S') /* load/store can cause at most one cache miss */
            {
                if (verbose)    printf("%c %lx ", ch, address);
                Result = HitMissEviction(search_line, Result, E, tag, verbose);
            }

            else if (ch == 'M') /* data modify (M) is treated as a load followed by a store to the same address.
                                   Hence, an M operation can result in two cache hits, or a miss and a hit plus an possible eviction. */
            {
                if (verbose)    printf("%c %lx ", ch, address);
                Result = HitMissEviction(search_line, Result, E, tag, verbose);  /* load, hit/miss(+eviction) */
                Result = HitMissEviction(search_line, Result, E, tag, verbose);  /* store, must hit */
            }

            else    /* ignore other cases */
            {
                continue;
            }
        }
    }
    return Result;
}

void RealseMemory(entry_of_sets cache, uint64_t S, uint64_t E)
{
    for (uint64_t i = 0; i < S; ++i)
    {
        free(cache[i]);
    }
    free(cache);
}
