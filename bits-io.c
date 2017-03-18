/********************************************************************

 The bits-io module is used to encode bits into an output file, and decode
 them from an input file. The bits-io module buffers output/input bits into an
 internal byte-sized buffer using a special format to know when it is supposed
 to write or read a full byte to the output or input file. Here are the
 details for both writing and reading bits to a file:

 WRITING BITS

   Because we are using the stdio function fputc to write a byte to an output
   file we are required by its interface to deal with bytes, not bits, when
   writing.  That is, it is not possible to write a single bit to an output
   file in Unix - so, we must buffer the bits that we ultimately want to write
   to the output file into a byte. When the byte is full we can then write it
   to the output file.

   To do this correctly, we invented a byte format such that we know when a
   byte is "full". (It is also possible to use a counter, but we desired a
   self-describing format that works nicely in the case of a partially-used
   last byte, too.)  When a byte is full we can then write it to the output
   file and start with an empty byte. The byte format we chose for this
   assignment begins with the following format:

     B = 11111110

   This bit representation can easily be specified in C using the bitwise
   complement operator on the value 1: ~1.  Since we are using gcc, we used a
   bit literal, however: 0b11111110.  (This is not available in all C
   compilers, so it is good that we used a #define for it :-) ).

   When we write a bit (say 1) to this byte B we write the bit into the least
   significant bit with other bits shifted left by 1. This will result in:

     B = 11111101
                ^
                |---------- bit 1 we just "wrote"

   If we were to then write the bit 0 to B it would result in the following
   byte:

     B = 11111010
                ^
                |---------- bit 0 we just "wrote"
  
   Using this format we use the left-most 0 bit as a delimiter between bits
   not written and bits written:

     B = [bits not written]0[bits written], where [bits not written], if any,
    are all 1 bits

   In this example, we have the following:

     B = 11111 0 10
          NW   D W

   Where NW are the not written bits, D is the delimiter 0, and W are the
   written bits. We would keep writing bits until the byte B is full. A full
   byte would look like this (where the bits a-g represents the bits written):

     B = 0abcdefg

   Thus, if the most significant bit is 0 we know the byte is full and we are
   now prepared to write the byte to the output file using fputc.  After we
   write the byte to the output file we then reset the byte to its
   original "empty buffer" value of 11111110.

   There is one special case, however.  It occurs when we do not have any more
   bits to write but we have not filled our byte buffer.  In this case, we
   simply write the remaining byte to the output file, and let the read
   routine deal with it.  This does not impact writing bytes, but it will
   be important to remember this case when we read bits back in from a
   compressed input file.

 READING BITS

   Because we are using the stdio function fgetc to read bytes from an input
   file, we are required by its interface to deal with bytes and not bits when
   reading (as for writing). In the common case (a byte that was full) we read
   in a byte from the compressed input file with the following format:

   B = 0abcdefg

   Our scheme for reading will be to read bits from the high end of the byte
   and to shift marker bits into the low end.  We will do this in such a way
   that we will know when we have read all the available bits.  In particular,
   the first marker bit we shift in will be a 1 and all subsequent marker bits
   will be 0.

   Again considering the common case, we prepare the newly read byte by
   shifting it left and inserting a 1 into the low bit, giving:

   B = abcdefg1

   We can then read a bit from the high end of the byte (the one marked 'a'
   here) and shift in a 0.  This will give;

   B = bcdefg10

   The next read will obtain 'b' and shift in another 0:

   B = cdefg100

   Subsequent reads cause B to evolve as shown here:

   B = defg1000
   B = efg10000
   B = fg100000
   B = g1000000
   B = 10000000 <-- indicates no more bits -- try next byte

   When we reach the state where B is 10000000, which cannot happen otherwise,
   we know we have read all bits and the next bit read should fetch a new
   byte.

   The partial word case works similarly in terms of what we shift in, namely
   a 1 for the first shift and a 0 for any later shifts.  But before we get to
   the first bit to read, we will have to shift multiple times, namely until
   the high order bit is a 0 (as for the full-byte case), and then once more
   (also as for the full byte case).  Here is an example of a partial byte
   holding only 4 bits that were written.  The byte B as written to the file
   and as read by fgetc is:

   B = 1110abcd

   Before reading bits out, we shift, following the rule of first bit in is 1,
   and the rest are 0, until the high bit is 0:

   B = 110abcd1
   B = 10abcd10
   B = 0abcd100

   Now we shift once more:

   B = abcd1000

   and can read bits out as before, with the same stopping condition:

   B = bcd10000
   B = cd100000
   B = d1000000
   B = 10000000 <-- stopping condition

   Finally, if B has value 10000000 and fgetc returns EOF, we stop.

   A note about end-of-file: fgetc returns EOF on end of file, which is -1.
   This is different from the normal char values of 0 through 255, when
   considered as an int.  (This is why fgetc returns an int, not a char!)
   Normally you could get yourself into trouble reading directly into an
   unsigned char and looking for EOF.  For one thing, if you do 'unsigned char
   c = fgetc(...);' and then try 'c == EOF', the == will always be false.
   This is because 'c' will be in the range 0 through 255 while EOF is -1
   ... even if you read an EOF into 'c'!  The EOF value, when truncated and
   stored into 'c' will be 255 (0b11111111).

   In this case, however, we have designed our bit packing format such that a
   legitimate byte will always contain a 0.  Therefore, you can safely compare
   the value of 'c' against 0b1111111 to test for EOF.  We named that value
   EOF_VALUE.  Note that, as previously mentioned, you should not comapre
   against EOF!  Compare against EOF_VALUE!

 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bits-io.h"
#include "tree.h"

/**
 * This structure is used to maintain the writing/reading of a
 * compressed file.
 */

