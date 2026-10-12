/*
 * codec_registry.h — extension -> codec_t dispatch for the player module.
 *
 * The registry is the codec "vtable table": every codec descriptor
 * (codec_t, codec.h) carries its own `extensions` key, is isolated in its
 * own file under player/codecs/, and is bound here at registration time.
 * Dropping a codec's .c from PLAYER_SRCS removes it from the build without
 * touching the core (Makefile-level conditional compilation).
 *
 * Public registration stays player_register_codec() (player.h) so tests
 * can inject mock codecs; codec_registry_find() resolves a path to its
 * codec (player.c's open path).
 */
#ifndef RECHORD_CODEC_REGISTRY_H
#define RECHORD_CODEC_REGISTRY_H

#include "../codec.h"

/* Resolve the codec for `path` by its (last) extension, case-insensitive.
 * Returns NULL when no codec is registered for the extension. */
const codec_t *codec_registry_find(const char *path);

#endif /* RECHORD_CODEC_REGISTRY_H */
