#ifndef __STBLOCKS_H__
#define __STBLOCKS_H__

#define ST_MAX_BLOCK_CNT 7
#define ST_MAX_BLOCK_ROT 4
#define ST_MAX_BLOCK_X 4
#define ST_MAX_BLOCK_Y 4

// block structure
typedef struct _ST_BLOCK {
    int x_size;
    int y_size;
    int block[ST_MAX_BLOCK_ROT][ST_MAX_BLOCK_Y][ST_MAX_BLOCK_X];
} ST_BLOCK, *PST_BLOCK;

// blocks
extern ST_BLOCK g_StBlocks[ST_MAX_BLOCK_CNT];

#endif