#define BUF_SIZE (1<<20)
struct BitsIOFile
{
    FILE *fp;            // The output/input file
    int count;           // Number of bytes read/written
    char mode;           // The mode 'w' for write and 'r' for read
    unsigned char byte;  // The byte buffer to hold the bits we are
    // reading/writing
    unsigned int index; //index into the buffer
    unsigned int read;  //number of bytes stored in buffer, if less than buffer size
    unsigned char buf[BUF_SIZE];//The buffer
    int nbits;          //number of bits wrote to current byte, range [0, 8]
};

#define NO_BITS_WRITTEN ((unsigned char)(0xFE))
#define ALL_BITS_READ   ((unsigned char)(0x80))
#define EOF_VALUE       ((unsigned char)(EOF))  // will be 0b11111111

static int fill_buf(BitsIOFile *bfile);
static int flush_buf(BitsIOFile *bfile);

/**
 * Opens a new BitsIOFile. Returns NULL if there is a failure.
 *
 * The `name` is the name of the file.
 * The `mode` is "w" for write and "r" for read.
 */
BitsIOFile *bits_io_open (const char *name, const char *mode)
{
    FILE *fp = fopen(name, mode);
    
    if (fp == NULL)
        return NULL;
    
    char mode_letter = mode[0];
    
    BitsIOFile *bfile = (BitsIOFile*)(calloc(1, sizeof(BitsIOFile)));
    bfile->fp    = fp;
    bfile->count = 0;
    bfile->mode  = mode_letter;
    bfile->byte  = 0;
    
    bfile->nbits = 0;
    bfile->index = 0;
    if(mode_letter == 'r')
    {
        //assume no more bits to read from current byte
        bfile->nbits = 8;
        
        //assume no more bytes to read from current buffer
        //this way the next call to bit_io_read will cause
        //the program to read in data
        bfile->index = BUF_SIZE;
    }
    
    return bfile;
}


/**
 * return the number of bytes read/written so far
 */
int bits_io_num_bytes (BitsIOFile *bfile)
{
    assert(bfile != NULL);
    return bfile->count;
}


static int fill_buf(BitsIOFile *bfile)
{
    int read = (int)fread(bfile->buf, 1, BUF_SIZE, bfile->fp);
    //If we encounter an error, return EOF
    if(read == 0)
        return EOF;
    bfile->read = (int)read;
    
    //reset the pointer to beginning of the buffer
    bfile->index = 0;
    return 0;
}

/**
 * Close the BitsIOFile. Returns EOF if there was an error.
 */
int bits_io_close (BitsIOFile *bfile)
{
    assert(bfile != NULL);
    
    if(bfile->mode == 'w')
    {
        flush_buf(bfile);
        if(bfile->nbits > 0)
        {
            int c = bfile->byte;
            //pad remaining bits with 0
            c = c << (8 - bfile->nbits);
            fputc(c, bfile->fp);
        }
    }
    fclose(bfile->fp);
    free(bfile);
    
    return 0;
}


/**
 * Read a bit from the BitsIOFile.
 * Returns 0 or 1 for a bit read,
 * or EOF (-1) if there are no more bits to read
 */
