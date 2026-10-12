/*
 * codec_registry.c — extension -> codec_t dispatch (implementation).
 *
 * Moved out of player.c so the core state machine owns playback only and
 * codecs stay fully self-describing (name + extensions in the descriptor,
 * one file per codec, registration explicit). Capacity is a deliberate
 * small cap: the device ships wav/mp3/flac (+ test mocks).
 */
#include <stddef.h>

#include "player.h"            /* player_register_codec, PLAYER_OK/ERR    */
#include "codec_registry.h"

#define CODEC_REGISTRY_MAX 8

typedef struct
{
    char            ext[8];
    const codec_t  *codec;
} codec_slot_t;

static codec_slot_t s_slots[CODEC_REGISTRY_MAX];
static int          s_count;

/* ---- tiny libc-free helpers (freestanding target) ---- */

static char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static int str_eq_i(const char *a, const char *b)
{
    unsigned i = 0;
    while (lower(a[i]) == lower(b[i])) {
        if (a[i] == '\0')
            return 1;
        i++;
    }
    return 0;
}

static const char *ext_of(const char *path)
{
    const char *dot = 0;
    unsigned i;

    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '.')
            dot = path + i + 1;
    }
    return dot;
}

/* ---- API ---- */

const codec_t *codec_registry_find(const char *path)
{
    const char *ext;
    int i;

    if (path == 0)
        return 0;
    ext = ext_of(path);
    if (ext == 0)
        return 0;
    for (i = 0; i < s_count; i++) {
        if (str_eq_i(ext, s_slots[i].ext))
            return s_slots[i].codec;
    }
    return 0;
}

int player_register_codec(const char *ext, const codec_t *codec)
{
    int i;

    if (ext == 0 || codec == 0 || ext[0] == '\0')
        return PLAYER_ERR;

    for (i = 0; i < s_count; i++) {
        if (str_eq_i(ext, s_slots[i].ext)) {
            s_slots[i].codec = codec;   /* override (tests inject mocks) */
            return PLAYER_OK;
        }
    }
    if (s_count >= CODEC_REGISTRY_MAX)
        return PLAYER_ERR;
    /* bounded copy of the extension key (max 7 chars + NUL) */
    for (i = 0; i < 7 && ext[i] != '\0'; i++)
        s_slots[s_count].ext[i] = ext[i];
    s_slots[s_count].ext[i] = '\0';
    s_slots[s_count].codec = codec;
    s_count++;
    return PLAYER_OK;
}
