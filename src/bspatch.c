/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// 2022
// Still essentially Colin's code - just some minor modifications to
// use the memory buffers instead of files on disk in a couple of places
// as it better suits the needs of this app, and to re-route the error logic.

#if 0
__FBSDID("$FreeBSD: src/usr.bin/bsdiff/bspatch/bspatch.c,v 1.1 2005/08/06 01:59:06 cperciva Exp $");
#endif

#include <bzlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* #include <err.h> */
#ifdef WIN32
 typedef unsigned char u_char;
#else
 #include <features.h>
#endif
#include <unistd.h>
#include <fcntl.h>

#define errx(RC, ERR_MSG) do { fprintf( stderr, "%s", ERR_MSG ); return RC; } while (0);

static off_t offtin(u_char *buf)
{
    off_t y;

    y=buf[7]&0x7F;
    y=y*256;y+=buf[6];
    y=y*256;y+=buf[5];
    y=y*256;y+=buf[4];
    y=y*256;y+=buf[3];
    y=y*256;y+=buf[2];
    y=y*256;y+=buf[1];
    y=y*256;y+=buf[0];

    if(buf[7]&0x80) y=-y;

    return y;
}

int bspatch(unsigned char *oldfile, ssize_t oldsize, const char *newfile, unsigned char *patch, ssize_t patchsize)
{
    bz_stream   cpf, dpf, epf;
    int         cbz2err, dbz2err, ebz2err;
    FILE       *fd;
    ssize_t     newsize;
    ssize_t     bzctrllen, bzdatalen;
    u_char      buf[8];
    u_char     *new;
    off_t       oldpos, newpos;
    off_t       ctrl[3];
    off_t       lenread;
    off_t       i;

    /*
    File format:
        0       8       "BSDIFF40"
        8       8       X
        16      8       Y
        24      8       sizeof(newfile)
        32      X       bzip2(control block)
        32+X    Y       bzip2(diff block)
        32+X+Y  ???     bzip2(extra block)
    with control block a set of triples (x,y,z) meaning "add x bytes
    from oldfile to x bytes from the diff block; copy y bytes from the
    extra block; seek forwards in oldfile by z bytes".
    */

    /* Read header */
    if (patchsize < 32)
        errx(1, "Corrupt patch\n");

    /* Check for appropriate magic */
    if (memcmp(patch, "BSDIFF40", 8) != 0)
        errx(1, "Corrupt patch\n");

    /* Read lengths from header */
    bzctrllen = offtin(patch+8);
    bzdatalen = offtin(patch+16);
    newsize   = offtin(patch+24);
    if ((bzctrllen<0) || (bzdatalen<0) || (newsize<0))
        errx(1, "Corrupt patch\n");

    cpf.next_in   = (char *)patch + 32;
    cpf.avail_in  = patchsize - 32;
    cpf.next_out  = (char *)buf;
    cpf.avail_out = 8;
    cpf.bzalloc   = NULL;
    cpf.bzfree    = NULL;
    cpf.opaque    = NULL;

    dpf.next_in   = (char *)patch + 32 + bzctrllen;
    dpf.avail_in  = patchsize - 32 - bzctrllen;
    dpf.next_out  = (char *)buf;
    dpf.avail_out = 8;
    dpf.bzalloc   = NULL;
    dpf.bzfree    = NULL;
    dpf.opaque    = NULL;

    epf.next_in   = (char *)patch + 32 + bzctrllen + bzdatalen;
    epf.avail_in  = patchsize - 32 - bzctrllen - bzdatalen;
    epf.next_out  = (char *)buf;
    epf.avail_out = 8;
    epf.bzalloc   = NULL;
    epf.bzfree    = NULL;
    epf.opaque    = NULL;

    BZ2_bzDecompressInit( &cpf, 0, 0 );
    BZ2_bzDecompressInit( &dpf, 0, 0 );
    BZ2_bzDecompressInit( &epf, 0, 0 );

    if ((new = malloc(newsize+1))==NULL)
        errx(1, "Couldn't malloc memory for patching\n");

    oldpos = 0;
    newpos = 0;
    while (newpos < newsize)
    {
        /* Read control data */
        for (i=0; i<=2; i++)
        {
           cpf.next_out  = (char *)buf;
           cpf.avail_out = 8;
           cbz2err = BZ2_bzDecompress(&cpf);
           lenread = 8 - cpf.avail_out;

            if ((lenread < 8) || ((cbz2err != BZ_OK) && (cbz2err != BZ_OUTBUFF_FULL) &&
                (cbz2err != BZ_STREAM_END)))
                errx(1, "Corrupt patch\n");
            ctrl[i] = offtin(buf);
        };

        /* Sanity-check */
        if (newpos+ctrl[0] > newsize)
            errx(1, "Corrupt patch\n");

        /* Read diff string */
        dpf.next_out  = (char *)new + newpos;
        dpf.avail_out = ctrl[0];
        dbz2err = BZ2_bzDecompress(&dpf);
        lenread = ctrl[0] - dpf.avail_out;

        if ((lenread < ctrl[0]) ||
            ((dbz2err != BZ_OK) && (dbz2err != BZ_OUTBUFF_FULL) && (dbz2err != BZ_STREAM_END)))
            errx(1, "Corrupt patch\n");

        /* Add oldfile data to diff string */
        for (i=0;i<ctrl[0];i++)
            if ((oldpos+i >= 0) && (oldpos+i < oldsize))
                new[newpos+i] += oldfile[oldpos+i];

        /* Adjust pointers */
        newpos += ctrl[0];
        oldpos += ctrl[0];

        /* Sanity-check */
        if(newpos+ctrl[1] > newsize)
            errx(1, "Corrupt patch\n");

        /* Read extra string */
        epf.next_out  = (char *)new + newpos;
        epf.avail_out = ctrl[1];
        ebz2err = BZ2_bzDecompress(&epf);
        lenread = ctrl[1] - epf.avail_out;

        if ((lenread < ctrl[1]) ||
            ((ebz2err != BZ_OK) && (ebz2err != BZ_OUTBUFF_FULL) && (ebz2err != BZ_STREAM_END)))
            errx(1, "Corrupt patch\n");

        /* Adjust pointers */
        newpos += ctrl[1];
        oldpos += ctrl[2];
    };

    BZ2_bzDecompressEnd( &cpf );
    BZ2_bzDecompressEnd( &dpf );
    BZ2_bzDecompressEnd( &epf );

    /* Write the new file - use fopen() instead of open so we
     * can ensure BINARY mode on Windows */
    if(((fd = fopen(newfile, "wb")) == NULL) ||
        (fwrite(new, 1, newsize, fd) != (size_t)newsize) ||
        (fclose(fd) == -1))
        errx(1, newfile);

    free(new);

    return 0;
}
