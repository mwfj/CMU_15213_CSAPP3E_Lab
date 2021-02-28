/**
 * @author Wufangjie Ma
 * @date 2021-1-29
**/ 
#include "cachelab.h"
#include <stdint.h>
#include <stdio.h> // fscanf
#include <stdlib.h> // malloc, free
#include <getopt.h> // getopt_long, getopt_long_only
#include <unistd.h> // getopt
// #include <time.h>
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
// the structure of word address in cache
typedef struct cache_line{
    uint64_t tag; // used for 64 bit
    uint64_t time;
    int valid; // valid bit
    // int block; // block offset(unused in this simulator)
}cache_line_t;

typedef cache_line_t *cache_line_ptr;

// Printing help information
void usage() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag that display trace info.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

// Phase the command line by using getopt()
void phase_command(int argc, char **argv, int *verbose, uint64_t* s, uint64_t* E, uint64_t* b, char **t){
    int option;
    while( (option = getopt(argc,argv,"hvs:E:b:t:"))!=-1 ){
        switch (option)
        {
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 'v':
            *verbose = 1;
            // usage();
            break;
        case 's':
            *s = atoi(optarg);
            if(*s<0){
                printf("Error: Invalid input for <s>, exit program \n \n");
                usage();
                exit(EXIT_FAILURE);
            }
            break;
        case 'E':
            *E = atoi(optarg);
            if(*E<0){
                printf("Error: Invalid input for <E>, exit program \n \n");
                usage();
                exit(EXIT_FAILURE);
            }
            break;
        case 'b':
            *b = atoi(optarg);
            if(*b<0){
                printf("Error: Invalid input for <E>, exit program \n \n");
                usage();
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            *t = optarg;
            break;
        default:
            printf("\n----------------------------------\n");
            printf("Invalid option input!! Try again. \n");
            printf("----------------------------------\n");
            usage();
            exit(EXIT_FAILURE);
        }
    }
}

// Init the cache simulator structure
// Where the cache totally has s sets and E lines in each set
cache_line_ptr* cache_init(uint64_t sets, uint64_t E){
    cache_line_ptr* cache_tmp;
    // Initialize the set
    if( (cache_tmp = calloc(sets,sizeof(cache_line_ptr)))==NULL ){
        perror("Failed to allocate space for calloc\n");
        exit(EXIT_FAILURE);
    }
    // Initialize the lines in each set
    for(int i=0; i<sets;i++){
        if( (cache_tmp[i] =calloc(E,sizeof(cache_line_t))) == NULL){
            perror("Failed to allocate space for lines\n");
            printf("sets number %d\n",i);
            exit(EXIT_FAILURE);
        }
        // Initialize the parameter in cache_line structure
        // Cause Segment Fault, ignore it.
        // for(int j=0; j<E; j++){
        //     cache_tmp[i][j].valid = 0;
        //     cache_tmp[i][j].time = -1;
        //     cache_tmp[i][j].tag = -1;
        // }
    }
    return cache_tmp;
}
// Free Request Heap Space
void relese_space(cache_line_ptr* cache, uint64_t sets){
    for(uint64_t i=0; i<sets; i++)
        free(cache[i]);
    free(cache);
}

// Using LRU strategy to calculate the miss, hit and evictions 
// Find the least recent used block and replace it with newly cache block.
void load_in_cache(int *hit, int *miss, int *evictions, 
                cache_line_ptr search_line,int E, int tag, int verbose){

    // Debug Information, can be deleted
    // printf("Before\n");
    // for(int i=0; i<E;i++){
    //     printf("tag: %lx, time: %lx, valid: %d\n",search_line[i].tag,search_line[i].time,search_line->valid);
    //     search_line[i].tag++;
    //     search_line[i].time++;
    //     search_line[i].valid = 1;
    // }
    
    // To record whether cache is hit
    // int isHit = 0;

    // Initialize time stamp
    uint64_t recent_time = 0;
    uint64_t oldest_time = UINT64_MAX;
    uint64_t oldest_block = UINT64_MAX;

    // Verify whether is hit in the current cache line
    for(uint64_t i=0; i<E;i++){
        if( (search_line[i].tag == tag) && (search_line[i].valid==1) ){
            if(verbose)
                printf("Hit Occured\n");
            // isHit = 1;
            (*hit)++;
            search_line[i].time++;
            return ;
        }
    }
    // Hit Miss
    if(verbose) printf("Miss Occured\n");
    (*miss)++;

    // Find the Least Recent Used Block
    for(uint64_t i=0; i<E;i++){
        if(search_line[i].time < oldest_time){
            oldest_time = search_line[i].time;
            oldest_block = i;
        }
        // find the recent used block
        if(search_line[i].time > recent_time){
            recent_time = search_line[i].time;
        }
    }
    // Replace Block
    search_line[oldest_block].time = recent_time+1;
    search_line[oldest_block].tag = tag;

    // Check whether the target block has been filled
    if(search_line[oldest_block].valid){ // Was an filled block
        if(verbose) printf("Eviction Occured \n");
        (*evictions)++;
    }else{ // The target block was an empty block
        search_line[oldest_block].valid = 1;
    }
}

// phase trace
void read_trace(FILE* file, cache_line_ptr* cache, uint64_t s, 
                uint64_t sets, uint64_t E,uint64_t b, 
                int verbose,int *hit,int *miss, int *evictions)
{
    char flag;
    uint64_t addr;
    int len; 
    // Read the trace file line by line
    while(fscanf(file," %c %lx, %d",&flag,&addr,&len) != EOF){
        if(flag == 'I')
            continue; // skip when flag is 'I'
        // Debug information
        // printf("c:%c,addr:%lx ,len:%d\n",flag,addr,len); 
        /**
         * Structure for block in Cache line:
         *      +-----------+----------+---------------+
         *      + Valid Bit + Tag Bit  +  Cache Block  + 
         *      +-----------+----------+---------------+    
         *
         *
         * Address of word that CPU send to Cache: 64bit 
         *      +-----------+------------+---------------+
         *      + Tag Bit   +  Set Index +  Block Offset +
         *      +-----------+------------+---------------+
        **/ 
        // Extract Set index 
        uint64_t set_index_mask = (1<<s)-1;
        uint64_t set_index = (addr>>b) & set_index_mask;
        uint64_t tag_bit = (addr>>b)>>s;
        cache_line_ptr search_line = cache[set_index];

        // Load or Store will cause at most one cache miss
        // When valid bit is 1 and tag bit is matched, cache hit
        // Cache miss otherwise
        if(flag == 'L' || flag == 'S'){ 
            // Display trace info
            if(verbose)
                printf("Flag: %c, Address: %lx\n",flag,addr);
            load_in_cache(hit,miss,evictions,search_line,E,tag_bit,verbose);
            // Debug Information, can be deleted
            // printf("After:\n");
            // for(int i=0; i<E;i++){
            //     printf("tag: %lx, time: %lx, valid: %d\n",search_line[i].tag,search_line[i].time,search_line->valid);

            // }
        }else if(flag == 'M'){
            /*
                Due to Modify operation involved both load and store
                Thus, a modify operation may make two hit operation or one miss(might plus one eviction)+hit.
                
            */
           if(verbose)  printf("Flag: %c, Address: %lx",flag,addr);
           // Load Operation: one miss or one miss + one eviction
           load_in_cache(hit,miss,evictions,search_line,E,tag_bit,verbose);
           // Store Operation: Must hit
           load_in_cache(hit,miss,evictions,search_line,E,tag_bit,verbose);
        }
    }
}

// Debug function, can be ignored if you don't think it useful.
void printInfo(uint64_t* s, uint64_t* E, uint64_t* b,char** t){
    printf("s: %ld\n",*s);
    printf("E: %ld\n",*E);
    printf("b: %ld\n",*b);
    printf("t: %s\n",*t);
}
int main(int argc, char **argv)
{
    int verbose = 0;
    uint64_t s=0,E=0,b=0;
    char* file_path = NULL;
    int hit=0, miss=0, evictions=0;
    phase_command(argc,argv,&verbose,&s,&E,&b,&file_path);
    // printInfo(&s,&E,&b,&file_path);
    // Init the cache line
    uint64_t sets = 1<<s;
    cache_line_ptr* cache = NULL;

    cache = cache_init(sets,E);
    // Open the file
    FILE* file = fopen(file_path,"r");
    if(!file){
        printf("Error: File Cannot Open in this path: %s! Exit Program\n",file_path);
        exit(EXIT_FAILURE);
    }
    // Read the file
    read_trace(file,cache,s,sets,E,b,verbose,&hit,&miss,&evictions);
    // Print the result
    printSummary(hit, miss, evictions);
    // Close the file after used it
    fclose(file);
    // Release the space
    relese_space(cache,sets);
    return 0;
}
