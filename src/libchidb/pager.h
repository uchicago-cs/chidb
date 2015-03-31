/*
 *  chidb - a didactic relational database management system
 *
 *  Pager header. See pager.c for more details.
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

#ifndef PAGER_H_
#define PAGER_H_

#include <stdio.h>
#include "chidbInt.h"

struct MemPage
{
    npage_t npage;
    uint8_t *data;
};
typedef struct MemPage MemPage;

struct Pager
{
    FILE *f;
    npage_t n_pages;
    uint16_t page_size;
};
typedef struct Pager Pager;

int chidb_Pager_open(Pager **pager, const char *filename);
int chidb_Pager_setPageSize(Pager *pager, uint16_t pagesize);
int chidb_Pager_readHeader(Pager *pager, uint8_t *header);
int chidb_Pager_allocatePage(Pager *pager, npage_t *npage);
int chidb_Pager_releaseMemPage(Pager *pager, MemPage *page);
int	chidb_Pager_readPage(Pager *pager, npage_t page_num, MemPage **page);
int chidb_Pager_writePage(Pager *pager, MemPage *page);
int chidb_Pager_getRealDBSize(Pager *pager, npage_t *npages);
int chidb_Pager_close(Pager *pager);

#endif /*PAGER_H_*/
