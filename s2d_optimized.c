#include <ultra64.h>
#include <PR/gs2dex.h>
#include <PR/gu.h>
#include <stdarg.h>

#include "config.h"

#include "s2d_draw.h"
#include "s2d_print.h"
#include "s2d_ustdlib.h"
#include "s2d_error.h"
#include "s2d_optimized.h"
#include "mtx.h"

static int s2d_width(const char *str, int line, int len);
static void s2d_snprint(int x, int y, int align, const char *str, int len);

void make_new_list_el(S2DList *s) {
    if (s->head == NULL) {
        s->head = alloc(sizeof(S2DListNode));
        s->tail = s->head;
    } else {
        if (s->tail) {
            s->tail->next = alloc(sizeof(S2DListNode));
            s->tail = s->tail->next;
        }
    }
    s->tail->next = NULL;
    s->len++;
}

void mtx_pipeline_op(uObjMtx *m, int x, int y, int scale) {
    // init
    Mat4 tmp, rot, scal, translate;
    guMtxIdentF(tmp);
    guScaleF(scal, BASE_SCALE * (scale), BASE_SCALE * (scale), 0);
    guRotateF(rot, (f32) myDegrees, 0, 0, 1.0f);
    guTranslateF(translate, x, y, 0);

    mtxf_mul(tmp, tmp, scal);
    mtxf_mul(tmp, tmp, rot);
    mtxf_mul(tmp, tmp, translate);

    gu_to_gs2dex(m, tmp);

    // gSPObjMatrix(gdl_head++, m);
}

void make_glyph(S2DList *s, int x, int y,
                int glyph,
                int r, int g, int b, int a,
                float scl,
                int d, int dx, int dy
) {
    make_new_list_el(s);
    S2DListNode *t = s->tail;

    mtx_pipeline2(&t->mtx, x, y, scl);

    t->envR = r;
    t->envG = g;
    t->envB = b;
    t->envA = a;

    t->glyph = glyph;

    if (d == 1) {
        t->dropshadow = alloc(sizeof(uObjMtx));
        mtx_pipeline_op(t->dropshadow, x + dx, y + dy, scl);
    }
}

#define CLAMP_0(x) ((x < 0) ? 0 : x)