int bits_io_read_bit (BitsIOFile *bfile)
{
    assert(bfile != NULL);
    
    // TODO:
    
    // First, we need to check whether we need to read a byte from the file.
    // The clue for this case is that `bfile->byte` equals `ALL_BITS_READ`.  If
    // the value read was EOF (that is, EOF_VALUE if you are looking at its
    // value in bfile->byte), then return EOF since there are no more bits to
    // read.
    
    // After reading a new byte successfully, increment the count of bytes read.
    // Then you need to shift the byte value left repeatedly until the high bit
    // is 0, and then shift once more to get rid of that 0.  Remember: when
    // shifting a byte the first time, insert a 1 in the low bit and all later
    // times, shift in a 1.  (We found it easy to have a variable indicating the
    // value to shift in.  Initialize it to 1 after reading a byte and after any
    // shift, set it to 0.)
    
    // If the value on entry was not `ALL_BITS_READ`, or if we have completed
    // the preparations after reading a fresh byte, we can proceed to extract
    // one bit and return it.  The bit to return is the high order bit of the
    // byte.  Perform the shift left operation before returning (shifting in a
    // 0), to leave the byte ready for the next call.
    unsigned char byte = bfile->byte;
    int nbits = bfile->nbits;
    if(nbits >= 8)
    {
        //If we reached the end of the buffer
        if(bfile->index >= bfile->read)
            if(fill_buf(bfile) == EOF)
                return EOF;
        
        //read a byte from buffer
        byte = bfile->buf[bfile->index++];
        bfile->count++;
        nbits = 0;
    }
    nbits++;
    int b = (byte >> 7);
    byte <<= 1;
    bfile->byte = byte;
    bfile->nbits = nbits;
    return b;
}

static int flush_buf(BitsIOFile *bfile)
{
    //if we failed to write all bytes
    if(fwrite(bfile->buf, 1, bfile->index, bfile->fp) < BUF_SIZE)
        return EOF;
    
    //reset the pointer
    bfile->index = 0;
    return 0;
}

/**
 * Writes the given bit (1 or 0) to the BitsIOFile.
 */
int bits_io_write_bit (BitsIOFile *bfile, int bit)
{
    assert(bfile != NULL);
    assert((bit & 1) == bit);
    
    // Write the bit into the byte:
    bfile->byte = (bfile->byte << 1) | bit;
    bfile->nbits++;
    
    // Check if the byte is full and write if it is.
    // A byte is full if its left-most bit is 0:
    if (bfile->nbits >= 8)
    {
        bfile->count++;
        //if we reached the end of the buffer, write buffer to disk
        if(bfile->index >= BUF_SIZE)
            if(flush_buf(bfile) == EOF)
                return EOF;
        
        //write the byte to buffer
        bfile->buf[bfile->index++] = bfile->byte;
        
        // Reset the byte:
        bfile->byte = 0;
        bfile->nbits = 0;
    }
    
    return bit;
}


/**
 * Writes the Huffman tree to the BitsIOFile.
 *
 * We need to write the tree to the file so that we can use it when we
 * decode the compressed file.
 */
int bits_io_write_tree (BitsIOFile *bfile, TreeNode *tree)
{
    // If the mode is not for writing we return -1.
    if (bfile->mode != 'w')
        return -1;
    
    tree_serialize(tree, bfile->fp);
    return tree_size(tree);
}


/**
 * Reads the huffman tree from the BitsIOFile.
 *
 * We need to do this first so we have a tree that will be used to
 * decode the rest of the input.
 */
TreeNode *bits_io_read_tree (BitsIOFile *bfile)
{
    // If the mode is not for writing we return -1.
    if (bfile->mode != 'r')
        return NULL;
    
    return tree_deserialize(bfile->fp);
}

/**
 * Return the size of file specified by filename, in bytes.
 */
uint64_t fsize(const char *filename) {
    struct stat st;
    
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

/**
 * Write an integer offset to outputfile
 */
int write_offset(BitsIOFile *bfile, uint64_t size)
{
    //do nothing if not in write mode
    if(bfile->mode != 'w')
        return EOF;
    
    for(int i = sizeof(uint64_t) - 1; i >= 0; i --)
    {
        //get the ith byte
        unsigned char c = (size >> (i << 3)) & 0xFF;
        if(fputc(c, bfile->fp) == EOF)
            return EOF;
    }
    return 0;
}

/**
 *  Read an integer offset from input file
 */
uint64_t read_offset(BitsIOFile *bfile)
{
    if(bfile->mode != 'r')
        return -1L;
    
    uint64_t size = 0;
    for(int i = 0; i < sizeof(uint64_t); i ++)
    {
        int c = fgetc(bfile->fp);
        if(c == EOF)
            return -1L;
        
        size = (size << 8) | c;
    }
    return size;
}
