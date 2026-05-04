#include "meow.h"

char is_path_absolute(char* path){
    if (!path || path[0] == '\0') return 0;
    if (path[0] == '/' || path[0] == '\\') return 1; // Unix
    if (isalpha(path[0]) && path[1] == ':') return 1; // Windows
    return 0;
}

void get_object_path(char *dest, const char *work_dir, uint8_t *hash) {
    sprintf(dest, "%s/objects/%02x/", work_dir, hash[0]);
    mkdir(dest, 0777);
    
    char hex_hash[41];
    for(int i = 0; i < 20; i++) {
        sprintf(hex_hash + i*2, "%02x", hash[i]);
    }
    strcat(dest, hex_hash + 2);
}

int make_path_relative(const char *root, const char *input, char *output) {
    char res[PATH_MAX];

    if (realpath(input, res) == NULL) {
        return -1;
    }

    if (strncmp(res, root, strlen(root)) != 0) {
        return -2;
    }

    const char *relative_ptr = res + strlen(root);

    if (*relative_ptr == '/') {
        relative_ptr++;
    }

    strcpy(output, relative_ptr);

    return 0;
}

char find_work_dir(char* buffer){
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    while(1){
        char tmp_path[PATH_MAX];
        snprintf(tmp_path, sizeof(tmp_path), "%s/.meow", cwd);

        struct stat st;

        if(!stat(tmp_path, &st) && S_ISDIR(st.st_mode)){
            strcpy(buffer, tmp_path);
            return 0;
        }

        if (!strcmp(tmp_path, "/")){
            break; // root
        }

        char* last_slash = strrchr(cwd, '/');
        if (last_slash == cwd){
            strcpy(cwd, "/"); // root
        } else if (last_slash){
            *last_slash = '\0';
        } else {
            break;
        }
    }
    return 1;
}


void find_project_dir(char* buffer, char* work_dir){
    char project_dir[PATH_MAX];
    int len = strlen(work_dir);
    if (len > 6) {
        strncpy(buffer, work_dir, len - 6);
        buffer[len-6] = '\0';
    } else {
        strcpy(buffer, "/");
    }
}


char add_to_indexfile(char* path, char* hash, char* work_dir){
    printf("%s", hash);
    struct stat st;
    if (stat(path, &st) != 0) assert("Err: file doesn`t exists");

    int64_t file_mtime = st.st_mtime;


    char path_to_index[PATH_MAX];
    char path_to_indextmp[PATH_MAX];
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_indextmp, PATH_MAX, "%s/index.tmp", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) assert("index not found");
    FILE* index_tmp = fopen(path_to_indextmp, "wb");
    if (!index) assert("index.tmp not created");
    unsigned int something = 0;
    fprintf(index_tmp, "%u\n", something);
    unsigned int entries_amt;
    fscanf(index, "%u\n", &entries_amt);

    indexEntry entry;

    char project_dir[PATH_MAX];
    char rel_path[PATH_MAX];
    find_project_dir(project_dir, work_dir);
    make_path_relative(project_dir, path, rel_path);

    char tmpbuffer[PATH_MAX+64];

    char already_in_index = 0;
    for (int i = 0; i < entries_amt; i++){
        fscanf(index, "%40s %hhu %lld %s", entry.hash, &entry.status, &entry.mtime, entry.path);
        if (!strcmp(rel_path, entry.path)){ // already in index -> modified
            already_in_index = 1;
            if (entry.mtime != file_mtime && strcmp(entry.hash, hash)){ // nothing to do
                entry.status = 0;
                strcpy(entry.hash, hash);
                entry.mtime = file_mtime;
            } else {
                puts("Nothing to do, already in index");
                return 1;
            }
        }
        fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
    }

    printf("%s\n", rel_path);
    if(!already_in_index){
        strcpy(entry.hash, hash);
        strcpy(entry.path, rel_path);
        entry.status = 1;
        entry.mtime = file_mtime;
        fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
        entries_amt++;
    }

    rewind(index_tmp);
    fprintf(index_tmp, "%u\n", entries_amt);
 
    fclose(index_tmp);
    fclose(index);

    if (rename(path_to_indextmp, path_to_index) != 0) {
        perror("Err: rename failed");
        return -1;
    }

    return 0;
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
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), _O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
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
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

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
int inf(FILE *source, FILE *dest){
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
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
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
void zerr(int ret)
{
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