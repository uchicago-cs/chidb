/*
 *  chidb - a didactic relational database management system
 *
 * This module contains functions to access a chidb file one page at a time.
 * All read/write operations must be done through this pager module.
 * In other words, if you need to read something from the database file,
 * you first use the pager to read the page containing the information
 * you're interested in. If you want to modify the file, you need to
 * modify the page returned by the pager and instruct the pager to
 * write it back to disk.
 *
 * The default pager is very simple and always creates an in-memory copy
 * of any page that is read (even if that page has already been read before).
 * More specifically, pages are read into a MemPage structure, which must
 * be freed (using the releaseMemPage function) once they are not needed.
 * In a real database, the pager component typically does some caching
 * of pages to reduce the number of disk accesses.
 *
 */

/*
 *  Copyright (c) 2009-2015, The University of Chicago
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or withsend
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of The University of Chicago nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software withsend specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY send OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <chidb/log.h>

#include "chidbInt.h"

#include "pager.h"

/* Open a file
 *
 * This function opens a file for paged access.
 *
 * Parameters
 * - pager: An out parameter. Used to return a pointer to the
 *			 newly created Pager.
 * - filename: Database file (might not exist)
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 * - CHIDB_EIO: An I/O error has occurred when accessing the file
 */
int chidb_Pager_open(Pager **pager, const char *filename)
{
    *pager = malloc(sizeof(Pager));
    if (pager == NULL)
        return CHIDB_ENOMEM;
    (*pager)->f = fopen(filename, "r+");

    if ((*pager)->f == NULL)
        (*pager)->f = fopen(filename, "w+");

    if ((*pager)->f == NULL)
        return CHIDB_EIO;
    else
        return CHIDB_OK;
}


/* Set the page size
 *
 * This tells the pager what the size of each page is.
 * This function must be called before operating on pages.
 * It will not verify if the page size makes size. If an incorrect
 * page size is provided, this will result in unexpected behaviour.
 *
 * Parameters
 * - pager: A Pager.
 * - pagesize: Size of a page (in bytes)
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_Pager_setPageSize(Pager *pager, uint16_t pagesize)
{
    pager->page_size = pagesize;
    chidb_Pager_getRealDBSize(pager, &pager->n_pages);

    return CHIDB_OK;
}


/* Read the chidb file header
 *
 * This function reads in the header of a chidb file and returns it
 * in a byte array. Note that this function can be called even if
 * the page size is unknown, since the chidb header always occupies
 * the first 100 bytes of the file.
 *
 * Parameters
 * - pager: A Pager.
 * - header: Pointer to a byte array with enough space for 100 bytes.
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_NOHEADER: The file does not have a header. This will
 */
int chidb_Pager_readHeader(Pager *pager, uint8_t *header)
{
    int count;
    count = fseek(pager->f, 0, SEEK_SET);
    count = fread(header, 1, 100, pager->f);
    if (count != 100)
        return CHIDB_NOHEADER;
    else
        return CHIDB_OK;
}


/* Allocate an extra page on the file
 *
 * Parameters
 * - pager: A Pager.
 * - npage: An out parameter that will contain the page number of the
 *          new page.
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_Pager_allocatePage(Pager *pager, npage_t *npage)
{
    /* We simply increment the page number counter. readPage
     * and writePage take care of the rest. */
    *npage = ++pager->n_pages;

    return CHIDB_OK;
}


/* Read a page from file
 *
 * This page reads a page from the file, and creates an in-memory copy
 * in a MemPage struct (see header file for more details on this struct).
 * Always use chidb_Pager_releaseMemPage to free the memory allocated for
 * a MemPage created by this function.
 * Any changes done to a MemPage will not be effective until you call
 * chidb_Pager_writePage with that MemPage.
 *
 * Parameters
 * - pager: A Pager.
 * - npage: Page number of page to read.
 * - page: Out parameter. Used to return a pointer to newly created MemPage
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 * - CHIDB_EIO: An I/O error has occurred when accessing the file
 */
int	chidb_Pager_readPage(Pager *pager, npage_t npage, MemPage **page)
{
    if (npage > pager->n_pages || npage <= 0)
        return CHIDB_EPAGENO;
    int n;

    *page = malloc(sizeof(MemPage));
    if (page == NULL)
        return CHIDB_ENOMEM;
    (*page)->npage = npage;
    (*page)->data = calloc(pager->page_size, 1);
    if ((*page)->data == NULL)
        return CHIDB_ENOMEM;
    fseek(pager->f, (npage - 1) * pager->page_size, SEEK_SET);
    n = fread((*page)->data, 1, pager->page_size, pager->f);
    chilog(TRACE, "Read %i bytes from page %i into memory [%x data: %x]", n, npage, *page, (*page)->data);

    return CHIDB_OK;
}


/* Write a page to file
 *
 * This page writes the in-memory copy of a page (stored in a MemPage
 * struct) back to disk.
 *
 * Parameters
 * - pager: A Pager.
 * - page: In-memory copy of page to write
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EPAGENO: The page has an incorrect page number
 * - CHIDB_EIO: An I/O error has occurred when accessing the file
 */
int	chidb_Pager_writePage(Pager *pager, MemPage *page)
{
    if (page->npage > pager->n_pages)
        return CHIDB_EPAGENO;
    int n;
    fseek(pager->f, (page->npage - 1) * pager->page_size, SEEK_SET);
    n = fwrite(page->data, 1, pager->page_size, pager->f);
    chilog(TRACE, "Wrote %i bytes to page %i", n, page->npage);
    return CHIDB_OK;
}


/* Release an in-memory copy of a page
 *
 * Parameters
 * - pager: A Pager.
 * - page: In-memory copy of page to write
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EPAGENO: The page has an incorrect page number
 */
int	chidb_Pager_releaseMemPage(Pager *pager, MemPage *page)
{
    if (page->npage > pager->n_pages)
        return CHIDB_EPAGENO;

    chilog(TRACE, "Releasing page %i from memory [%x data: %x]", page->npage, page, page->data);
    free(page->data);
    free(page);

    return CHIDB_OK;
}


/* Computes the number of pages in a file.
 *
 * Parameters
 * - pager: A Pager.
 * - npages: Out parameter. Number of pages.
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_Pager_getRealDBSize(Pager *pager, npage_t *npages)
{
    struct stat buf;
    fstat(fileno(pager->f), &buf);
    *npages = buf.st_size / pager->page_size;

    return CHIDB_OK;
}


/* Closes a pager and frees up all resources used by the pager.
 *
 * Parameters
 * - pager: A Pager.
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EIO: An I/O error has occurred when accessing the file
 */
int chidb_Pager_close(Pager *pager)
{
    fclose(pager->f);
    free(pager);

    return CHIDB_OK;
}
