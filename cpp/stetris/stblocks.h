#ifndef __STBLOCKS_H__
#define __STBLOCKS_H__

// Block table dimensions. The ports spell these Blocks.COUNT, Blocks.SIZE
// and so on, so they are grouped here rather than left as loose macros.
struct StBlocks
{
    // Number of block types
    static const int COUNT = 7;
    // Number of rotations each type has
    static const int ROTATIONS = 4;
    // Width and height of the cell grid a shape is drawn in
    static const int SIZE = 4;
};

// block structure
typedef struct _ST_BLOCK {
    int x_size;
    int y_size;
    int block[StBlocks::ROTATIONS][StBlocks::SIZE][StBlocks::SIZE];
} ST_BLOCK, *PST_BLOCK;

// blocks
extern ST_BLOCK g_StBlocks[StBlocks::COUNT];

#endif
