// int s2d_atoi(char *s, char **s2) {
// 	int ret = 0;
// 	int isNegative = (*s == '-');
// 	if (isNegative) {s++; (*s2)++;}
// 	for (; *s != '\0' && *s != ' ' && *s >= '0' && *s <= '9'; s++) {
// 		ret *= 10;
// 		if (*s >= '0' && *s <= '9')
// 			ret += *s - '0';
// 		else break;
// 		if (!(*(s+1) != '\0' && *(s+1) != ' ' && *(s+1) >= '0' && *(s+1) <= '9')) break;
// 		(*s2)++;
// 	}
// 	if (isNegative) ret *= -1;
// 	return ret;
// }
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct {
  int   A, B, C, D;     /* s15.16 */
  short   X, Y;           /* s10.2  */
  short   BaseScaleX;	/* u5.10  */
  short   BaseScaleY;	/* u5.10  */
} uObjMtx_t;		/* 24 bytes */

typedef union {
  uObjMtx_t	m;
  long long int force_structure_alignment;
} uObjMtx;

#define SCALE      "\x80" // SCALE (an integer percentage; 100 -> 100%, 25 -> 25%, etc.)
#define ROTATE     "\x81" // ROTATE (degrees) // TODO: maybe add axis?
#define TRANSLATE  "\x82" // TRANSLATE (x) (y)
#define COLOR      "\x83" // COLOR (r) (g) (b) (a)
#define DROPSHADOW "\x84" // DROPSHADOW (no args)
#define BACKGROUND "\x85" // BACKGROUND (w) (h) (alpha)
#define SEPARATOR  "\x86"
#define RESET      "\x87"

#define CH_SCALE      '\x80'
#define CH_ROT        '\x81'
#define CH_TRANSLATE  '\x82'
#define CH_COLOR      '\x83'
#define CH_DROPSHADOW '\x84'
#define CH_BACKGROUND '\x85'
#define CH_SEPARATOR  '\x86'
#define CH_RESET      '\x87'

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2


#define TAB_WIDTH_H TEX_WIDTH * 2
#define TAB_WIDTH_V TEX_WIDTH / 2

#define CH_GET_NEXT(x) (*(++x))
#define CH_SKIP(x) ((++x))

#define	FTOFIX32(x)	(long)((x) * (float)0x00010000)
#define	FIX32TOF(x)	((float)(x) * (1.0f / (float)0x00010000))

#define ftoq FTOFIX32
#define qtof FIX32TOF

void mat2_dst_mul(uObjMtx *dst, uObjMtx *m1, uObjMtx *m2) {
	dst->m.A = ftoq((qtof(m1->m.A) * qtof(m2->m.A)) + (qtof(m1->m.B) * qtof(m2->m.C)));
	dst->m.B = ftoq((qtof(m1->m.A) * qtof(m2->m.B)) + (qtof(m1->m.B) * qtof(m2->m.D)));
	dst->m.C = ftoq((qtof(m1->m.C) * qtof(m2->m.A)) + (qtof(m1->m.D) * qtof(m2->m.C)));
	dst->m.D = ftoq((qtof(m1->m.C) * qtof(m2->m.B)) + (qtof(m1->m.D) * qtof(m2->m.D)));
}

void mat2_dst_add(uObjMtx *dst, uObjMtx *m1, uObjMtx *m2) {
	dst->m.A = (m1->m.A + m2->m.A);
	dst->m.B = (m1->m.B + m2->m.B);
	dst->m.C = (m1->m.C + m2->m.C);
	dst->m.D = (m1->m.D + m2->m.D);
}

void mat2_copy(uObjMtx *dst, uObjMtx *src) {
	dst->m.A = src->m.A;
	dst->m.B = src->m.B;
	dst->m.C = src->m.C;
	dst->m.D = src->m.D;
	dst->m.X = src->m.X;
	dst->m.Y = src->m.Y;
}

