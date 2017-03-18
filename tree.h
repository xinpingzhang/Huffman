#ifndef __TREE_H
#define __TREE_H

#include <stdio.h>
#include <stdbool.h>

/**
 * This structure represents the frequency of a character encountered in the
 * input text. It holds the frequency and the associated character.
 */
typedef struct Frequency Frequency;
struct Frequency {
    // Frequency value:
    int  v;
    // Character:
    char c;
};


/**
 * A TreeNode represents a node in the tree. It has a unique id (id) that
 * identifies the node, the type (INTERNAL/LEAF), the frequency, the left
 * child, and the right child.
 *
 * The `next` field is used to create a linked list of TreeNode objects.  This
 * allows us to use our TreeNode objects in a linked list.  This is used by
 * the deserialization process to maintain a linked list of tree nodes as the
 * original tree is reconstructed.
 */
typedef struct TreeNode TreeNode;

struct TreeNode {
    Frequency  freq;
    TreeNode  *left;
    TreeNode  *right;
};



/**
 * Returns a TreeNode object.
 */
TreeNode *tree_new ();


/**
 * Deallocates a TreeNode object.
 */
void tree_free (TreeNode *root);


/**
 * Returns the size of the tree.
 */
int tree_size (TreeNode *root);


/**
 * Prints the tree.
 */
void tree_print (TreeNode *root);


/**
 * Serializes the given tree to the file fp.  The starting format of the
 * serialized tree is the character '#' and the ending format is also the
 * character '#'.  Returns a negative value if there was an error writing to
 * the file.
 */
int tree_serialize (TreeNode *root, FILE *fp);

/**
 * Returns a TreeNode object deserialized from the file fp or NULL if an error
 * was encountered in the format.
 */
TreeNode* tree_deserialize (FILE *fp);

bool tree_is_leaf(TreeNode *node);

#endif
