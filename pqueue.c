/********************************************************************
 
 The pqueue module implements a priority queue for the Huffman coding
 algorithm.  It is a very simple implementation of a priority queue.  In
 particular, it simply maintains an array of TreeNode objects that gets sorted
 each time a TreeNode is enqueued (not efficient, but does the job).
 
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pqueue.h"
#include "tree.h"

// The maximum size of the priority queue.  It should never exceed this size
// as it is the maximum number of characters.  In fact, we do not even need to
// have this many entries as we are only dealing with ASCII encoded
// characters (0 through 127).
#define MAXSIZE 256
#define LEFT_CHILD(i) ((i<<1)+1)
#define RIGHT_CHILD(i) ((i<<1)+2)
#define PARENT(i) ((i-1)>>1)

/**
 * The PriorityQueue struct represents a priority queue.  It holds an array of
 * TreeNode's and an int indicating the number of currently used entries in
 * the array.
 */
struct PriorityQueue
{
    TreeNode *queue[MAXSIZE];
    int       count;
};

/**
 * This function is used by the C library qsort routine.  It is similar to a
 * Comparator object in Java.  However, C uses a comparator function.  It
 * compares two TreeNode objects to determine when one is less than the other.
 * The Huffman coding algorithm requires characters to be sorted by their
 * frequencies in ascending order.
 */
static int comparator (const void *x, const void *y)
{
    TreeNode* n1 = *(TreeNode**)x;
    TreeNode* n2 = *(TreeNode**)y;
    int a = n1->freq.v;
    int b = n2->freq.v;
    //first compare by frequency, character is the tie breaker
    if (a < b) return -1;
    else if(a > b) return 1;
    else return n1->freq.c - n2->freq.c;
}

//Bubble element at index i down, until end.
static void bubble_down(TreeNode *arr[], int i, int end)
{
    int left = LEFT_CHILD(i);
    int right = RIGHT_CHILD(i);
    TreeNode *src = arr[i];
    
    while(left < end)
    {
        int min = left;
        if(right < end && comparator(&arr[right], &arr[left]) < 0)
            min = right;
        
        if(comparator(&src, &arr[min]) < 0)
            break;
        
        arr[i] = arr[min];
        i = min;
        left = LEFT_CHILD(i);
        right = left + 1;
    }
    arr[i] = src;
}

//bubble element at index i up.
static void bubble_up(TreeNode *arr[], int i)
{
    TreeNode *src = arr[i];
    int parent = PARENT(i);
    
    while (parent >= 0 && comparator(&arr[parent], &src) > 0)
    {
        arr[i] = arr[parent];
        i = parent;
        parent = PARENT(i);
    }
    arr[i] = src;
}


/**
 * A utility function to sort a PriorityQueue.
 */
//static void sort(PriorityQueue *pq)
//{
//	qsort(pq->queue, pq->count, sizeof(TreeNode *), comparator);
//}


/**
 * Creates a new PriorityQueue object.
 */
PriorityQueue *pqueue_new()
{
    //calloc() automatically zeros the memory for us
    PriorityQueue *pq = (PriorityQueue *)(calloc(1, sizeof(PriorityQueue)));
    return pq;
}


/**
 * Returns 1 if the priority queue is empty; 0 otherwise.
 */
static int is_empty(PriorityQueue *pq)
{
    return pq->count <= 0;
}


/**
 * Deallocates a priority queue.
 */
void pqueue_free(PriorityQueue *pq)
{
    for (int i = 0; i < MAXSIZE; i++)
    {
        if (pq->queue[i] != NULL)
            free(pq->queue[i]);
    }
    free(pq);
}


/**
 * Enqueues the given TreeNode object `n` in the
 * PriorityQueue `pq`.
 */
void pqueue_enqueue (PriorityQueue *pq, TreeNode *n)
{
    //    checkRep(pq);
    // TODO:
    
    // First, check whether there is still room in the PriorityQueue (comparing
    // count against MAXSIZE).  Then, if there is room, store the tree node in
    // the array slot indicated by count, and bubble it up
    if(pq->count >= MAXSIZE)
        return;
    pq->queue[pq->count++] = n;
    bubble_up(pq->queue, pq->count - 1);
    
    //    checkRep(pq);
    return;
}


/**
 * Dequeues a TreeNode object frm the PriorityQueue.  Returns NULL if the
 * PriorityQueue is empty.
 */
TreeNode *pqueue_dequeue (PriorityQueue *pq)
{
    //    checkRep(pq);
    // TODO:
    
    // First, check to see if the priority queue is empty.  If it is, return
    // NULL.  Add overwrite the old at index of by moving the last element
    // to index 0. Then bubble it down.
    if(pq == NULL)
        return NULL;
    if(is_empty(pq))
        return NULL;
    
    void *data = pq->queue[0];
    pq->queue[0] = pq->queue[--pq->count];
    pq->queue[pq->count] = NULL;
    
    bubble_down(pq->queue, 0, pq->count);
    
    //    checkRep(pq);
    return data;
}

/**
 * Prints the priority queue.
 */
void pqueue_print (PriorityQueue *pq)
{
    for (int i = 0; i < pq->count; i++)
    {
        printf("%c", pq->queue[i]->freq.c);
        
        if (i + 1 < pq->count)
            printf(", ");
        else
            printf("\n");
    }
}


/**
 * Returns the size of the priority queue.
 */
int pqueue_size (PriorityQueue *pq)
{
    return pq->count;
}

//Check if the PriorityQueue is internally consistent
//That is, all nodes are smaller than its left child
//and right child

//void checkRep(PriorityQueue *pq)
//{
//    int limit = pq->count;
//    
//    for(int i = 0; i < limit; i ++)
//    {
//        int left = LEFT_CHILD(i);
//        int right = left+1;
//        if(left >= limit)
//        {
//            return;
//        }
//        assert(comparator(&pq->queue[i], &pq->queue[left]) <= 0);
//        if(right < limit)
//        {
//            assert(comparator(&pq->queue[i], &pq->queue[right]) <= 0);
//        }
//    }
//}
