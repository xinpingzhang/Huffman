/********************************************************************
 
 This represents the main Huffman coding algorithm. The algorithm operates in
 three phases:
 
 (1) Compute Frequencies
 
 This phase works by reading each character from the input and updating
 its frequency in a table of Frequency objects (see tree.h).
 
 (2) TreeNode Creation
 
 After the frequency of each character in the input have been found we
 create a new TreeNode object for each character and add it to a
 priority queue. These initial TreeNode objects do not have child
 nodes. We figure out the children in part (3).
 
 (3) Huffman Tree Construction
 
 The third phase dequeues TreeNode objects from the priority queue until
 the priority queue contains only a single TreeNode object - this
 represents the root of the tree. With each iteration we dequeue the
 next two TreeNode objects L and R, create a new internal TreeNode X
 with its frequency being the addition of the two TreeNode objects we
 dequeued. We then assign the left child of X to be L and right child of
 X to be R. We then enqueue X into the priority queue.
 
 
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "huffman.h"
#include "tree.h"
#include "pqueue.h"

#define NUMBER_OF_CHARS 256


/**
 * The Context object is used to pass information between each of the Huffman
 * phases.  It contains the table of frequencies of the characters and a
 * pointer to the priority queue.
 */
typedef struct Context Context;
struct Context {
    Frequency table[NUMBER_OF_CHARS];
    PriorityQueue *pq;
};


/**
 * (1) Compute Frequencies
 *
 * This phase computes the frequencies of the characters given the FILE
 * pointer fp.
 */
static void compute_freq(FILE *fp, Context *ctx)
{
    // TODO:
    
    // First, we need to initialize the frequency table in the Context
    // object. We can initialize each entry's frequency to 0 and its character
    // to its position in the table (0 for entry 0, 1 for entry 1, etc., up
    // through the all the possible entries.  Next, we iterate over the input to
    // determine the frequencies of each character.  A useful function to grab a
    // character from a file is `fgetc`.  You should look at its man page for
    // more details.  Update the input character's entry in the frequency
    // table by incrementing its frequency.
    Frequency *arr = ctx->table;

    //properly initialize the table
    for(int i = 0; i < NUMBER_OF_CHARS; i ++)
    {
        arr[i].c = i;
        arr[i].v = 0;
    }
    
    //create a buffer on stack!
    unsigned char buf[1024*1024];
    
    //number of bytes read
    size_t read = 0;
    
    //read one chunck at a time
    while((read = fread(buf, 1, sizeof(buf), fp)))
    {
        //increment the corresponding frequency
        for(int i = 0; i < read; i ++)
            arr[buf[i]].v++;
    }
    return;
}


/**
 * (2) TreeNode Creation
 *
 * This phase iterates over the frequency table we constructed in Phase (1)
 * and creates a new TreeNode object for each character found in the frequency
 * table.  Each new TreeNode object is enqueued into the priority queue.
 */
static void create_tree_nodes(Context *ctx)
{
    // TODO:
    
    // First, we need to iterate over the characters we are considering.  If the
    // character was not encountered in the input we skip it as we do not need
    // to worry about encoding that character.  If the frequency of a character
    // is greater than 0, we create a new TreeNode object.  The new TreeNode
    // object must be initialized properly.  In particular, it should be a LEAF
    // node, it should receive the Frequency object in the frequency table, and
    // its left and right children should be NULL.  We worry about constructing
    // the tree in Phase (3).  Lastly, enqueue the new TreeNode in the priority
    // queue.
    Frequency *arr = ctx->table;
    PriorityQueue *pq = ctx->pq;
        
    for(int i = 0; i < NUMBER_OF_CHARS; i ++)
    {
        //positive frequency means we encountered this char in the input file
        if(arr[i].v > 0)
        {
            TreeNode *node = tree_new();
            node->freq = arr[i];
            pqueue_enqueue(pq, node);
        }
    }
    return;
}


/**
 * (3) Huffman Tree Construction
 *
 * This is the third and final phase that constructs the Huffman tree from the
 * priority queue populated in Phase (2).  This function returns the final
 * tree that can be used to encode/decode characters and binary encoding
 * respectively.
 */
static TreeNode *build_tree(Context *ctx)
{
    // TODO:
    
    // You must iterate while the size of the priority queue is greater than 1.
    // For each iteration you must dequeue the next two TreeNode objects L and
    // R.  After you dequeue, you need to compute the sum of their frequencies,
    // F.  Next, create a new TreeNode object N with its type being INTERNAL,
    // its frequency being F, its left child pointing to L, and its right child
    // pointing to R.  Lastly, enqueue N into the priority queue.  After you
    // break out of this loop, your priority queue will have a single TreeNode
    // object which represents the root of the tree.  Dequeue the remaining
    // TreeNode and return it.
    return merge_nodes(ctx->pq);
}

TreeNode *merge_nodes(PriorityQueue *pq)
{
    //Single character with non-zero frequency
    if(pqueue_size(pq) == 1)
    {
        TreeNode *node = tree_new();
        //-1 is guarenteed to be smaller than any other character's frequency
        node->freq.v = -1;
        
        pqueue_enqueue(pq, node);
    }
    
    while(pqueue_size(pq) > 1)
    {
        //Dequeue two nodes with smallest frequency
        TreeNode *l = pqueue_dequeue(pq);
        TreeNode *r = pqueue_dequeue(pq);
        
        //merge them into one node
        TreeNode *parent = tree_new();
        parent->freq.v = l->freq.v + r->freq.v;
        parent->left = l;
        parent->right = r;
        
        //enque the combined node
        pqueue_enqueue(pq, parent);
    }
    
    return pqueue_dequeue(pq);
}

/**
 * Returns a pointer to a TreeNode object or NULL if there is an error.
 *
 * Given a filename this function will use the Huffman coding algorithm to
 * construct a Huffman tree that can be used to encode characters from the
 * input file.
 */
TreeNode* huffman_build_tree(const char *filename)
{
    // Define a Context object:
    Context *ctx = malloc(sizeof(Context));
    
    // Create a priority queue:
    ctx->pq = pqueue_new();
    
    // If the priority queue is NULL there was a problem!
    if (ctx->pq == NULL)
        return NULL;
    
    // (1) Compute the frequencies:
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;
    
    compute_freq(fp, ctx);
    fclose(fp);
    
    // (2) Create the tree nodes:
    create_tree_nodes(ctx);
    
    // (3) Build Huffman tree:
    TreeNode *root = build_tree(ctx);
    
    // Free the priority queue - it is no longer needed.
    free(ctx->pq);
    free(ctx);
    return root;
}


/**
 * Returns the character for the given encoding string or -1 on error.
 *
 * This function will traverse the given tree with the provided encoding
 * string (see table.c for the format of the encoding string). For each "bit"
 * in the encoding string we traverse left if it is a 1 and right if it is a
 * 0. When we reach -1 we should be at a LEAF - we then return the character
 * found at that leaf.
 */
int huffman_find(TreeNode *tree, char *encoding)
{
    TreeNode *t = tree;
    for (char *ch = encoding; *ch; ++ch)
    {
        if (*ch == '1')
        {
            if (t->left == NULL)
                return -1;
            
            t = t->left;
        }else
        {
            if (t->right == NULL)
                return -1;
            
            t = t->right;
        }
    }
    assert (tree_is_leaf(t));
    return t->freq.c;
}
