#ifndef S2D_OPTIMIZE_H
#define S2D_OPTIMIZE_H

struct s2d_optimized_linkedlist {
    struct s2d_optimized_linkedlist *next;
    u8 envR;
    u8 envG;
    u8 envB;
    u8 envA;
    int glyph;
    uObjMtx mtx;
    uObjMtx *dropshadow;
};
typedef struct s2d_optimized_linkedlist S2DListNode;

struct s2dlist {
	S2DListNode *head;
	S2DListNode *tail;
	int len;
};
typedef struct s2dlist S2DList;

#endif