void mat2_ident(uObjMtx *dst, float scale) {
	dst->m.A = (1 << 16);
	dst->m.B = 0;
	dst->m.C = 0;
	dst->m.D = (1 << 16);

	dst->m.X = 0;
	dst->m.Y = 0;
}
// cos -sin sin cos
void mat2_rotate(uObjMtx *dst, float degrees) {
	dst->m.A = ftoq(cosf(degrees));
	dst->m.B = ftoq(sinf(degrees));
	dst->m.C = ftoq(-sinf(degrees));
	dst->m.D = ftoq(cosf(degrees));
}

void mat2_mul(uObjMtx *m1, uObjMtx *m2) {
	mat2_dst_mul(m1, m1, m2);
}

void mat2_add(uObjMtx *m1, uObjMtx *m2) {
	mat2_dst_mul(m1, m1, m2);
}

void mat2_scale(uObjMtx *dst, int scale) {
	dst->m.A *= scale;
	// dst->m.B *= scale;
	// dst->m.C *= scale;
	dst->m.D *= scale;
}

#define	FTOFIX16(x)	(long)((x) * (float)(1 << 2))
#define	FIX16TOF(x)	((float)(x) * (1.0f / (float)(1 << 2)))
void mat2_translate(uObjMtx *m, int x, int y) {
	m->m.X = x<<2;
	m->m.Y = y<<2;
}


int s2d_atoi(char *s, char **s2) {
	int ret = 0;
	int isNegative = (*s == '-');
	if (isNegative) {
		s++;
		(*s2)++;
	}
	for (; *s != '\0' && *s != ' ' && *s != CH_SEPARATOR && *s >= '0' && *s <= '9'; s++) {
		ret *= 10;
		if (*s >= '0' && *s <= '9')
			ret += *s - '0';
		else break;
		if (!(*(s+1) != '\0' && *(s+1) != CH_SEPARATOR && *(s+1) != ' ' && *(s+1) >= '0' && *(s+1) <= '9')) break;
		(*s2)++;
	}
	if (isNegative) ret *= -1;
	return ret;
}

float myScale = 1.0f;
int myDegrees = 0;
uObjMtx final_mtx, rot_mtx;
int s2d_red = 255, s2d_green = 255, s2d_blue = 255, s2d_alpha = 255;
int drop_shadow = 0;
int drop_x = 0;
int drop_y = 0;