int slatch = 0;
void draw_all_glyphs(S2DList *s) {
    S2DListNode *tmp = s->head;

    while (tmp != NULL) {
        gDPPipeSync(gdl_head++);
        gDPSetEnvColor(gdl_head++, tmp->envR, tmp->envG, tmp->envB, tmp->envA);
        gSPObjLoadTxtr(gdl_head++, &s2d_tex[tmp->glyph]);
        // if (tmp->dropshadow) {
        //     gDPPipeSync(gdl_head++);
        //     gDPSetEnvColor(gdl_head++,
        //                CLAMP_0(s2d_red - 100),
        //                CLAMP_0(s2d_green - 100),
        //                CLAMP_0(s2d_blue - 100),
        //                s2d_alpha);
        //     gSPObjMatrix(gdl_head++, &tmp->dropshadow);
        //     gSPObjSprite(gdl_head++, &s2d_dropshadow);
        //     gDPPipeSync(gdl_head++);
        //     gDPSetEnvColor(gdl_head++, s2d_red, s2d_green, s2d_blue, s2d_alpha);
        // }

        gSPObjMatrix(gdl_head++, &tmp->mtx);
        // gSPObjMatrix(gdl_head++, &s2d_mat);
        gSPObjSprite(gdl_head++, &s2d_font);
        // draw_s2d_glyph(tmp-/>glyph, tmp->x, tmp->y, &tmp->mtx);

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

    s2d_rdp_init();

    // resets parameters
    s2d_red = s2d_green = s2d_blue = 255;
    s2d_alpha = 255;
    drop_shadow = FALSE;

    switch (align) {
        case ALIGN_CENTER:
            x = orig_x - s2d_width(str, line, len) / 2;
            break;
        case ALIGN_RIGHT:
            x = orig_x - s2d_width(str, line, len);
    }

    S2DList *s2d_list = alloc(sizeof(S2DList));
    s2d_list->head = NULL;
    s2d_list->tail = NULL;
    s2d_list->len = 0;

    do {
        char current_char = *p;

        switch (current_char) {
            case CH_SCALE:
                CH_SKIP(p);
                myScale = (f32)s2d_atoi(p, &p) / 100.0f;
                break;
            case CH_ROT:
                CH_SKIP(p);
                myDegrees = s2d_atoi(p, &p);
                break;
            case CH_TRANSLATE:
                CH_SKIP(p);
                orig_x = s2d_atoi(p, &p);
                line++;
                switch (align) {
                    case ALIGN_LEFT:
                        x = orig_x;
                        break;
                    case ALIGN_CENTER:
                        x = orig_x - s2d_width(str, line, len) / 2;
                        break;
                    case ALIGN_RIGHT:
                        x = orig_x - s2d_width(str, line, len);
                }
                CH_SKIP(p);
                CH_SKIP(p);
                orig_y = s2d_atoi(p, &p);
                y = orig_y;
                break;
            case CH_COLOR:
                CH_SKIP(p);
                s2d_red = s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);

                s2d_green = s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);

                s2d_blue = s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);
                
                s2d_alpha = s2d_atoi(p, &p);
                break;
            case CH_DROPSHADOW:
                drop_shadow ^= 1;
                break;
            case '\n':
                line++;
                switch (align) {
                    case ALIGN_LEFT:
                        x = orig_x;
                        break;
                    case ALIGN_CENTER:
                        x = orig_x - s2d_width(str, line, len) / 2;
                        break;
                    case ALIGN_RIGHT:
                        x = orig_x - s2d_width(str, line, len);
                }
                y += TEX_HEIGHT / TEX_RES;
                break;
            case '\t':
                x += TAB_WIDTH_H / TEX_RES;
                break;
            case '\v':
                x += TAB_WIDTH_V / TEX_RES;
                y += TEX_HEIGHT / TEX_RES;
                break;
            // case CH_SEPARATOR:
            //  CH_SKIP(p);
            //  break;
            case CH_RESET:
                s2d_red = s2d_green = s2d_blue = 255;
                s2d_alpha = 255;
                drop_shadow = FALSE;
                drop_x = 0;
                drop_y = 0;
                myScale = 1;
                myDegrees = 0;
                break;
            default:
                if (current_char != '\0' && current_char != CH_SEPARATOR) {
                    char *tbl = segmented_to_virtual(s2d_kerning_table);

                    make_glyph(s2d_list, x, y,
                               current_char,
                               s2d_red, s2d_green, s2d_blue, s2d_alpha,
                               myScale,
                               drop_shadow, drop_x, drop_y
                    );

                    (x += (tbl[(int) current_char] * (BASE_SCALE * myScale)));
                }
        }
        if (*p == '\0') break;
        p++;
        tmp_len++;
    } while (tmp_len < len);
    myScale = 1.0f;
    myDegrees = 0;
    drop_shadow = FALSE;
    drop_x = 0;
    drop_y = 0;

    draw_all_glyphs(s2d_list);
}

void s2d_print_optimized(int x, int y, const char *str) {
    int len;

    if (s2d_check_str(str)     != 0) return;

    len = s2d_strlen(str);

    s2d_snprint(x, y, ALIGN_LEFT, str, len);
}

static int s2d_width(const char *str, int line, int len) {
    char *p = str;
    int tmp_len = 0;
    int curLine = 0;
    int width = 0;
    int scale = 1;

    if (*p == '\0') return width;

    do {
        char current_char = *p;
        switch (current_char) {
            case CH_SCALE:
                CH_SKIP(p);
                scale = s2d_atoi(p, &p);
                break;
            case CH_ROT:
                CH_SKIP(p);
                s2d_atoi(p, &p);
                break;
            case CH_TRANSLATE:
                CH_SKIP(p);
                s2d_atoi(p, &p);
                curLine++;
                CH_SKIP(p);
                CH_SKIP(p);
                s2d_atoi(p, &p);
                break;
            case CH_COLOR:
                CH_SKIP(p);
                s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);
                s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);
                s2d_atoi(p, &p);
                CH_SKIP(p); CH_SKIP(p);
                s2d_atoi(p, &p);
                break;
            case CH_DROPSHADOW:
            case CH_RESET:
                break;
            case '\n':
                curLine++;
                break;
            case '\t':
                if (curLine == line)
                    width += TAB_WIDTH_H / TEX_RES;
                break;
            case '\v':
                if (curLine == line)
                    width += TAB_WIDTH_V / TEX_RES;
                break;
            default:
                if (current_char != '\0' && curLine == line)
                    width += s2d_kerning_table[(int) current_char] * scale;
        }
        if (*p == '\0') break;
        p++;
        tmp_len++;
    } while (tmp_len < len && curLine <= line);
    return width;
}
