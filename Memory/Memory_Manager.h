#pragma once
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include"SortedLinkedList.h"
#define smallest_size 8
#define MEM_SIZE 1024
SortedLinkedList** MEM;
//int NO_LinkedLists =  (log2(MEM_SIZE/smallest_size)) +1;;
//int NO_LinkedLists =  8;

void MEM_init();
int get_index(int size);
int allocate_MEM(int process_size);
void deallocate_MEM(int start_pos,int process_size);
void print_freemem ();
void DestroyMEM();