struct s2d_optimized_linkedlist {
    struct s2d_optimized_linkedlist *next;
    unsigned char envR;
    unsigned char envG;
    unsigned char envB;
    unsigned char envA;
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

#include <stdarg.h>

static int s2d_width(const char *str, int line, int len);
static void s2d_snprint(int x, int y, int align, const char *str, int len);

void make_new_list_el(S2DList *s) {
    if (s->head == NULL) {
        s->head = malloc(sizeof(S2DListNode));
        s->tail = s->head;
    } else {
        if (s->tail) {
            s->tail->next = malloc(sizeof(S2DListNode));
            s->tail = s->tail->next;
        }
    }
    s->tail->next = NULL;
    s->len++;
}

void mtx_pipeline_op(uObjMtx *m, int x, int y, float scale) {
    mat2_ident(m, 1);
    mat2_scale(m, scale);
    mat2_translate(m, x, y);
}


void make_glyph(S2DList *s, int x, int y,
                int glyph,
                int r, int g, int b, int a,
                float scl,
                int d, int dx, int dy
) {
    make_new_list_el(s);
    S2DListNode *t = s->tail;

    // mtx_pipeline_op(&t->mtx, x, y, scl);

    t->envR = r;
    t->envG = g;
    t->envB = b;
    t->envA = a;

    t->glyph = glyph;

    if (d == 1) {
        t->dropshadow = malloc(sizeof(uObjMtx));
        // mtx_pipeline_op(t->dropshadow, x + dx, y + dy, scl);
    }
}

#define CLAMP_0(x) ((x < 0) ? 0 : x)

int slatch = 0;
void draw_all_glyphs(S2DList *s) {
    S2DListNode *tmp = s->head;

    while (tmp != NULL) {
        putc(tmp->glyph, stdout);
        tmp = tmp->next;
    }
    slatch = 0;
}



static void s2d_snprint(int x, int y, int align, const char *str, int len) {
    char *p = str;
    int tmp_len = 0;
    int orig_x = x;
    int orig_y = y;
    int line = 0;

    if (*p == '\0') return;

    // resets parameters
    // s2d_red = s2d_green = s2d_blue = 255;
    // s2d_alpha = 255;
    drop_shadow = 0;


    S2DList *s2d_list = malloc(sizeof(S2DList));
    s2d_list->head = NULL;
    s2d_list->tail = NULL;
    s2d_list->len = 0;

    do {
        char current_char = *p;

        switch (current_char) {
            case CH_SCALE:
                CH_SKIP(p);
                myScale = (float)s2d_atoi(p, &p) / 100.0f;
                break;
            case CH_ROT:
                CH_SKIP(p);
                myDegrees = s2d_atoi(p, &p);
                break;
            case CH_TRANSLATE:
                CH_SKIP(p);
                orig_x = s2d_atoi(p, &p);
                line++;
                CH_SKIP(p);
                CH_SKIP(p);
                orig_y = s2d_atoi(p, &p);
                y = orig_y;
                break;
            case CH_COLOR:
                // CH_SKIP(p);
                // s2d_red = s2d_atoi(p, &p);
                // CH_SKIP(p); CH_SKIP(p);

                // s2d_green = s2d_atoi(p, &p);
                // CH_SKIP(p); CH_SKIP(p);

                // s2d_blue = s2d_atoi(p, &p);
                // CH_SKIP(p); CH_SKIP(p);
                
                // s2d_alpha = s2d_atoi(p, &p);
                break;
            case CH_DROPSHADOW:
                drop_shadow ^= 1;
                break;
            case '\n':
                line++;
                break;
            case '\t':
                break;
            case '\v':
                break;
            // case CH_SEPARATOR:
            //  CH_SKIP(p);
            //  break;
            case CH_RESET:
                // s2d_red = s2d_green = s2d_blue = 255;
                // s2d_alpha = 255;
                drop_shadow = 0;
                // drop_x = 0;
                // drop_y = 0;
                myScale = 1;
                myDegrees = 0;
                break;
            default:
                if (current_char != '\0' && current_char != CH_SEPARATOR) {

                    make_glyph(s2d_list, x, y,
                               current_char,
                               s2d_red, s2d_green, s2d_blue, s2d_alpha,
                               myScale,
                               drop_shadow, drop_x, drop_y
                    );

                }
        }
        if (*p == '\0') break;
        p++;
        tmp_len++;
    } while (tmp_len < len);
    myScale = 1.0f;
    myDegrees = 0;
    drop_shadow = 0;
    drop_x = 0;
    drop_y = 0;

    draw_all_glyphs(s2d_list);
}

void s2d_print_optimized(int x, int y, const char *str) {
    int len;

    len = strlen(str);

    s2d_snprint(x, y, ALIGN_LEFT, str, len);
}


int s2d_ilen(char *s) {
	int ret = 0;
	char *p = s;
	while (*p >= '0' && *p <= '9') {
		ret++;
		p++;
	}
	return ret;
}

int s2d_atoi2(char *s) {
	int ret = 0;
	int isNegative = (*s == '-');
	if (isNegative) {
		s++;
	}
	for (; *s != '\0' && *s != ' ' && *s >= '0' && *s <= '9'; s++) {
		ret *= 10;
		if (*s >= '0' && *s <= '9')
			ret += *s - '0';
		else break;
		if (!(*(s+1) != '\0' && *(s+1) != ' ' && *(s+1) >= '0' && *(s+1) <= '9')) break;
	}
	// (*s2)--;
	if (isNegative) ret *= -1;
	return ret;
}

int pos = 0;
int main(void) {
	s2d_print_optimized(32, 24, SCALE "25" COLOR "0 0 0 255"
                "\tIn this essay, I will be analyzing the effects of\n"
                "anime exposure on the overall quality of Super Mario 64 ROM hacks,\n"
                "with a additional look into the consequences of believing that\n"
                "Honoka Kousaka could jump over that lake.\n");
}