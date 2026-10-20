/*
 * test_resume.c — host test for the /.rechord/state backend (BRNOMT-87).
 *
 * The in-place FAT rewrite itself is covered by tools/test_fs_target.py; this
 * covers the glue: defaults on a missing file, the key=value parser (unknown
 * and torn lines ignored), the serialize -> fs_target_file_rewrite hand-off,
 * and the error mapping.
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_target.h"
#include "rechord_files.h"
#include "resume.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* ---- stub file + rewrite hook ------------------------------------------- */

struct fs_file {
    long pos;
};

static struct fs_file s_f;
static char s_content[2048];
static long s_size;
static int  s_present;
static char s_written[2048];
static long s_written_len = -1;
static char s_written_path[64];
static int  s_rewrite_rc;

fs_file_t *fs_open_read(const char *path)
{
    if (!s_present || path == 0 || strcmp(path, RECHORD_STATE) != 0)
        return 0;
    s_f.pos = 0;
    return &s_f;
}

long fs_read(fs_file_t *f, void *buf, unsigned long cap)
{
    long n = s_size - f->pos;

    if (n > (long)cap)
        n = (long)cap;
    if (n <= 0)
        return 0;
    memcpy(buf, s_content + f->pos, (size_t)n);
    f->pos += n;
    return n;
}

void fs_close(fs_file_t *f)
{
    (void)f;
}

int fs_target_file_rewrite(const char *path, const void *data, uint32_t len)
{
    if (s_rewrite_rc != 0)
        return s_rewrite_rc;
    snprintf(s_written_path, sizeof s_written_path, "%s", path ? path : "");
    memcpy(s_written, data, len);
    s_written_len = (long)len;
    return FS_OK;
}

/* ---- tests -------------------------------------------------------------- */

int main(void)
{
    resume_state_t s;
    int n;

    printf("== resume: /.rechord/state ==\n");

    /* 1. missing file -> load fails, defaults stay. */
    s_present = 0;
    memset(&s, 0xAA, sizeof s);
    CHECK(resume_load(&s) < 0);
    CHECK(s.track[0] == '\0' && s.folder[0] == '\0');
    CHECK(s.pos_ms == 0 && s.row == 0 && s.boot == 0);

    /* 2. present file -> parsed; unknown and torn lines are ignored. */
    s_present = 1;
    s_size = (long)snprintf(s_content, sizeof s_content,
                            "# ReChord state\n"
                            "track=/MUSIC/Album/01 Song.flac\n"
                            "pos_ms=73421\n"
                            "folder=/MUSIC/Album\n"
                            "row=7\n"
                            "boot=42\n"
                            "junk line without equals\n"
                            "=emptyname\n"
                            "unknown=x\n");
    CHECK(resume_load(&s) == 0);
    CHECK(strcmp(s.track, "/MUSIC/Album/01 Song.flac") == 0);
    CHECK(s.pos_ms == 73421UL);
    CHECK(strcmp(s.folder, "/MUSIC/Album") == 0);
    CHECK(s.row == 7);
    CHECK(s.boot == 42UL);

    /* 3. CRLF + a truncated last line (power cut mid-write): what is complete
     *    survives, the torn tail is ignored. */
    s_size = (long)snprintf(s_content, sizeof s_content,
                            "track=/A/B.flac\r\n"
                            "pos_ms=1200\r\n"
                            "folder=/A\r\n"
                            "row=3\r\n"
                            "tra");
    CHECK(resume_load(&s) == 0);
    CHECK(strcmp(s.track, "/A/B.flac") == 0);
    CHECK(s.pos_ms == 1200UL && s.row == 3);

    /* 4. save -> serialized text, exact path, in-place hook. */
    s_written_len = -1;
    s_written_path[0] = '\0';
    resume_default(&s);
    snprintf(s.track, sizeof s.track, "%s", "/SD/Music/t.flac");
    snprintf(s.folder, sizeof s.folder, "%s", "/SD/Music");
    s.pos_ms = 4200;
    s.row = 11;
    s.boot = 9;
    CHECK(resume_save(&s) == 0);
    CHECK(strcmp(s_written_path, RECHORD_STATE) == 0);
    CHECK(s_written_len > 0);
    CHECK(strstr(s_written, "track=/SD/Music/t.flac") != 0);
    CHECK(strstr(s_written, "pos_ms=4200") != 0);
    CHECK(strstr(s_written, "folder=/SD/Music") != 0);
    CHECK(strstr(s_written, "row=11") != 0);
    CHECK(strstr(s_written, "boot=9") != 0);
    /* ... and it round-trips through the parser. */
    memcpy(s_content, s_written, (size_t)s_written_len);
    s_content[s_written_len] = '\0';
    s_size = s_written_len;
    {
        resume_state_t r;

        CHECK(resume_load(&r) == 0);
        CHECK(strcmp(r.track, s.track) == 0);
        CHECK(strcmp(r.folder, s.folder) == 0);
        CHECK(r.pos_ms == 4200UL && r.row == 11);
        CHECK(r.boot == 9UL);
    }

    /* 5. over-long value is truncated to the field, not rejected. */
    s_size = (long)snprintf(s_content, sizeof s_content, "track=%0*d\n",
                            400, 0);
    CHECK(resume_load(&s) == 0);
    CHECK(strlen(s.track) == sizeof s.track - 1u);

    /* 6. rewrite failure -> save reports it (previous file untouched). */
    s_rewrite_rc = FS_ERR_UNSUPPORTED;
    CHECK(resume_save(&s) < 0);
    s_rewrite_rc = 0;

    (void)n;
    printf("%d checks, %d failures\n", s_checks, s_failures);
    return s_failures == 0 ? 0 : 1;
}
