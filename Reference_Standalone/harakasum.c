#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "hash.h"

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 16384
#endif

#ifndef CHUNK_SIZE_BIG
#define CHUNK_SIZE_BIG 1048576
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MODE_TEXT 0
#define MODE_BINARY 1
#define MODE_CHECK 2

#define HASHING_16kRootsSEQ 0
#define HASHING_1MbTreesRoot 1

const char *program = "harakasum";
const char *algorithm = "haraka";

int process_16kRoots_256seq (struct hash *val, const char *name, int mode) {
    FILE *fp = NULL;
    const char *modestr[] = { [MODE_TEXT] = "r", [MODE_BINARY] = "rb" };
    size_t nread = 0;
    int j = 0;

    if (!name)
        fp = stdin;
    else if (!(fp = fopen (name, modestr[mode]))) {
        fprintf (stderr, "%s: error opening file %s with mode %s: %s\n",
                 program, name, modestr[mode], strerror (errno));
        return -1;
    }

    struct hash bufhash, sum, tmp[2];
    uint8_t *buf = (uint8_t*) malloc(CHUNK_SIZE);

    do {
        nread = fread (buf, 1, CHUNK_SIZE, fp);
        if (nread) {
            hash_to_N (&bufhash, buf, nread);
            if (j) {
                tmp[0] = sum;
                tmp[1] = bufhash;
                hash_2N_to_N (&sum, &tmp[0]);
            }
            if (!j) {
                sum = bufhash;
                j++;
            }
        }
    } while (nread > 0);

    if (ferror (fp)) {
        fprintf (stderr, "%s: error reading file %s: %s\n", program, name, strerror (errno));
        fclose (fp);
        return -1;
    }

    *val = sum;

    free(buf);
    fclose (fp);
    return 0;
}

int process_1mbRoots_256root (struct hash *val, const char *name, int mode) {
    FILE *fp = NULL;
    const char *modestr[] = { [MODE_TEXT] = "r", [MODE_BINARY] = "rb" };

    if (!name)
        fp = stdin;
    else if (!(fp = fopen (name, modestr[mode]))) {
        fprintf (stderr, "%s: error opening file %s with mode %s: %s\n",
                 program, name, modestr[mode], strerror (errno));
        return -1;
    }

    uint8_t *buf = malloc(CHUNK_SIZE_BIG);
    size_t roots_num = 256, i = 0;
    struct hash *roots = (struct hash*) malloc(roots_num * HASH_SIZE);
    size_t nread = 0;

    do {
        if (i == roots_num) {
            roots_num += 256;
            roots = (struct hash*) realloc(roots, roots_num * HASH_SIZE);
        }

        nread = fread (buf, 1, CHUNK_SIZE_BIG, fp);
        if (nread) {
            hash_to_N (&roots[i++], buf, nread);
        }
    } while (nread > 0);

    if (ferror (fp)) {
        fprintf (stderr, "%s: error reading file %s: %s\n", program, name, strerror (errno));
        fclose (fp);
        return -1;
    }

    hash_to_N(val, (const uint8_t*)roots, i * HASH_SIZE);

    fclose (fp);
    free(roots);
    free(buf);
    return 0;
}

int process(struct hash *val, const char *name, int mode, int hashing) {
    if (hashing == HASHING_16kRootsSEQ) {
        return process_16kRoots_256seq(val, name, mode);
    }
    if (hashing == HASHING_1MbTreesRoot) {
        return process_1mbRoots_256root(val, name, mode);
    }
    return -1;
}

void print (struct hash *h, const char *name, int mode, int hashing) {
    for (int i = 0; i < HASH_SIZE; i++) printf ("%02x", h->h[i]);
    printf (" %c%c%s\n", (hashing == HASHING_1MbTreesRoot) ? '^' : ' ', (mode == MODE_BINARY) ? '*' : ' ', name ? name : "-");
}

