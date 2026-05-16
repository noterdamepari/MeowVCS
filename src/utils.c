#include "meow.h"
#include <stdio.h>

char is_path_absolute(char* path) {
    if (!path || path[0] == '\0')
        return 0;
    if (path[0] == '/' || path[0] == '\\')
        return 1; // Unix
    if (isalpha(path[0]) && path[1] == ':')
        return 1; // Windows
    return 0;
}

void get_object_path(char* dest, const char* work_dir, uint8_t* hash) {
    sprintf(dest, "%s/objects/%02x/", work_dir, hash[0]);
    mkdir(dest, 0777);

    char hex_hash[41];
    for (int i = 0; i < 20; i++) {
        sprintf(hex_hash + i * 2, "%02x", hash[i]);
    }
    strcat(dest, hex_hash + 2);
}

int make_path_relative(const char* root, const char* input, char* output) {
    char res[PATH_MAX];

    if (realpath(input, res) == NULL) {
        return -1;
    }

    if (strncmp(res, root, strlen(root)) != 0) {
        return -2;
    }

    const char* relative_ptr = res + strlen(root);

    if (*relative_ptr == '/') {
        relative_ptr++;
    }

    strcpy(output, relative_ptr);

    return 0;
}

char find_work_dir(char* buffer) {
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    while (1) {
        char tmp_path[PATH_MAX];
        snprintf(tmp_path, sizeof(tmp_path), "%s/.meow", cwd);

        struct stat st;

        if (!stat(tmp_path, &st) && S_ISDIR(st.st_mode)) {
            strcpy(buffer, tmp_path);
            return 0;
        }

        if (!strcmp(cwd, "/")) {
            break; // root
        }

        char* last_slash = strrchr(cwd, '/');
        if (last_slash != NULL) {
            if (last_slash == cwd) {
                strcpy(cwd, "/"); // root
            } else {
                *last_slash = '\0';
            }
        } else {
            break;
        }
    }
    return -1;
}

void find_project_dir(char* buffer, char* work_dir) {
    int len = strlen(work_dir);
    if (len > 6) {
        strncpy(buffer, work_dir, len - 6);
        buffer[len - 6] = '\0';
    } else {
        strcpy(buffer, "/");
    }
}

char create_blob(char* path, char* work_dir, struct stat* st, char* hash) {
    char work_obj_dir[PATH_MAX];
    char path_to_tempfile[PATH_MAX];
    char path_to_blob_dir[PATH_MAX];
    char path_to_blob[PATH_MAX];

    snprintf(work_obj_dir, PATH_MAX, "%s%s", work_dir, objects_dir);
    snprintf(path_to_tempfile, PATH_MAX, "%s/tempfile", work_obj_dir);

    uint8_t binary_hash[CHUNK];

    SHA1_CTX sha;
    SHA1Init(&sha);

    FILE* f = fopen(path, "rb");
    if (!f)
        assert("Cannot open file");
    FILE* tempfile = tmpfile();
    if (!tempfile)
        assert("Cannot create tempfile");

    char read_buffer[CHUNK];

    // формируем хедер
    char header[64];
    uint32_t h_len = snprintf(header, sizeof(header), "blob %ld", st->st_size);
    h_len++;

    SHA1Update(&sha, (uint8_t*)header, h_len); // хешируем хедер блоба
    fwrite(header, sizeof(char), h_len, tempfile);

    size_t bytes_read;
    while ((bytes_read = fread(read_buffer, sizeof(uint8_t), CHUNK, f)) > 0) {
        SHA1Update(&sha, (uint8_t*)read_buffer, bytes_read);
        fwrite(read_buffer, sizeof(uint8_t), bytes_read, tempfile);
    }
    SHA1Final(binary_hash, &sha);

    for (int i = 0; i < 20; i++)
        sprintf(hash + (i * 2), "%02x", binary_hash[i]);
    hash[40] = '\0';

    char blob_dir_name[3];
    for (int i = 0; i < 2; i++)
        blob_dir_name[i] = hash[i];
    blob_dir_name[2] = '\0';
    snprintf(path_to_blob_dir, PATH_MAX, "%s/%s", work_obj_dir, blob_dir_name);
    mkdir(path_to_blob_dir, 0777);

    const char* blob_file_name = hash + 2;
    snprintf(path_to_blob, PATH_MAX, "%s/%s", path_to_blob_dir, blob_file_name);

    FILE* blob = fopen(path_to_blob, "wb");
    if (!blob)
        assert("Cannot create blob");
    rewind(tempfile); // return cursor
    def(tempfile, blob, Z_DEFAULT_COMPRESSION);

    printf("%s\n", hash);

    fclose(f);
    fclose(tempfile);
    remove(path_to_tempfile);
    fclose(blob);
    return 0;
}

void write_tree(indexEntry* entries, int entries_amt) {
    FILE* tree_obj = tmpfile();
    for (int i = 0; i < entries_amt; i++) {
        char* something = strrchr(entries[i].path, '/');
        // in root dir
        if (!something) {
            fprintf(tree_obj, "100644 blob %s\n", entries[i].hash, entries[i].path);
        } else {
            *something = '\0';
        }
    }
}

/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.5  11 February 2026  Mark Adler */

/* Version history:
   1.0  30 Oct 2004  First version
   1.1   8 Nov 2004  Add void casting for unused return values
                     Use switch statement for inflate() return values
   1.2   9 Nov 2004  Add assertions to document zlib guarantees
   1.3   6 Apr 2005  Remove incorrect assertion in inf()
   1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
                     Avoid some compiler warnings for input and output buffers
   1.5  11 Feb 2026  Use underscores for Windows POSIX names
 */

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), _O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE* source, FILE* dest, int level) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);   /* no bad return value */
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END); /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE* source, FILE* dest) {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR; /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret) {
    fputs("zpipe: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
    }
}
