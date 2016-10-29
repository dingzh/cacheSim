/* 周鼎
 * 5140219268
 */
#include "cachelab.h"
#define _POSIX_C_SOURCE 2
#include <stdio.h>      
#include <stdlib.h>
#include <stdbool.h> 
#include <unistd.h>

// struct usde to hold parsed arguments
struct argStruct {
	bool helpFlag;
	bool verboseFlag;
	int  setIndexBits;
	int  associativity;
	int  blockBits;
	char* traceFile;
};


struct instrStruct {
	char type;
	unsigned long long int addr; 
	unsigned int size;
};

// struct for every blork within a set, pred/succ used to form a double
// link list, more specificlly, a queue, to implement LRU.
struct blockStruct {
	bool valid;
	unsigned long long int tag;
	int pred;
	int succ;
};

// struct used to hold info for a single set
struct setStruct {
	unsigned int front;
	unsigned int rear;
	//blocks poinst to the first block within this set
	struct blockStruct *blocks;
};

struct cacheCountStruct
{
	unsigned int hit;
	unsigned int miss;
	unsigned int eviction;
};

struct argStruct parseArguments(int argc, char **argv)
{
	/*
	 * initial the agrument struct, print and abort if args invalid.
	 */
	char c;
	struct argStruct args;
	while ((c = getopt (argc, argv, "hvs:E:b:t:")) != -1)
	switch (c) {
	case 'h':
		args.helpFlag = true;
		break;
	case 'v':
		args.verboseFlag = true;
		break;
	case 's':
		args.setIndexBits = atoi(optarg);
		break;
	case 'E':
		args.associativity = atoi(optarg);
		break;
	case 'b':
		args.blockBits = atoi(optarg);
		break;
	case 't':
		args.traceFile = optarg;
		break;
	default:
		printf("Invalid arguments!\n");
		abort ();
	}
	if(args.setIndexBits == 0) {
		printf("Set Index Bit can't be 0\n");
		abort ();
	}
	if(args.associativity == 0) {
		printf("Associativity can't be 0\n");
		abort ();
	}
	if(args.blockBits == 0) {
		printf("Block bits can't be 0\n");
		abort ();
	}
	if(args.traceFile == NULL) {
		printf("Trace file must be set!\n");
		abort ();
	}
	return args;
}

void setInit(struct setStruct *set, const struct argStruct *args)
{
	int totalBlocks = args->associativity;
	set->front = 0;
	set->rear = totalBlocks - 1;
	set->blocks = calloc(totalBlocks, sizeof(struct blockStruct));
	struct blockStruct *block = set->blocks;
	for(int i = 0; i < totalBlocks; ++i) {
		block->pred = i -1;
		block->succ = i +1;
		++block;
	}
}


struct setStruct * cacheInit(const struct argStruct *args)
{
	unsigned long long int totalSets = 1ULL << args->setIndexBits;
	struct setStruct *indexPtr = malloc(totalSets * sizeof(struct setStruct));
	for(int i = 0; i < totalSets; ++i) {
		setInit(indexPtr + i, args);
	}
	return indexPtr;
}

void cacheFree(struct setStruct *indexPtr, const struct argStruct *args)
{
	for(int i = 0; i < args->setIndexBits; ++i) {
		free(indexPtr[i].blocks);
	}
	free(indexPtr);
}

bool getInstr(FILE *trace_fp, struct instrStruct *instr)
{
	static char buff[32];
	LOOP:
	if(fgets(buff, 32, trace_fp) != NULL) {
		if(buff[0] == 'I')
			goto LOOP;
		sscanf(buff+1,"%c %llx,%u", &instr->type, &instr->addr, &instr->size);
		return true;
	}
	else {
		fclose(trace_fp);
		return false;
	}
}

// LRU implemented with queue, put note back to end, pick front as victim
void markUsed(struct setStruct *set, int pos)
{
	if(pos == set->rear)
		return;
	struct blockStruct *blockArray = set->blocks;
	int pred = blockArray[pos].pred;
	int succ = blockArray[pos].succ;
	blockArray[succ].pred = pred;
	blockArray[pos].pred = set->rear;
	blockArray[set->rear].succ = pos;
	set->rear = pos;
	if(pos == set->front)
		set->front = succ;
	else
		blockArray[pred].succ = succ;

}

void cacheAccess(unsigned long long int index, 
				 unsigned long long int tag,
				 struct setStruct* cache, 
				 const struct argStruct *args, 
				 struct cacheCountStruct *cacheCount)
{
	struct setStruct *set = cache + index;
	struct blockStruct *blockArray = set->blocks;
	int setSize = args->associativity;
	//check if hit
	for(int i = 0; i < setSize; ++i) 
		if(blockArray[i].valid && blockArray[i].tag == tag){
			markUsed(set, i);
			if(args->verboseFlag)
				printf("Hit\t");
			cacheCount->hit += 1;
			return;
		}
	cacheCount->miss += 1;
	if(args->verboseFlag)
		printf("Miss\t");
	struct blockStruct *victim = blockArray + set->front;
	if(victim->valid) {
		cacheCount->eviction += 1;
		printf("Evition\t");
	}
	victim->tag = tag;
	victim->valid = true;
	markUsed(set, set->front);
	return;
}

int main(int argc, char **argv)
{
	const struct argStruct args = parseArguments(argc, argv);
	struct setStruct* const cache = cacheInit(&args);
	
	struct instrStruct instr;
	struct cacheCountStruct cacheCount = {0,0,0};
	FILE* trace_fp = fopen(args.traceFile, "r");
	// file will be closed in getInstr

	unsigned long long int indexFilter = (1ULL<<(args.setIndexBits+args.blockBits)) - 1;
	unsigned long long int index = 0;
	unsigned long long int tag = 0;
	//fetch instruction and excute within this loop
	while(getInstr(trace_fp, &instr)) {
		if(args.verboseFlag)
			printf("%c %llx,%u\t", instr.type, instr.addr, instr.size);

		// bit manipulation to get tag and index
		index = (instr.addr & indexFilter) >> args.blockBits;
		tag = instr.addr >> (args.setIndexBits+args.blockBits);
		printf("index:%llx, tag:%llx\t",index,tag);

		// access cache once, if M, access again.
		cacheAccess(index, tag, cache, &args, &cacheCount);
		if(instr.type == 'M')
			cacheAccess(index, tag, cache, &args, &cacheCount);
		
		if(args.verboseFlag)
			printf("\n");
	}
	// fee allocated memory, file already closed.
	cacheFree(cache, &args);
	printSummary(cacheCount.hit, cacheCount.miss, cacheCount.eviction);
	return 0;
}