int check (const char *filename) {
    int errors = 0;
    FILE *fp;
    char buf[(HASH_SIZE * 2) + 2 + PATH_MAX + 2];
    char fnbuf[sizeof (buf)];

    if (!filename)
        fp = stdin;
    else if (!(fp = fopen (filename, "r"))) {
        fprintf (stderr, "%s: error opening file %s with mode %s: %s\n",
                 program, filename, "r", strerror (errno));
        return -1;
    }

    while (fgets (buf, sizeof (buf), fp)) {
        struct hash val, computed;
        size_t len = strlen (buf);
        char dummy, type, hashtype;
        int mode, cmp, hashing;
        if (buf[len - 1] != '\n') continue;
        buf[len - 1] = '\0';
        for (int i = 0; i < HASH_SIZE; i++)
            if (!sscanf (buf + (2 * i), "%02hhx", &val.h[i])) goto next;
        if (sscanf (buf + (2 * HASH_SIZE), "%c%c%c%s", &dummy, &hashtype, &type, fnbuf) != 4)
            continue;
        fnbuf[sizeof (fnbuf) - 1] = '\0';
        if (dummy != ' ') continue;
        switch (type) {
        case ' ':
            mode = MODE_TEXT;
            break;
        case '*':
            mode = MODE_BINARY;
            break;
        default:
            continue;
        }
        switch (hashtype) {
        case ' ':
            hashing = HASHING_16kRootsSEQ;
            break;
        case '^':
            hashing = HASHING_1MbTreesRoot;
            break;
        default:
            continue;
        }
        if (process (&computed, fnbuf, mode, hashing) < 0) continue;
        if ((cmp = memcmp (&computed, &val, sizeof (val)))) errors++;
        printf ("%s: %s\n", fnbuf, cmp ? "FAILED" : "OK");
    next:;
    }

    fclose (fp);
    return errors;
}

int usage (int ret) {
    printf ("Usage: %s [-tbc] [file1] [file2] ...\n"
            "Print or check %s hashes.\n"
            "\n"
            "-s, --seq: sequential tree hashing (default, lower footprint)\n"
            "-r, --root: construct a tree of roots and then calculate is merkle root (faster on big files)\n"
            "-t, --text: read file in text mode (default)\n"
            "-b, --binary: read file in binary mode\n"
            "-c, --check: read hashes from the file and check them\n",
            program, algorithm);
    return ret;
}

int main (int argc, char **argv) {
    int c, mode = MODE_TEXT, hashing = HASHING_16kRootsSEQ;
    int retval = 0;

    if (argc == 1) return usage (0);

    while ((c = getopt (argc, argv, "bctrs:-:")) != -1) {
        if (c == '-') {
            if (!strcmp (optarg, "seq"))
                c = 's';
            if (!strcmp (optarg, "root"))
                c = 'r';
            if (!strcmp (optarg, "text"))
                c = 't';
            else if (!strcmp (optarg, "binary"))
                c = 'b';
            else if (!strcmp (optarg, "check"))
                c = 'c';
            else if (!strcmp (optarg, "help"))
                return usage (0);
            else
                c = '?';
        }
        switch (c) {
        case 'b':
            mode = MODE_BINARY;
            break;
        case 'r':
            hashing = HASHING_1MbTreesRoot;
            break;
        case 's':
            hashing = HASHING_16kRootsSEQ;
            break;
        case 't':
            mode = MODE_TEXT;
            break;
        case 'c':
            mode = MODE_CHECK;
            break;
        case '?':
            return usage (2);
        }
    }

    if (mode == MODE_CHECK) {
        const char *p = argv[optind];
        do {
            if (check (p)) retval = 1;
        } while (p && (p = argv[++optind]));
    } else {
        struct hash val;
        const char *p = argv[optind];
        do {
            if (process (&val, p, mode, hashing))
                retval = 1;
            else
                print (&val, p, mode, hashing);
        } while (p && (p = argv[++optind]));
    }

    return retval;
}

