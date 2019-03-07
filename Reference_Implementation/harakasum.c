#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "hash.h"

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 16384
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MODE_TEXT 0
#define MODE_BINARY 1
#define MODE_CHECK 2

const char *program = "harakasum";
const char *algorithm = "haraka";

int process (struct hash *val, const char *name, int mode) {
    struct hash bufhash, sum, tmp[2];

    FILE *fp = NULL;
    const char *modestr[] = { [MODE_TEXT] = "r", [MODE_BINARY] = "rb" };
    uint8_t buf[CHUNK_SIZE];
    size_t nread = 0;
    int j = 0;

    if (!name)
        fp = stdin;
    else if (!(fp = fopen (name, modestr[mode]))) {
        fprintf (stderr, "%s: error opening file %s with mode %s: %s\n",
                 program, name, modestr[mode], strerror (errno));
        return -1;
    }

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

    fclose (fp);
    return 0;
}

void print (struct hash *h, const char *name, int mode) {
    for (int i = 0; i < HASH_SIZE; i++) printf ("%02x", h->h[i]);
    printf (" %c%s\n", (mode == MODE_BINARY) ? '*' : ' ', name ? name : "-");
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
        char dummy, type;
        int mode, cmp;
        if (buf[len - 1] != '\n') continue;
        buf[len - 1] = '\0';
        for (int i = 0; i < HASH_SIZE; i++)
            if (!sscanf (buf + (2 * i), "%02hhx", &val.h[i])) goto next;
        if (sscanf (buf + (2 * HASH_SIZE), "%c%c%s", &dummy, &type, fnbuf) != 3)
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
        if (process (&computed, fnbuf, mode) < 0) continue;
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
            "-t, --text: read file in text mode (default)\n"
            "-b, --binary: read file in binary mode\n"
            "-c, --check: read hashes from the file and check them\n",
            program, algorithm);
    return ret;
}

int main (int argc, char **argv) {
    int c, mode = MODE_TEXT;
    int retval = 0;

    while ((c = getopt (argc, argv, "bcta:-:")) != -1) {
        if (c == '-') {
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
            if (process (&val, p, mode))
                retval = 1;
            else
                print (&val, p, mode);
        } while (p && (p = argv[++optind]));
    }

    return retval;
}

