/*
 * Copyright (c) 2009-2012 Tony Bybell.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#include "fstapi.h"
#include "fastlz.h"

#ifndef HAVE_LIBPTHREAD
#undef FST_WRITER_PARALLEL
#endif

#ifdef FST_WRITER_PARALLEL
#include <pthread.h>
#endif

/* this define is to force writer backward compatibility with old readers */
#ifndef FST_DYNAMIC_ALIAS_DISABLE
/* note that Judy versus Jenkins requires more experimentation: they are  */
/* functionally equivalent though it appears Jenkins is slightly faster.  */
/* in addition, Jenkins is not bound by the LGPL.                         */
#ifdef _WAVE_HAVE_JUDY
#include <Judy.h>
#else
typedef const void *Pcvoid_t;
typedef void *Pvoid_t;
typedef void **PPvoid_t;
#define JudyHSIns(a,b,c,d) JenkinsIns((a),(b),(c),(hashmask))
#define JudyHSFreeArray(a,b) JenkinsFree((a),(hashmask))
void JenkinsFree(void *base_i, uint32_t hashmask);
void **JenkinsIns(void *base_i, unsigned char *mem, uint32_t length, uint32_t hashmask);
#endif
#endif


#undef  FST_DEBUG

#define FST_BREAK_SIZE 			(1UL << 27)
#define FST_BREAK_ADD_SIZE		(1UL << 22)
#define FST_BREAK_SIZE_MAX		(1UL << 31)
#define FST_ACTIVATE_HUGE_BREAK		(2000000)
#define FST_ACTIVATE_HUGE_INC		(2000000)

#define FST_WRITER_STR 			"fstWriter"
#define FST_ID_NAM_SIZ 			(512)
#define FST_DOUBLE_ENDTEST 		(2.7182818284590452354)
#define FST_HDR_SIM_VERSION_SIZE 	(128)
#define FST_HDR_DATE_SIZE 		(120)
#define FST_GZIO_LEN			(32768)

#if defined(__i386__) || defined(__x86_64__) || defined(_AIX)
#define FST_DO_MISALIGNED_OPS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define FST_MACOSX 
#endif


/***********************/
/***                 ***/
/*** common function ***/
/***                 ***/
/***********************/

#ifdef __MINGW32__
#include <io.h>
/* #define ftello ftell */
/* #define fseeko fseek */
#endif


/* the recoded "extra" values... */ 
#define FST_RCV_X (1 | (0<<1))
#define FST_RCV_Z (1 | (1<<1))
#define FST_RCV_H (1 | (2<<1))
#define FST_RCV_U (1 | (3<<1))
#define FST_RCV_W (1 | (4<<1))
#define FST_RCV_L (1 | (5<<1))
#define FST_RCV_D (1 | (6<<1))

#define FST_RCV_STR "xzhuwl-?"
/*                   01234567 */


/*
 * prevent old file overwrite when currently being read
 */
static FILE *unlink_fopen(const char *nam, const char *mode)
{
unlink(nam);
return(fopen(nam, mode));
}


/* 
 * to remove warn_unused_result compile time messages
 * (in the future there needs to be results checking)
 */
static size_t fstFread(void *buf, size_t siz, size_t cnt, FILE *fp)
{
return(fread(buf, siz, cnt, fp));
}

static size_t fstFwrite(const void *buf, size_t siz, size_t cnt, FILE *fp)
{
return(fwrite(buf, siz, cnt, fp));
}

static int fstFtruncate(int fd, off_t length)
{
return(ftruncate(fd, length));
}


/*
 * mmap compatibility
 */
#if defined __CYGWIN__ || defined __MINGW32__
#include <limits.h>
#define fstMmap(__addr,__len,__prot,__flags,__fd,__off) fstMmap2((__len), (__fd), (__off))
#define fstMunmap(__addr,__len)				free(__addr)

static void *fstMmap2(size_t __len, int __fd, off_t __off)
{
unsigned char *pnt = malloc(__len);
off_t cur_offs = lseek(__fd, 0, SEEK_CUR);
size_t i;

lseek(__fd, 0, SEEK_SET);
for(i=0;i<__len;i+=SSIZE_MAX)
	{
	read(__fd, pnt + i, ((__len - i) >= SSIZE_MAX) ? SSIZE_MAX : (__len - i));
	}
lseek(__fd, cur_offs, SEEK_SET);
return(pnt);
}
#else
#include <sys/mman.h>
#if defined(__SUNPRO_C)
#define FST_CADDR_T_CAST (caddr_t)
#else
#define FST_CADDR_T_CAST
#endif
#define fstMmap(__addr,__len,__prot,__flags,__fd,__off) (void*)mmap(FST_CADDR_T_CAST (__addr),(__len),(__prot),(__flags),(__fd),(__off))
#define fstMunmap(__addr,__len) 			{ if(__addr) munmap(FST_CADDR_T_CAST (__addr),(__len)); }
#endif


/*
 * regular and variable-length integer access functions
 */
#ifdef FST_DO_MISALIGNED_OPS
#define fstGetUint32(x) (*(uint32_t *)(x))
#define fstWriterSetUint32(x,y) (*(uint32_t *)(x)) = (y)
#else
static uint32_t fstGetUint32(unsigned char *mem)
{
uint32_t u32;
unsigned char *buf = (unsigned char *)(&u32);

buf[0] = mem[0];
buf[1] = mem[1];
buf[2] = mem[2];
buf[3] = mem[3];

return(*(uint32_t *)buf);
}


static void fstWriterSetUint32(unsigned char *mem, uint32_t u32)
{
unsigned char *buf = (unsigned char *)(&u32);

mem[0] = buf[0];
mem[1] = buf[1];
mem[2] = buf[2];
mem[3] = buf[3];
}
#endif


static int fstWriterUint64(FILE *handle, uint64_t v)
{
unsigned char buf[8];
int i;

for(i=7;i>=0;i--)
	{
	buf[i] = v & 0xff;
	v >>= 8;
	}

fstFwrite(buf, 8, 1, handle);
return(8);
}


static uint64_t fstReaderUint64(FILE *f)
{
uint64_t val = 0;
unsigned char buf[sizeof(uint64_t)];
int i;

fstFread(buf, sizeof(uint64_t), 1, f);
for(i=0;i<sizeof(uint64_t);i++)
	{
	val <<= 8;
	val |= buf[i];
	}

return(val);
}


static uint32_t fstGetVarint32(unsigned char *mem, int *skiplen)
{
unsigned char *mem_orig = mem;
uint32_t rc = 0;
while(*mem & 0x80)
	{
	mem++;
	}

*skiplen = mem - mem_orig + 1;
for(;;)
	{
	rc <<= 7;
	rc |= (uint32_t)(*mem & 0x7f);
	if(mem == mem_orig) 
		{
		break;
		}
	mem--;
	} 

return(rc);
}


static uint32_t fstGetVarint32Length(unsigned char *mem)
{
unsigned char *mem_orig = mem;

while(*mem & 0x80)
	{
	mem++;
	}

return(mem - mem_orig + 1);
}


static uint32_t fstGetVarint32NoSkip(unsigned char *mem)
{
unsigned char *mem_orig = mem;
uint32_t rc = 0;
while(*mem & 0x80)
        {
        mem++;
        }

for(;;)
        {
        rc <<= 7;
        rc |= (uint32_t)(*mem & 0x7f);
        if(mem == mem_orig)
                {
                break;
                }
        mem--;
        }
 
return(rc);       
}


static unsigned char *fstCopyVarint32ToLeft(unsigned char *pnt, uint32_t v)
{
unsigned char buf[5];
unsigned char *spnt = buf;  
uint32_t nxt;

while((nxt = v>>7))
        {
        *(spnt++) = (v&0x7f) | 0x80;
        v = nxt;
        }
*(spnt++) = (v&0x7f);

do      {
        *(--pnt) = *(--spnt);
        } while(spnt != buf);

return(pnt);
}
 

static uint64_t fstGetVarint64(unsigned char *mem, int *skiplen)
{
unsigned char *mem_orig = mem;
uint64_t rc = 0;
while(*mem & 0x80)
        {
        mem++;
        }

*skiplen = mem - mem_orig + 1;
for(;;)
        {
        rc <<= 7;
        rc |= (uint64_t)(*mem & 0x7f);
        if(mem == mem_orig)
                {
                break;
                }
        mem--;
        }
 
return(rc);       
}


static uint32_t fstReaderVarint32(FILE *f)
{
unsigned char buf[5];
unsigned char *mem = buf;
uint32_t rc = 0;
int ch;

do
	{
	ch = fgetc(f);
	*(mem++) = ch;
	} while(ch & 0x80);
mem--;

for(;;)
        {
        rc <<= 7;
        rc |= (uint32_t)(*mem & 0x7f);
        if(mem == buf)
                {
                break;
                }
        mem--;
        }
 
return(rc);       
}


static uint32_t fstReaderVarint32WithSkip(FILE *f, uint32_t *skiplen)
{
unsigned char buf[5];
unsigned char *mem = buf;
uint32_t rc = 0;
int ch;

do
	{
	ch = fgetc(f);
	*(mem++) = ch;
	} while(ch & 0x80);
*skiplen = mem - buf;
mem--;

for(;;)
        {
        rc <<= 7;
        rc |= (uint32_t)(*mem & 0x7f);
        if(mem == buf)
                {
                break;
                }
        mem--;
        }
 
return(rc);       
}


static uint64_t fstReaderVarint64(FILE *f)
{
unsigned char buf[16];
unsigned char *mem = buf;
uint64_t rc = 0;
int ch;

do
	{
	ch = fgetc(f);
	*(mem++) = ch;
	} while(ch & 0x80);
mem--;

for(;;)
        {
        rc <<= 7;
        rc |= (uint64_t)(*mem & 0x7f);
        if(mem == buf)
                {
                break;
                }
        mem--;
        }
 
return(rc);       
}


static int fstWriterVarint(FILE *handle, uint64_t v)
{
uint64_t nxt;
unsigned char buf[10]; /* ceil(64/7) = 10 */
unsigned char *pnt = buf;
int len;

while((nxt = v>>7))
        {
        *(pnt++) = (v&0x7f) | 0x80;
        v = nxt;
        }
*(pnt++) = (v&0x7f);

len = pnt-buf;
fstFwrite(buf, len, 1, handle);
return(len);
}


/***********************/
/***                 ***/
/*** writer function ***/
/***                 ***/
/***********************/

/*
 * private structs
 */
struct fstBlackoutChain
{
struct fstBlackoutChain *next;
uint64_t tim;
unsigned active : 1;
};


struct fstWriterContext
{
FILE *handle;
FILE *hier_handle;
FILE *geom_handle;
FILE *valpos_handle;
FILE *curval_handle;
FILE *tchn_handle;

unsigned char *vchg_mem;

off_t hier_file_len;

uint32_t *valpos_mem;
unsigned char *curval_mem;

char *filename;

fstHandle maxhandle;
fstHandle numsigs;
uint32_t maxvalpos;

unsigned vc_emitted : 1;
unsigned is_initial_time : 1;
unsigned fastpack : 1;

int64_t timezero;
off_t section_header_truncpos;
uint32_t tchn_cnt, tchn_idx;
uint64_t curtime;
uint64_t firsttime;
uint32_t vchg_siz;
uint32_t vchg_alloc_siz;


uint32_t secnum;
off_t section_start;

uint32_t numscopes;
double nan; /* nan value for uninitialized doubles */

struct fstBlackoutChain *blackout_head;
struct fstBlackoutChain *blackout_curr;
uint32_t num_blackouts;

uint64_t dump_size_limit;

unsigned compress_hier : 1;
unsigned repack_on_close : 1;
unsigned skip_writing_section_hdr : 1;
unsigned size_limit_locked : 1;
unsigned section_header_only : 1;
unsigned flush_context_pending : 1;
unsigned parallel_enabled : 1;
unsigned parallel_was_enabled : 1;

/* should really be semaphores, but are bytes to cut down on read-modify-write window size */
unsigned char already_in_flush; /* in case control-c handlers interrupt */
unsigned char already_in_close; /* in case control-c handlers interrupt */

#ifdef FST_WRITER_PARALLEL
pthread_mutex_t mutex;
pthread_t thread;
pthread_attr_t thread_attr;
struct fstWriterContext *xc_parent;
#endif

size_t fst_orig_break_size;
size_t fst_orig_break_add_size;

size_t fst_break_size;
size_t fst_break_add_size;

size_t fst_huge_break_size;

fstHandle next_huge_break;
};


static uint32_t fstWriterUint32WithVarint32(struct fstWriterContext *xc, uint32_t *u, uint32_t v, const void *dbuf, uint32_t siz)
{
unsigned char *buf = xc->vchg_mem + xc->vchg_siz;
unsigned char *pnt = buf;
uint32_t nxt;
uint32_t len;

#ifdef FST_DO_MISALIGNED_OPS
(*(uint32_t *)(pnt)) = (*(uint32_t *)(u));
#else
memcpy(pnt, u, sizeof(uint32_t));
#endif
pnt += 4;

while((nxt = v>>7))
        {
        *(pnt++) = (v&0x7f) | 0x80;
        v = nxt;
        }
*(pnt++) = (v&0x7f);
memcpy(pnt, dbuf, siz);

len = pnt-buf + siz;
return(len);
}


static uint32_t fstWriterUint32WithVarint32AndLength(struct fstWriterContext *xc, uint32_t *u, uint32_t v, const void *dbuf, uint32_t siz)
{
unsigned char *buf = xc->vchg_mem + xc->vchg_siz;
unsigned char *pnt = buf;
uint32_t nxt;
uint32_t len;

#ifdef FST_DO_MISALIGNED_OPS
(*(uint32_t *)(pnt)) = (*(uint32_t *)(u));
#else
memcpy(pnt, u, sizeof(uint32_t));
#endif
pnt += 4;

while((nxt = v>>7))
        {
        *(pnt++) = (v&0x7f) | 0x80;
        v = nxt;
        }
*(pnt++) = (v&0x7f);

v = siz;
while((nxt = v>>7))
        {
        *(pnt++) = (v&0x7f) | 0x80;
        v = nxt;
        }
*(pnt++) = (v&0x7f);

memcpy(pnt, dbuf, siz);

len = pnt-buf + siz;
return(len);
}


/*
 * header bytes, write here so defines are set up before anything else
 * that needs to use them
 */
static void fstWriterEmitHdrBytes(struct fstWriterContext *xc)
{
char vbuf[FST_HDR_SIM_VERSION_SIZE];
char dbuf[FST_HDR_DATE_SIZE];
double endtest = FST_DOUBLE_ENDTEST;
time_t walltime;

#define FST_HDR_OFFS_TAG 			(0)
fputc(FST_BL_HDR, xc->handle);			/* +0 tag */

#define FST_HDR_OFFS_SECLEN 			(FST_HDR_OFFS_TAG + 1)
fstWriterUint64(xc->handle, 329);		/* +1 section length */

#define FST_HDR_OFFS_START_TIME			(FST_HDR_OFFS_SECLEN + 8)
fstWriterUint64(xc->handle, 0);			/* +9 start time */

#define FST_HDR_OFFS_END_TIME			(FST_HDR_OFFS_START_TIME + 8)
fstWriterUint64(xc->handle, 0);  		/* +17 end time */

#define FST_HDR_OFFS_ENDIAN_TEST		(FST_HDR_OFFS_END_TIME + 8)
fstFwrite(&endtest, 8, 1, xc->handle); 		/* +25 endian test for reals */

#define FST_HDR_OFFS_MEM_USED			(FST_HDR_OFFS_ENDIAN_TEST + 8)
fstWriterUint64(xc->handle, xc->fst_break_size);/* +33 memory used by writer */

#define FST_HDR_OFFS_NUM_SCOPES			(FST_HDR_OFFS_MEM_USED + 8)
fstWriterUint64(xc->handle, 0);			/* +41 scope creation count */

#define FST_HDR_OFFS_NUM_VARS			(FST_HDR_OFFS_NUM_SCOPES + 8)
fstWriterUint64(xc->handle, 0);			/* +49 var creation count */

#define FST_HDR_OFFS_MAXHANDLE			(FST_HDR_OFFS_NUM_VARS + 8)
fstWriterUint64(xc->handle, 0);			/* +57 max var idcode */

#define FST_HDR_OFFS_SECTION_CNT		(FST_HDR_OFFS_MAXHANDLE + 8)
fstWriterUint64(xc->handle, 0);			/* +65 vc section count */

#define FST_HDR_OFFS_TIMESCALE			(FST_HDR_OFFS_SECTION_CNT + 8)
fputc((-9)&255, xc->handle);			/* +73 timescale 1ns */

#define FST_HDR_OFFS_SIM_VERSION		(FST_HDR_OFFS_TIMESCALE + 1)
memset(vbuf, 0, FST_HDR_SIM_VERSION_SIZE);
strcpy(vbuf, FST_WRITER_STR);
fstFwrite(vbuf, FST_HDR_SIM_VERSION_SIZE, 1, xc->handle); /* +74 version */

#define FST_HDR_OFFS_DATE			(FST_HDR_OFFS_SIM_VERSION + FST_HDR_SIM_VERSION_SIZE)
memset(dbuf, 0, FST_HDR_DATE_SIZE);
time(&walltime);
strcpy(dbuf, asctime(localtime(&walltime)));
fstFwrite(dbuf, FST_HDR_DATE_SIZE, 1, xc->handle);	/* +202 date */

/* date size is deliberately overspecified at 120 bytes in order to provide backfill for new args */

#define FST_HDR_OFFS_TIMEZERO			(FST_HDR_OFFS_DATE + FST_HDR_DATE_SIZE)
fstWriterUint64(xc->handle, xc->timezero);	/* +322 timezero */

#define FST_HDR_LENGTH				(FST_HDR_OFFS_TIMEZERO + 8)
						/* +330 next section starts here */
fflush(xc->handle);
}


/*
 * mmap functions
 */
static void fstWriterCreateMmaps(struct fstWriterContext *xc)
{
off_t curpos = ftello(xc->handle);

fflush(xc->hier_handle);

/* write out intermediate header */
fseeko(xc->handle, FST_HDR_OFFS_START_TIME, SEEK_SET);
fstWriterUint64(xc->handle, xc->firsttime);
fstWriterUint64(xc->handle, xc->curtime);
fseeko(xc->handle, FST_HDR_OFFS_NUM_SCOPES, SEEK_SET);
fstWriterUint64(xc->handle, xc->numscopes);
fstWriterUint64(xc->handle, xc->numsigs);
fstWriterUint64(xc->handle, xc->maxhandle);
fstWriterUint64(xc->handle, xc->secnum);
fseeko(xc->handle, curpos, SEEK_SET);
fflush(xc->handle);

/* do mappings */
if(!xc->valpos_mem)
	{
	fflush(xc->valpos_handle);
	xc->valpos_mem = fstMmap(NULL, xc->maxhandle * 4 * sizeof(uint32_t), PROT_READ|PROT_WRITE, MAP_SHARED, fileno(xc->valpos_handle), 0);
	}
if(!xc->curval_mem)
	{
	fflush(xc->curval_handle);
	xc->curval_mem = fstMmap(NULL, xc->maxvalpos, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(xc->curval_handle), 0);
	}
}


static void fstDestroyMmaps(struct fstWriterContext *xc, int is_closing)
{
fstMunmap(xc->valpos_mem, xc->maxhandle * 4 * sizeof(uint32_t));
xc->valpos_mem = NULL;

#if defined __CYGWIN__ || defined __MINGW32__
if(xc->curval_mem)
	{
	if(!is_closing) /* need to flush out for next emulated mmap() read */
		{
		unsigned char *pnt = xc->curval_mem;
		int __fd = fileno(xc->curval_handle);
		off_t cur_offs = lseek(__fd, 0, SEEK_CUR);
		size_t i;
		size_t __len = xc->maxvalpos;

		lseek(__fd, 0, SEEK_SET);
		for(i=0;i<__len;i+=SSIZE_MAX)
		        {
		        write(__fd, pnt + i, ((__len - i) >= SSIZE_MAX) ? SSIZE_MAX : (__len - i));
		        }
		lseek(__fd, cur_offs, SEEK_SET);
		}
	}
#endif

fstMunmap(xc->curval_mem, xc->maxvalpos);
xc->curval_mem = NULL;
}


/*
 * set up large and small memory usages
 * crossover point in model is FST_ACTIVATE_HUGE_BREAK number of signals
 */
static void fstDetermineBreakSize(struct fstWriterContext *xc)
{
#if defined(__linux__) || defined(FST_MACOSX)

#ifdef __linux__
FILE *f = fopen("/proc/meminfo", "rb");
#else
FILE *f = popen("system_profiler", "r");
#endif

int was_set = 0;

if(f)
	{
	char buf[257];
	char *s;
	while(!feof(f))
		{
		buf[0] = 0;
		s = fgets(buf, 256, f);
		if(s && *s)
			{
#ifdef __linux__
			if(!strncmp(s, "MemTotal:", 9))
				{
				size_t v = atol(s+10);
				v *= 1024; /* convert to bytes */
#else
			if((s=strstr(s, "Memory:")))
				{
				size_t v = atol(s+7);
				v <<= 30; /* convert GB to bytes */
#endif

				v /= 8; /* chop down to 1/8 physical memory */
				if(v > FST_BREAK_SIZE)
					{
					if(v > FST_BREAK_SIZE_MAX)
						{
						v = FST_BREAK_SIZE_MAX;
						}

					xc->fst_huge_break_size = v;
					was_set = 1;
					break;
					}
				}
			}
		}

#ifdef __linux__
	fclose(f);
#else
	pclose(f);
#endif
	} 

if(!was_set)
#endif
	{
	xc->fst_huge_break_size = FST_BREAK_SIZE;
	}

xc->fst_break_size = xc->fst_orig_break_size = FST_BREAK_SIZE;
xc->fst_break_add_size = xc->fst_orig_break_add_size = FST_BREAK_ADD_SIZE;
xc->next_huge_break = FST_ACTIVATE_HUGE_BREAK;
}


/*
 * file creation and close
 */
void *fstWriterCreate(const char *nam, int use_compressed_hier)
{
struct fstWriterContext *xc = calloc(1, sizeof(struct fstWriterContext));

xc->compress_hier = use_compressed_hier;
fstDetermineBreakSize(xc);

if((!nam)||(!(xc->handle=unlink_fopen(nam, "w+b"))))
        {
        free(xc);
        xc=NULL;
        }
        else
        {
	int flen = strlen(nam);
	char *hf = calloc(1, flen + 6);

	memcpy(hf, nam, flen);
	strcpy(hf + flen, ".hier");
	xc->hier_handle = unlink_fopen(hf, "w+b");

	xc->geom_handle = tmpfile();	/* .geom */
	xc->valpos_handle = tmpfile();	/* .offs */
	xc->curval_handle = tmpfile();	/* .bits */
	xc->tchn_handle = tmpfile();	/* .tchn */
	xc->vchg_alloc_siz = xc->fst_break_size + xc->fst_break_add_size;
	xc->vchg_mem = malloc(xc->vchg_alloc_siz);

	free(hf);
	if(xc->hier_handle && xc->geom_handle && xc->valpos_handle && xc->curval_handle && xc->vchg_mem && xc->tchn_handle)
		{
	        xc->filename = strdup(nam);
		xc->is_initial_time = 1;

		fstWriterEmitHdrBytes(xc);
		xc->nan = strtod("NaN", NULL);
#ifdef FST_WRITER_PARALLEL
		pthread_mutex_init(&xc->mutex, NULL);
		pthread_attr_init(&xc->thread_attr);
		pthread_attr_setdetachstate(&xc->thread_attr, PTHREAD_CREATE_DETACHED);
#endif
		}
		else
		{
		if(xc->hier_handle) fclose(xc->hier_handle);
		if(xc->geom_handle) fclose(xc->geom_handle);
		if(xc->valpos_handle) fclose(xc->valpos_handle);
		if(xc->curval_handle) fclose(xc->curval_handle);
		if(xc->tchn_handle) fclose(xc->tchn_handle);
		free(xc->vchg_mem);
	        free(xc);
	        xc=NULL;
		}
	}

return(xc);
}


/*
 * generation and writing out of value change data sections
 */
static void fstWriterEmitSectionHeader(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

if(xc)
	{
	unsigned long destlen;
	unsigned char *dmem;
        int rc;

	destlen = xc->maxvalpos;
	dmem = malloc(destlen);
        rc = compress2(dmem, &destlen, xc->curval_mem, xc->maxvalpos, 9);

	fputc(FST_BL_SKIP, xc->handle);			/* temporarily tag the section, use FST_BL_VCDATA on finalize */
	xc->section_start = ftello(xc->handle);
#ifdef FST_WRITER_PARALLEL
	if(xc->xc_parent) xc->xc_parent->section_start = xc->section_start;
#endif
	xc->section_header_only = 1;			/* indicates truncate might be needed */
	fstWriterUint64(xc->handle, 0); 		/* placeholder = section length */
	fstWriterUint64(xc->handle, xc->is_initial_time ? xc->firsttime : xc->curtime); 	/* begin time of section */
	fstWriterUint64(xc->handle, xc->curtime); 	/* end time of section (placeholder) */
	fstWriterUint64(xc->handle, 0);			/* placeholder = amount of buffer memory required in reader for full vc traversal */
	fstWriterVarint(xc->handle, xc->maxvalpos);	/* maxvalpos = length of uncompressed data */

	if((rc == Z_OK) && (destlen < xc->maxvalpos))
		{
		fstWriterVarint(xc->handle, destlen);	/* length of compressed data */
		}
		else
		{
		fstWriterVarint(xc->handle, xc->maxvalpos); /* length of (unable to be) compressed data */
		}
	fstWriterVarint(xc->handle, xc->maxhandle);	/* max handle associated with this data (in case of dynamic facility adds) */

	if((rc == Z_OK) && (destlen < xc->maxvalpos))
		{
		fstFwrite(dmem, destlen, 1, xc->handle);
		}
		else /* comparison between compressed / decompressed len tells if compressed */
		{
		fstFwrite(xc->curval_mem, xc->maxvalpos, 1, xc->handle);
		}

	free(dmem);
	}
}


/*
 * only to be called directly by fst code...otherwise must
 * be synced up with time changes
 */
#ifdef FST_WRITER_PARALLEL
static void fstWriterFlushContextPrivate2(void *ctx)
#else
static void fstWriterFlushContextPrivate(void *ctx)
#endif
{
#ifdef FST_DEBUG
int cnt = 0;
#endif
int i;
unsigned char *vchg_mem;
FILE *f;
off_t fpos, indxpos, endpos;
uint32_t prevpos;
int zerocnt;
unsigned char *scratchpad;
unsigned char *scratchpnt;
unsigned char *tmem;
off_t tlen;
off_t unc_memreq = 0; /* for reader */
unsigned char *packmem;
unsigned int packmemlen;
uint32_t *vm4ip;
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
#ifdef FST_WRITER_PARALLEL
struct fstWriterContext *xc2 = xc->xc_parent;
#else
struct fstWriterContext *xc2 = xc;
#endif

#ifndef FST_DYNAMIC_ALIAS_DISABLE
Pvoid_t PJHSArray = (Pvoid_t) NULL;
#ifndef _WAVE_HAVE_JUDY
uint32_t hashmask =  xc->maxhandle;
hashmask |= hashmask >> 1;
hashmask |= hashmask >> 2;
hashmask |= hashmask >> 4;
hashmask |= hashmask >> 8;
hashmask |= hashmask >> 16;
#endif
#endif

if((!xc)||(xc->vchg_siz <= 1)||(xc->already_in_flush)) return;
xc->already_in_flush = 1; /* should really do this with a semaphore */

xc->section_header_only = 0;
scratchpad = malloc(xc->vchg_siz);

vchg_mem = xc->vchg_mem;

f = xc->handle;
fstWriterVarint(f, xc->maxhandle);	/* emit current number of handles */
fputc(xc->fastpack ? 'F' : 'Z', f);
fpos = 1;

packmemlen = 1024;			/* maintain a running "longest" allocation to */
packmem = malloc(packmemlen);		/* prevent continual malloc...free every loop iter */

for(i=0;i<xc->maxhandle;i++)
	{
	vm4ip = &(xc->valpos_mem[4*i]);

	if(vm4ip[2]) 
		{
		uint32_t offs = vm4ip[2];
		uint32_t next_offs;
		int wrlen;

		vm4ip[2] = fpos;

		scratchpnt = scratchpad + xc->vchg_siz;		/* build this buffer backwards */
		if(vm4ip[1] <= 1)
			{
			if(vm4ip[1] == 1)
				{
				wrlen = fstGetVarint32Length(vchg_mem + offs + 4); /* used to advance and determine wrlen */
#ifndef FST_REMOVE_DUPLICATE_VC
				xc->curval_mem[vm4ip[0]] = vchg_mem[offs + 4 + wrlen]; /* checkpoint variable */
#endif
	                        while(offs)
	                                {
	                                unsigned char val;
	                                uint32_t time_delta, rcv;
	                                next_offs = fstGetUint32(vchg_mem + offs);
	                                offs += 4;   
                        
	                                time_delta = fstGetVarint32(vchg_mem + offs, &wrlen);
	                                val = vchg_mem[offs+wrlen];
					offs = next_offs;

	                                switch(val)
	                                        {
	                                        case '0':
	                                        case '1':               rcv = ((val&1)<<1) | (time_delta<<2);
	                                                                break; /* pack more delta bits in for 0/1 vchs */
	        
	                                        case 'x': case 'X':     rcv = FST_RCV_X | (time_delta<<4); break;
	                                        case 'z': case 'Z':     rcv = FST_RCV_Z | (time_delta<<4); break;
	                                        case 'h': case 'H':     rcv = FST_RCV_H | (time_delta<<4); break;
	                                        case 'u': case 'U':     rcv = FST_RCV_U | (time_delta<<4); break;
	                                        case 'w': case 'W':     rcv = FST_RCV_W | (time_delta<<4); break;
	                                        case 'l': case 'L':     rcv = FST_RCV_L | (time_delta<<4); break;
	                                        default:                rcv = FST_RCV_D | (time_delta<<4); break;
	                                        }
	                
	                                scratchpnt = fstCopyVarint32ToLeft(scratchpnt, rcv);
					}
				}
				else
				{
				/* variable length */
				/* fstGetUint32 (next_offs) + fstGetVarint32 (time_delta) + fstGetVarint32 (len) + payload */
				unsigned char *pnt;
				uint32_t record_len;
				uint32_t time_delta;				

				while(offs)
					{
					next_offs = fstGetUint32(vchg_mem + offs);
					offs += 4;
					pnt = vchg_mem + offs;
					offs = next_offs;
					time_delta = fstGetVarint32(pnt, &wrlen);
					pnt += wrlen;
					record_len = fstGetVarint32(pnt, &wrlen);
					pnt += wrlen;

					scratchpnt -= record_len;
					memcpy(scratchpnt, pnt, record_len);

					scratchpnt = fstCopyVarint32ToLeft(scratchpnt, record_len);
					scratchpnt = fstCopyVarint32ToLeft(scratchpnt, (time_delta << 1)); /* reserve | 1 case for future expansion */
					}
				}
			}
			else
			{
			wrlen = fstGetVarint32Length(vchg_mem + offs + 4); /* used to advance and determine wrlen */
#ifndef FST_REMOVE_DUPLICATE_VC
			memcpy(xc->curval_mem + vm4ip[0], vchg_mem + offs + 4 + wrlen, vm4ip[1]); /* checkpoint variable */
#endif
			while(offs)
				{
				int idx;
				char is_binary = 1;
				unsigned char *pnt;
				uint32_t time_delta;

				next_offs = fstGetUint32(vchg_mem + offs);
				offs += 4;

				time_delta = fstGetVarint32(vchg_mem + offs, &wrlen);

				pnt = vchg_mem+offs+wrlen;
				offs = next_offs;

				for(idx=0;idx<vm4ip[1];idx++)
					{
					if((pnt[idx] == '0') || (pnt[idx] == '1'))
						{
						continue;
						}
						else
						{
						is_binary = 0;
						break;
						}
					}

				if(is_binary)
					{
					unsigned char acc = 0;
					int shift = 7 - ((vm4ip[1]-1) & 7);
					for(idx=vm4ip[1]-1;idx>=0;idx--)
						{
						acc |= (pnt[idx] & 1) << shift;
						shift++;
						if(shift == 8)
							{
							*(--scratchpnt) = acc;
							shift = 0;
							acc = 0;
							}						
						}					

	                                scratchpnt = fstCopyVarint32ToLeft(scratchpnt, (time_delta << 1));
					}
					else
					{
					scratchpnt -= vm4ip[1];
					memcpy(scratchpnt, pnt, vm4ip[1]);

	                                scratchpnt = fstCopyVarint32ToLeft(scratchpnt, (time_delta << 1) | 1);
					}
				}
			}

		wrlen = scratchpad + xc->vchg_siz - scratchpnt;
		unc_memreq += wrlen;
		if(wrlen > 32)
			{
			unsigned long destlen = wrlen;
			unsigned char *dmem;
		        int rc;

			if(!xc->fastpack)
				{
				if(wrlen <= packmemlen)
					{
					dmem = packmem;
					}
					else
					{
					free(packmem);
					dmem = packmem = malloc(packmemlen = wrlen);
					}

		        	rc = compress2(dmem, &destlen, scratchpnt, wrlen, 4);
				if(rc == Z_OK)
					{
#ifndef FST_DYNAMIC_ALIAS_DISABLE
					PPvoid_t pv = JudyHSIns(&PJHSArray, dmem, destlen, NULL);
					if(*pv)
						{
						uint32_t pvi = (long)(*pv);
						vm4ip[2] = -pvi;
						}
						else
						{
						*pv = (void *)(long)(i+1);
#endif
						fpos += fstWriterVarint(f, wrlen);
						fpos += destlen;
						fstFwrite(dmem, destlen, 1, f);
#ifndef FST_DYNAMIC_ALIAS_DISABLE
						}
#endif
					}
					else
					{
#ifndef FST_DYNAMIC_ALIAS_DISABLE
					PPvoid_t pv = JudyHSIns(&PJHSArray, scratchpnt, wrlen, NULL);
					if(*pv)
						{
						uint32_t pvi = (long)(*pv);
						vm4ip[2] = -pvi;
						}
						else
						{
						*pv = (void *)(long)(i+1);
#endif
						fpos += fstWriterVarint(f, 0);
						fpos += wrlen;
						fstFwrite(scratchpnt, wrlen, 1, f);
#ifndef FST_DYNAMIC_ALIAS_DISABLE
						}
#endif
					}
				}
				else
				{
				if(((wrlen * 2) + 2) <= packmemlen)
					{
					dmem = packmem;
					}
					else
					{
					free(packmem);
					dmem = packmem = malloc(packmemlen = (wrlen * 2) + 2);
					}

				rc = fastlz_compress(scratchpnt, wrlen, dmem);
				if(rc < destlen)
        				{
#ifndef FST_DYNAMIC_ALIAS_DISABLE
					PPvoid_t pv = JudyHSIns(&PJHSArray, dmem, rc, NULL);
					if(*pv)
						{
						uint32_t pvi = (long)(*pv);
						vm4ip[2] = -pvi;
						}
						else
						{
						*pv = (void *)(long)(i+1);
#endif
						fpos += fstWriterVarint(f, wrlen);
						fpos += rc;
						fstFwrite(dmem, rc, 1, f);
#ifndef FST_DYNAMIC_ALIAS_DISABLE
						}
#endif
        				}
        				else
        				{
#ifndef FST_DYNAMIC_ALIAS_DISABLE
					PPvoid_t pv = JudyHSIns(&PJHSArray, scratchpnt, wrlen, NULL);
					if(*pv)
						{
						uint32_t pvi = (long)(*pv);
						vm4ip[2] = -pvi;
						}
						else
						{
						*pv = (void *)(long)(i+1);
#endif
						fpos += fstWriterVarint(f, 0);
						fpos += wrlen;
						fstFwrite(scratchpnt, wrlen, 1, f);
#ifndef FST_DYNAMIC_ALIAS_DISABLE
						}
#endif
        				}
				}
			}
			else
			{
#ifndef FST_DYNAMIC_ALIAS_DISABLE
			PPvoid_t pv = JudyHSIns(&PJHSArray, scratchpnt, wrlen, NULL);
			if(*pv)
				{
				uint32_t pvi = (long)(*pv);
				vm4ip[2] = -pvi;
				}
				else
				{
				*pv = (void *)(long)(i+1);
#endif
				fpos += fstWriterVarint(f, 0);
				fpos += wrlen;
				fstFwrite(scratchpnt, wrlen, 1, f);
#ifndef FST_DYNAMIC_ALIAS_DISABLE
				}
#endif
			}

		/* vm4ip[3] = 0; ...redundant with clearing below */
#ifdef FST_DEBUG
		cnt++;
#endif
		}
	}

#ifndef FST_DYNAMIC_ALIAS_DISABLE
JudyHSFreeArray(&PJHSArray, NULL);
#endif

free(packmem); packmem = NULL; /* packmemlen = 0; */ /* scan-build */

prevpos = 0; zerocnt = 0;
free(scratchpad); scratchpad = NULL;

indxpos = ftello(f);
xc->secnum++;

for(i=0;i<xc->maxhandle;i++)
	{
	vm4ip = &(xc->valpos_mem[4*i]);

	if(vm4ip[2])
		{
		if(zerocnt)
			{
			fpos += fstWriterVarint(f, (zerocnt << 1));
			zerocnt = 0;
			}

		if(vm4ip[2] & 0x80000000)
			{
			fpos += fstWriterVarint(f, 0); /* signal */
			fpos += fstWriterVarint(f, (-(int32_t)vm4ip[2]));
			}
			else
			{
			fpos += fstWriterVarint(f, ((vm4ip[2] - prevpos) << 1) | 1);
			prevpos = vm4ip[2];
			}
		vm4ip[2] = 0;
		vm4ip[3] = 0; /* clear out tchn idx */
		}
		else
		{
		zerocnt++;		
		}
	}
if(zerocnt)
	{
	/* fpos += */ fstWriterVarint(f, (zerocnt << 1)); /* scan-build */
	}
#ifdef FST_DEBUG
printf("value chains: %d\n", cnt);
#endif

xc->vchg_mem[0] = '!';
xc->vchg_siz = 1;

endpos = ftello(xc->handle);
fstWriterUint64(xc->handle, endpos-indxpos);		/* write delta index position at very end of block */

/*emit time changes for block */
fflush(xc->tchn_handle);
tlen = ftello(xc->tchn_handle);
fseeko(xc->tchn_handle, 0, SEEK_SET);

tmem = fstMmap(NULL, tlen, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(xc->tchn_handle), 0);
if(tmem)
	{
	unsigned long destlen = tlen;
	unsigned char *dmem = malloc(destlen);
        int rc = compress2(dmem, &destlen, tmem, tlen, 9);

	if((rc == Z_OK) && (destlen < tlen))
		{
		fstFwrite(dmem, destlen, 1, xc->handle);
		}
		else /* comparison between compressed / decompressed len tells if compressed */
		{
		fstFwrite(tmem, tlen, 1, xc->handle);
		destlen = tlen;
		}
	free(dmem);
	fstMunmap(tmem, tlen);
	fstWriterUint64(xc->handle, tlen);		/* uncompressed */
	fstWriterUint64(xc->handle, destlen);		/* compressed */
	fstWriterUint64(xc->handle, xc->tchn_cnt); 	/* number of time items */
	}

xc->tchn_cnt = xc->tchn_idx = 0;
fseeko(xc->tchn_handle, 0, SEEK_SET);
fstFtruncate(fileno(xc->tchn_handle), 0);

/* write block trailer */
endpos = ftello(xc->handle);
fseeko(xc->handle, xc->section_start, SEEK_SET);
fstWriterUint64(xc->handle, endpos - xc->section_start); 	/* write block length */
fseeko(xc->handle, 8, SEEK_CUR);				/* skip begin time */
fstWriterUint64(xc->handle, xc->curtime); 			/* write end time for section */
fstWriterUint64(xc->handle, unc_memreq);			/* amount of buffer memory required in reader for full traversal */
fflush(xc->handle);

fseeko(xc->handle, xc->section_start-1, SEEK_SET);		/* write out FST_BL_VCDATA over FST_BL_SKIP */

#ifndef FST_DYNAMIC_ALIAS_DISABLE
fputc(FST_BL_VCDATA_DYN_ALIAS, xc->handle);
#else
fputc(FST_BL_VCDATA, xc->handle);
#endif

fflush(xc->handle);

fseeko(xc->handle, endpos, SEEK_SET);				/* seek to end of file */

xc2->section_header_truncpos = endpos;				/* cache in case of need to truncate */
if(xc->dump_size_limit)
	{
	if(endpos >= xc->dump_size_limit)
		{
		xc2->skip_writing_section_hdr = 1;
		xc2->size_limit_locked = 1;
		xc2->is_initial_time = 1; /* to trick emit value and emit time change */
#ifdef FST_DEBUG
		printf("<< dump file size limit reached, stopping dumping >>\n");
#endif
		}
	}

if(!xc2->skip_writing_section_hdr)
	{
	fstWriterEmitSectionHeader(xc);				/* emit next section header */
	}
fflush(xc->handle);

xc->already_in_flush = 0;
}


#ifdef FST_WRITER_PARALLEL
static void *fstWriterFlushContextPrivate1(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

fstWriterFlushContextPrivate2(xc);

pthread_mutex_unlock(&(xc->xc_parent->mutex));

#ifdef FST_REMOVE_DUPLICATE_VC
free(xc->curval_mem);
#endif
free(xc->valpos_mem);
free(xc->vchg_mem);
fclose(xc->tchn_handle);
free(xc);

return(NULL);
}


static void fstWriterFlushContextPrivate(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

if(xc->parallel_enabled)
	{
	struct fstWriterContext *xc2 = malloc(sizeof(struct fstWriterContext));
	int i;

	pthread_mutex_lock(&xc->mutex);
	pthread_mutex_unlock(&xc->mutex);

	xc->xc_parent = xc;
	memcpy(xc2, xc, sizeof(struct fstWriterContext));

	xc2->valpos_mem = malloc(xc->maxhandle * 4 * sizeof(uint32_t));
	memcpy(xc2->valpos_mem, xc->valpos_mem, xc->maxhandle * 4 * sizeof(uint32_t));

	/* curval mem is updated in the thread */
#ifdef FST_REMOVE_DUPLICATE_VC
	xc2->curval_mem = malloc(xc->maxvalpos);
	memcpy(xc2->curval_mem, xc->curval_mem, xc->maxvalpos);
#endif

	xc->vchg_mem = malloc(xc->vchg_alloc_siz);
	xc->vchg_mem[0] = '!';
	xc->vchg_siz = 1;

	for(i=0;i<xc->maxhandle;i++)
		{
		uint32_t *vm4ip = &(xc->valpos_mem[4*i]);
	        vm4ip[2] = 0; /* zero out offset val */
	        vm4ip[3] = 0; /* zero out last time change val */
	        }

	xc->tchn_cnt = xc->tchn_idx = 0;
	xc->tchn_handle = tmpfile();
	fseeko(xc->tchn_handle, 0, SEEK_SET);
	fstFtruncate(fileno(xc->tchn_handle), 0);

	xc->section_header_only = 0;
	xc->secnum++;

	pthread_mutex_lock(&xc->mutex);

	pthread_create(&xc->thread, &xc->thread_attr, fstWriterFlushContextPrivate1, xc2);
	}
	else
	{
	if(xc->parallel_was_enabled) /* conservatively block */
		{
		pthread_mutex_lock(&xc->mutex);
		pthread_mutex_unlock(&xc->mutex);
		}

	xc->xc_parent = xc;
	fstWriterFlushContextPrivate2(xc);
	}
}
#endif


/*
 * queues up a flush context operation
 */
void fstWriterFlushContext(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
        {
	if(xc->tchn_idx > 1)
		{
		xc->flush_context_pending = 1;
		}
	}
}


/*
 * close out FST file
 */
void fstWriterClose(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

#ifdef FST_WRITER_PARALLEL
if(xc)
	{
	pthread_mutex_lock(&xc->mutex);
	pthread_mutex_unlock(&xc->mutex);
	}
#endif

if(xc && !xc->already_in_close && !xc->already_in_flush)
	{
	unsigned char *tmem;
	off_t fixup_offs, tlen, hlen;

	xc->already_in_close = 1; /* never need to zero this out as it is freed at bottom */

	if(xc->section_header_only && xc->section_header_truncpos && (xc->vchg_siz <= 1) && (!xc->is_initial_time))
		{
		fstFtruncate(fileno(xc->handle), xc->section_header_truncpos);
		fseeko(xc->handle, xc->section_header_truncpos, SEEK_SET);
		xc->section_header_only = 0;
		}
		else
		{
		xc->skip_writing_section_hdr = 1;
		if(!xc->size_limit_locked)
			{
			if(xc->is_initial_time) /* simulation time never advanced so mock up the changes as time zero ones */
				{
				fstHandle dupe_idx;
	
				fstWriterEmitTimeChange(xc, 0); /* emit some time change just to have one */
				for(dupe_idx = 0; dupe_idx < xc->maxhandle; dupe_idx++) /* now clone the values */
					{
					fstWriterEmitValueChange(xc, dupe_idx+1, xc->curval_mem + xc->valpos_mem[4*dupe_idx]);
					}
				}
			fstWriterFlushContextPrivate(xc);
#ifdef FST_WRITER_PARALLEL
			pthread_mutex_lock(&xc->mutex);
			pthread_mutex_unlock(&xc->mutex);
#endif
			}
		}
	fstDestroyMmaps(xc, 1);

	/* write out geom section */
	fflush(xc->geom_handle);
	tlen = ftello(xc->geom_handle);
	tmem = fstMmap(NULL, tlen, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(xc->geom_handle), 0);
	if(tmem)
		{
		unsigned long destlen = tlen;
		unsigned char *dmem = malloc(destlen);
	        int rc = compress2(dmem, &destlen, tmem, tlen, 9);

		if((rc != Z_OK) || (destlen > tlen))
			{
			destlen = tlen;
			}

		fixup_offs = ftello(xc->handle);
		fputc(FST_BL_SKIP, xc->handle);			/* temporary tag */
		fstWriterUint64(xc->handle, destlen + 24);	/* section length */
		fstWriterUint64(xc->handle, tlen);		/* uncompressed */
								/* compressed len is section length - 24 */
		fstWriterUint64(xc->handle, xc->maxhandle);	/* maxhandle */
		fstFwrite((destlen != tlen) ? dmem : tmem, destlen, 1, xc->handle);
		fflush(xc->handle);

		fseeko(xc->handle, fixup_offs, SEEK_SET);
		fputc(FST_BL_GEOM, xc->handle);			/* actual tag */

		fseeko(xc->handle, 0, SEEK_END);		/* move file pointer to end for any section adds */
		fflush(xc->handle);

		free(dmem);
		fstMunmap(tmem, tlen);
		}

	if(xc->num_blackouts)
		{
		uint64_t cur_bl = 0;
		off_t bpos, eos;
		uint32_t i;

		fixup_offs = ftello(xc->handle);
		fputc(FST_BL_SKIP, xc->handle);			/* temporary tag */
		bpos = fixup_offs + 1;
		fstWriterUint64(xc->handle, 0);			/* section length */
		fstWriterVarint(xc->handle, xc->num_blackouts);

		for(i=0;i<xc->num_blackouts;i++)
			{
			fputc(xc->blackout_head->active, xc->handle);
			fstWriterVarint(xc->handle, xc->blackout_head->tim - cur_bl);
			cur_bl = xc->blackout_head->tim;
			xc->blackout_curr = xc->blackout_head->next;
			free(xc->blackout_head);
			xc->blackout_head = xc->blackout_curr;	
			}

		eos = ftello(xc->handle);
		fseeko(xc->handle, bpos, SEEK_SET);
		fstWriterUint64(xc->handle, eos - bpos);		
		fflush(xc->handle);

		fseeko(xc->handle, fixup_offs, SEEK_SET);
		fputc(FST_BL_BLACKOUT, xc->handle);	/* actual tag */

		fseeko(xc->handle, 0, SEEK_END);	/* move file pointer to end for any section adds */
		fflush(xc->handle);
		}

	if(xc->compress_hier)
		{
		unsigned char *mem = malloc(FST_GZIO_LEN);
		off_t hl, eos;
		gzFile zhandle;
		int zfd;
#ifndef __MINGW32__
		char *fnam = malloc(strlen(xc->filename) + 5 + 1);
#endif

		fixup_offs = ftello(xc->handle);
		fputc(FST_BL_SKIP, xc->handle);			/* temporary tag */
		hlen = ftello(xc->handle);
		fstWriterUint64(xc->handle, 0);			/* section length */
		fstWriterUint64(xc->handle, xc->hier_file_len);	/* uncompressed length */
		
		fflush(xc->handle);
		zfd = dup(fileno(xc->handle));
		zhandle = gzdopen(zfd, "wb4");
		if(zhandle)
			{
			fseeko(xc->hier_handle, 0, SEEK_SET);
			for(hl = 0; hl < xc->hier_file_len; hl += FST_GZIO_LEN)
				{
				unsigned len = ((xc->hier_file_len - hl) > FST_GZIO_LEN) ? FST_GZIO_LEN : (xc->hier_file_len - hl);
				fstFread(mem, len, 1, xc->hier_handle);
				gzwrite(zhandle, mem, len);
				}
			gzclose(zhandle);
			}
			else
			{
			close(zfd);
			}
		free(mem);

		fseeko(xc->handle, 0, SEEK_END);
		eos = ftello(xc->handle);
		fseeko(xc->handle, hlen, SEEK_SET);
		fstWriterUint64(xc->handle, eos - hlen);
		fflush(xc->handle);

		fseeko(xc->handle, fixup_offs, SEEK_SET);
		fputc(FST_BL_HIER, xc->handle);		/* actual tag */

		fseeko(xc->handle, 0, SEEK_END);	/* move file pointer to end for any section adds */
		fflush(xc->handle);

#ifndef __MINGW32__
		sprintf(fnam, "%s.hier", xc->filename);
		unlink(fnam);
		free(fnam);
#endif
		}

	/* finalize out header */
	fseeko(xc->handle, FST_HDR_OFFS_START_TIME, SEEK_SET);
	fstWriterUint64(xc->handle, xc->firsttime);
	fstWriterUint64(xc->handle, xc->curtime);
	fseeko(xc->handle, FST_HDR_OFFS_NUM_SCOPES, SEEK_SET);
	fstWriterUint64(xc->handle, xc->numscopes);
	fstWriterUint64(xc->handle, xc->numsigs);
	fstWriterUint64(xc->handle, xc->maxhandle);
	fstWriterUint64(xc->handle, xc->secnum);
	fflush(xc->handle);
	
	if(xc->tchn_handle) { fclose(xc->tchn_handle); xc->tchn_handle = NULL; }
	free(xc->vchg_mem); xc->vchg_mem = NULL;
	if(xc->curval_handle) { fclose(xc->curval_handle); xc->curval_handle = NULL; }
	if(xc->valpos_handle) { fclose(xc->valpos_handle); xc->valpos_handle = NULL; }
	if(xc->geom_handle) { fclose(xc->geom_handle); xc->geom_handle = NULL; }
	if(xc->hier_handle) { fclose(xc->hier_handle); xc->hier_handle = NULL; }
	if(xc->handle) 
		{ 
		if(xc->repack_on_close)
			{
			FILE *fp;
			off_t offpnt, uclen;
			int flen = strlen(xc->filename);
			char *hf = calloc(1, flen + 5);

			strcpy(hf, xc->filename);
			strcpy(hf+flen, ".pak");
			fp = fopen(hf, "wb");

			if(fp)
				{
				void *dsth;
				int zfd;
				char gz_membuf[FST_GZIO_LEN];

				fseeko(xc->handle, 0, SEEK_END);
				uclen = ftello(xc->handle);

				fputc(FST_BL_ZWRAPPER, fp);
				fstWriterUint64(fp, 0);
				fstWriterUint64(fp, uclen);
				fflush(fp);

				fseeko(xc->handle, 0, SEEK_SET);
				zfd = dup(fileno(fp));
				dsth = gzdopen(zfd, "wb4");
				if(dsth)
					{
					for(offpnt = 0; offpnt < uclen; offpnt += FST_GZIO_LEN)
						{
						size_t this_len = ((uclen - offpnt) > FST_GZIO_LEN) ? FST_GZIO_LEN : (uclen - offpnt);
						fstFread(gz_membuf, this_len, 1, xc->handle);
						gzwrite(dsth, gz_membuf, this_len);
						}
					gzclose(dsth);
					}
					else
					{
					close(zfd);
					}
				fseeko(fp, 0, SEEK_END);
				offpnt = ftello(fp);
				fseeko(fp, 1, SEEK_SET);
				fstWriterUint64(fp, offpnt - 1);
				fclose(fp);
				fclose(xc->handle); xc->handle = NULL; 

				unlink(xc->filename);
				rename(hf, xc->filename);
				}
				else
				{
				xc->repack_on_close = 0;
				fclose(xc->handle); xc->handle = NULL; 
				}

			free(hf);
			}
			else
			{
			fclose(xc->handle); xc->handle = NULL; 
			}
		}

#ifdef __MINGW32__ 
	{
	int flen = strlen(xc->filename);
	char *hf = calloc(1, flen + 6);
	strcpy(hf, xc->filename);

	if(xc->compress_hier)
		{
		strcpy(hf + flen, ".hier");
		unlink(hf); /* no longer needed as a section now exists for this */
		}

	free(hf);
	}
#endif

#ifdef FST_WRITER_PARALLEL
	pthread_mutex_destroy(&xc->mutex);
	pthread_attr_destroy(&xc->thread_attr);
#endif

	free(xc->filename); xc->filename = NULL;
	free(xc);
	}
}


/*
 * functions to set miscellaneous header/block information
 */
void fstWriterSetDate(void *ctx, const char *dat)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
        {
	char s[FST_HDR_DATE_SIZE];
	off_t fpos = ftello(xc->handle);
	int len = strlen(dat);

	fseeko(xc->handle, FST_HDR_OFFS_DATE, SEEK_SET);
	memset(s, 0, FST_HDR_DATE_SIZE);
	memcpy(s, dat, (len < FST_HDR_DATE_SIZE) ? len : FST_HDR_DATE_SIZE);
	fstFwrite(s, FST_HDR_DATE_SIZE, 1, xc->handle);
	fflush(xc->handle);
	fseeko(xc->handle, fpos, SEEK_SET);
	}
}


void fstWriterSetVersion(void *ctx, const char *vers)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc && vers)
        {
	char s[FST_HDR_SIM_VERSION_SIZE];
	off_t fpos = ftello(xc->handle);
	int len = strlen(vers);

	fseeko(xc->handle, FST_HDR_OFFS_SIM_VERSION, SEEK_SET);
	memset(s, 0, FST_HDR_SIM_VERSION_SIZE);
	memcpy(s, vers, (len < FST_HDR_SIM_VERSION_SIZE) ? len : FST_HDR_SIM_VERSION_SIZE);
	fstFwrite(s, FST_HDR_SIM_VERSION_SIZE, 1, xc->handle);
	fflush(xc->handle);
	fseeko(xc->handle, fpos, SEEK_SET);
	}
}


void fstWriterSetTimescale(void *ctx, int ts)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
        {
	off_t fpos = ftello(xc->handle);
	fseeko(xc->handle, FST_HDR_OFFS_TIMESCALE, SEEK_SET);
	fputc(ts & 255, xc->handle);
	fflush(xc->handle);
	fseeko(xc->handle, fpos, SEEK_SET);
	}
}


void fstWriterSetTimescaleFromString(void *ctx, const char *s)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc && s)
        {
        int mat = 0;
	int seconds_exp = -9;
	int tv = atoi(s);
	const char *pnt = s;

	while(*pnt)
        	{
                switch(*pnt)
                	{
                        case 'm': seconds_exp = -3; mat = 1; break;
                        case 'u': seconds_exp = -6; mat = 1; break;
                        case 'n': seconds_exp = -9; mat = 1; break;
                        case 'p': seconds_exp = -12; mat = 1; break;
                        case 'f': seconds_exp = -15; mat = 1; break;
                        case 'a': seconds_exp = -18; mat = 1; break;
                        case 'z': seconds_exp = -21; mat = 1; break;
                        case 's': seconds_exp = -0; mat = 1; break;
                        default: break;
                        }

		if(mat) break;
                pnt++;
                }

	if(tv == 10)
        	{
                seconds_exp++;
                } 
        else
        if(tv == 100)
        	{
                seconds_exp+=2;
                }
                              
	fstWriterSetTimescale(ctx, seconds_exp);
        }
}


void fstWriterSetTimezero(void *ctx, int64_t tim)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
        {
	off_t fpos = ftello(xc->handle);
	fseeko(xc->handle, FST_HDR_OFFS_TIMEZERO, SEEK_SET);
	fstWriterUint64(xc->handle, (xc->timezero = tim));
	fflush(xc->handle);
	fseeko(xc->handle, fpos, SEEK_SET);
	}
}


void fstWriterSetPackType(void *ctx, int typ)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
	{
	xc->fastpack = (typ != 0);
	}
}


void fstWriterSetRepackOnClose(void *ctx, int enable)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
	{
	xc->repack_on_close = (enable != 0);
	}
}


void fstWriterSetParallelMode(void *ctx, int enable)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
	{
	xc->parallel_was_enabled |= xc->parallel_enabled; /* make sticky */
	xc->parallel_enabled = (enable != 0);
#ifndef FST_WRITER_PARALLEL
	if(xc->parallel_enabled)
		{
		fprintf(stderr, "ERROR: fstWriterSetParallelMode(), FST_WRITER_PARALLEL not enabled during compile, exiting.\n");
		exit(255);
		}
#endif
	}
}


void fstWriterSetDumpSizeLimit(void *ctx, uint64_t numbytes)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
	{
	xc->dump_size_limit = numbytes;
	}
}


int fstWriterGetDumpSizeLimitReached(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
if(xc)
        {
        return(xc->size_limit_locked != 0);
        }

return(0);
}


/*
 * writer scope/var creation
 */
fstHandle fstWriterCreateVar(void *ctx, enum fstVarType vt, enum fstVarDir vd,
        uint32_t len, const char *nam, fstHandle aliasHandle)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
int i, nlen, is_real;
                
if(xc && nam)
        {
	if(xc->valpos_mem)
		{
		fstDestroyMmaps(xc, 0);
		}

	fputc(vt, xc->hier_handle);
	fputc(vd, xc->hier_handle);
	nlen = strlen(nam);
	fstFwrite(nam, nlen, 1, xc->hier_handle);
	fputc(0, xc->hier_handle);
	xc->hier_file_len += (nlen+3);

	if((vt == FST_VT_VCD_REAL) || (vt == FST_VT_VCD_REAL_PARAMETER) || (vt == FST_VT_VCD_REALTIME))
		{
		is_real = 1;
		len = 8; /* recast number of bytes to that of what a double is */
		}
		else
		{
		is_real = 0;
		if(vt == FST_VT_GEN_STRING)
			{
			len = 0;
			}
		}

	xc->hier_file_len += fstWriterVarint(xc->hier_handle, len);	

	if(aliasHandle > xc->maxhandle) aliasHandle = 0;
	xc->hier_file_len += fstWriterVarint(xc->hier_handle, aliasHandle);	
	xc->numsigs++;
	if(xc->numsigs == xc->next_huge_break)
		{
		if(xc->fst_break_size < xc->fst_huge_break_size)
			{
			xc->next_huge_break += FST_ACTIVATE_HUGE_INC;
			xc->fst_break_size += xc->fst_orig_break_size;
			xc->fst_break_add_size += xc->fst_orig_break_add_size;

			xc->vchg_alloc_siz = xc->fst_break_size + xc->fst_break_add_size;
			if(xc->vchg_mem)
				{
				xc->vchg_mem = realloc(xc->vchg_mem, xc->vchg_alloc_siz);
				}
			}
		}

	if(!aliasHandle)
		{
		uint32_t zero = 0;

		if(len)
			{
			fstWriterVarint(xc->geom_handle, !is_real ? len : 0); /* geom section encodes reals as zero byte */
			}
			else
			{
			fstWriterVarint(xc->geom_handle, 0xFFFFFFFF);         /* geom section encodes zero len as 32b -1 */
			}

		fstFwrite(&xc->maxvalpos, sizeof(uint32_t), 1, xc->valpos_handle);
		fstFwrite(&len, sizeof(uint32_t), 1, xc->valpos_handle);
		fstFwrite(&zero, sizeof(uint32_t), 1, xc->valpos_handle);
		fstFwrite(&zero, sizeof(uint32_t), 1, xc->valpos_handle);

		if(!is_real)
			{
			for(i=0;i<len;i++)
				{
				fputc('x', xc->curval_handle);
				}
			}
			else
			{
			fstFwrite(&xc->nan, 8, 1, xc->curval_handle); /* initialize doubles to NaN rather than x */
			}			
		
		xc->maxvalpos+=len;
		xc->maxhandle++;
		return(xc->maxhandle);
		}
		else
		{
		return(aliasHandle);
		}
	}

return(0);
}


void fstWriterSetScope(void *ctx, enum fstScopeType scopetype,
                const char *scopename, const char *scopecomp)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

if(xc && scopename)
	{
	fputc(FST_ST_VCD_SCOPE, xc->hier_handle);
	if((scopetype < FST_ST_VCD_MODULE) || (scopetype > FST_ST_MAX)) { scopetype = FST_ST_VCD_MODULE; }
	fputc(scopetype, xc->hier_handle);
	fprintf(xc->hier_handle, "%s%c%s%c",
		scopename ? scopename : "", 0,
		scopecomp ? scopecomp : "", 0);
	
	if(scopename)
		{
		xc->hier_file_len += strlen(scopename);
		}
	if(scopecomp)
		{
		xc->hier_file_len += strlen(scopecomp);
		}

	xc->hier_file_len += 4; /* FST_ST_VCD_SCOPE + scopetype + two string terminating zeros */
	xc->numscopes++;
	}
}


void fstWriterSetUpscope(void *ctx)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

if(xc)
	{
	fputc(FST_ST_VCD_UPSCOPE, xc->hier_handle);
	xc->hier_file_len++;
	}
}


/*
 * value and time change emission
 */
void fstWriterEmitValueChange(void *ctx, fstHandle handle, const void *val)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
const unsigned char *buf = (const unsigned char *)val;
uint32_t offs;
int len;

if((xc) && (handle <= xc->maxhandle))
	{
	uint32_t fpos;
	uint32_t *vm4ip;

	if(!xc->valpos_mem)
		{
		xc->vc_emitted = 1;
		fstWriterCreateMmaps(xc);
		}

	handle--; /* move starting at 1 index to starting at 0 */
	vm4ip = &(xc->valpos_mem[4*handle]);

	len  = vm4ip[1];
	if(len) /* len of zero = variable length, use fstWriterEmitVariableLengthValueChange */
		{
		if(!xc->is_initial_time)
			{
			fpos = xc->vchg_siz;
	
			if((fpos + len + 10) > xc->vchg_alloc_siz)
				{
				xc->vchg_alloc_siz += (xc->fst_break_add_size + len); /* +len added in the case of extremely long vectors and small break add sizes */
				xc->vchg_mem = realloc(xc->vchg_mem, xc->vchg_alloc_siz);
				if(!xc->vchg_mem)
					{
					fprintf(stderr, "FATAL ERROR, could not realloc() in fstWriterEmitValueChange, exiting.\n");
					exit(255); 
					}
				}
#ifdef FST_REMOVE_DUPLICATE_VC
			offs = vm4ip[0];

			if(len != 1)
				{
				if((vm4ip[3]==xc->tchn_idx)&&(vm4ip[2]))
					{
					unsigned char *old_value = xc->vchg_mem + vm4ip[2] + 4; /* the +4 skips old vm4ip[2] value */
					while(*(old_value++) & 0x80) { /* skips over varint encoded "xc->tchn_idx - vm4ip[3]" */ }
					memcpy(old_value, buf, len); /* overlay new value */
	
					memcpy(xc->curval_mem + offs, buf, len);
					return;
					}
				else
					{
					if(!memcmp(xc->curval_mem + offs, buf, len))
						{
						if(!xc->curtime)
							{
							int i;
							for(i=0;i<len;i++)
								{
								if(buf[i]!='x') break;
								}

							if(i<len) return;
							}
							else
							{
							return;
							}
						}
					}
	
				memcpy(xc->curval_mem + offs, buf, len);
				}
				else
				{
				if((vm4ip[3]==xc->tchn_idx)&&(vm4ip[2]))
					{
					unsigned char *old_value = xc->vchg_mem + vm4ip[2] + 4; /* the +4 skips old vm4ip[2] value */
					while(*(old_value++) & 0x80) { /* skips over varint encoded "xc->tchn_idx - vm4ip[3]" */ }
					*old_value = *buf; /* overlay new value */
	
					*(xc->curval_mem + offs) = *buf;
					return;
					}
				else
					{
					if((*(xc->curval_mem + offs)) == (*buf))
						{
						if(!xc->curtime)
							{
							if(*buf != 'x') return;
							}
							else
							{
							return;
							}
						}
					}
	
				*(xc->curval_mem + offs) = *buf;
				}
#endif
			xc->vchg_siz += fstWriterUint32WithVarint32(xc, &vm4ip[2], xc->tchn_idx - vm4ip[3], buf, len); /* do one fwrite op only */
			vm4ip[3] = xc->tchn_idx;
			vm4ip[2] = fpos;
			}
			else
			{
			offs = vm4ip[0];
			memcpy(xc->curval_mem + offs, buf, len);
			}
		}
	}
}


void fstWriterEmitVariableLengthValueChange(void *ctx, fstHandle handle, const void *val, uint32_t len)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
const unsigned char *buf = (const unsigned char *)val;

if((xc) && (handle <= xc->maxhandle))
	{
	uint32_t fpos;
	uint32_t *vm4ip;

	if(!xc->valpos_mem)
		{
		xc->vc_emitted = 1;
		fstWriterCreateMmaps(xc);
		}

	handle--; /* move starting at 1 index to starting at 0 */
	vm4ip = &(xc->valpos_mem[4*handle]);

	/* there is no initial time dump for variable length value changes */
	if(!vm4ip[1]) /* len of zero = variable length */
		{
		fpos = xc->vchg_siz;

		if((fpos + len + 10 + 5) > xc->vchg_alloc_siz)
			{
			xc->vchg_alloc_siz += (xc->fst_break_add_size + len + 5); /* +len added in the case of extremely long vectors and small break add sizes */
			xc->vchg_mem = realloc(xc->vchg_mem, xc->vchg_alloc_siz);
			if(!xc->vchg_mem)
				{
				fprintf(stderr, "FATAL ERROR, could not realloc() in fstWriterEmitVariableLengthValueChange, exiting.\n");
				exit(255); 
				}
			}

		xc->vchg_siz += fstWriterUint32WithVarint32AndLength(xc, &vm4ip[2], xc->tchn_idx - vm4ip[3], buf, len); /* do one fwrite op only */
		vm4ip[3] = xc->tchn_idx;
		vm4ip[2] = fpos;
		}
	}
}


void fstWriterEmitTimeChange(void *ctx, uint64_t tim)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;
int i;
int skip = 0;
if(xc)
	{
	if(xc->is_initial_time)
		{
		if(xc->size_limit_locked)	/* this resets xc->is_initial_time to one */
			{
			return;
			}

		if(!xc->valpos_mem)
			{
			fstWriterCreateMmaps(xc);
			}

		skip = 1;

		xc->firsttime = (xc->vc_emitted) ? 0: tim;
		xc->curtime = 0;
		xc->vchg_mem[0] = '!';
		xc->vchg_siz = 1;
		fstWriterEmitSectionHeader(xc);
		for(i=0;i<xc->maxhandle;i++)
			{
			xc->valpos_mem[4*i+2] = 0; /* zero out offset val */
			xc->valpos_mem[4*i+3] = 0; /* zero out last time change val */
			}
		xc->is_initial_time = 0;
		}
		else
		{
		if((xc->vchg_siz >= xc->fst_break_size) || (xc->flush_context_pending))
			{
			xc->flush_context_pending = 0;
			fstWriterFlushContextPrivate(xc);
			xc->tchn_cnt++;
			fstWriterVarint(xc->tchn_handle, xc->curtime);
			}
		}

	if(!skip)
		{
		xc->tchn_idx++;
		}
	fstWriterVarint(xc->tchn_handle, tim - xc->curtime);
	xc->tchn_cnt++;
	xc->curtime = tim;
	}
}


void fstWriterEmitDumpActive(void *ctx, int enable)
{
struct fstWriterContext *xc = (struct fstWriterContext *)ctx;

if(xc)
	{
	struct fstBlackoutChain *b = calloc(1, sizeof(struct fstBlackoutChain));

	b->tim = xc->curtime;
	b->active = (enable != 0);

	xc->num_blackouts++;
	if(xc->blackout_curr)
		{
		xc->blackout_curr->next = b;
		xc->blackout_curr = b;
		}
		else
		{
		xc->blackout_head = b;
		xc->blackout_curr = b;
		}
	}
}


/***********************/
/***                 ***/
/*** reader function ***/
/***                 ***/
/***********************/

/*
 * private structs
 */
static const char *vartypes[] = {
	"event", "integer", "parameter", "real", "real_parameter",
	"reg", "supply0", "supply1", "time", "tri",
	"triand", "trior", "trireg", "tri0", "tri1", 
	"wand", "wire", "wor", "port", "array", "realtime",
	"string"
	};

static const char *modtypes[] = {
	"module", "task", "function", "begin", "fork"
	};

struct fstCurrHier
{
struct fstCurrHier *prev;
void *user_info;
int len;
};


struct fstReaderContext
{
/* common entries */

FILE *f, *fh;

uint64_t start_time, end_time;
uint64_t mem_used_by_writer;
uint64_t scope_count;
uint64_t var_count;
fstHandle maxhandle;
uint64_t num_alias;
uint64_t vc_section_count;

uint32_t *signal_lens;			/* maxhandle sized */
unsigned char *signal_typs;		/* maxhandle sized */
unsigned char *process_mask;		/* maxhandle-based, bitwise sized */
uint32_t longest_signal_value_len;	/* longest len value encountered */
unsigned char *temp_signal_value_buf;	/* malloced for len in longest_signal_value_len */

signed char timescale;
unsigned double_endian_match : 1;
unsigned native_doubles_for_cb : 1;
unsigned contains_geom_section : 1;
unsigned contains_hier_section : 1;	/* valid for hier_pos */
unsigned limit_range_valid : 1;		/* valid for limit_range_start, limit_range_end */

char version[FST_HDR_SIM_VERSION_SIZE + 1];
char date[FST_HDR_DATE_SIZE + 1];
int64_t timezero;

char *filename, *filename_unpacked;
off_t hier_pos;

uint32_t num_blackouts;
uint64_t *blackout_times;
unsigned char *blackout_activity;

uint64_t limit_range_start, limit_range_end;

/* entries specific to read value at time functions */

unsigned rvat_data_valid : 1;
uint64_t *rvat_time_table;
uint64_t rvat_beg_tim, rvat_end_tim;
unsigned char *rvat_frame_data;
uint64_t rvat_frame_maxhandle;
off_t *rvat_chain_table;
uint32_t *rvat_chain_table_lengths;
uint64_t rvat_vc_maxhandle;
off_t rvat_vc_start;
uint32_t *rvat_sig_offs;

uint32_t rvat_chain_len;
unsigned char *rvat_chain_mem;
fstHandle rvat_chain_facidx;

uint32_t rvat_chain_pos_tidx;
uint32_t rvat_chain_pos_idx;  
uint64_t rvat_chain_pos_time;
unsigned rvat_chain_pos_valid : 1;

/* entries specific to hierarchy traversal */

struct fstHier hier;
struct fstCurrHier *curr_hier;
fstHandle current_handle;
char *curr_flat_hier_nam;
int flat_hier_alloc_len;
unsigned do_rewind : 1;
char str_scope_nam[FST_ID_NAM_SIZ+1];
char str_scope_comp[FST_ID_NAM_SIZ+1];

};


/*
 * scope -> flat name handling
 */
static void fstReaderDeallocateScopeData(struct fstReaderContext *xc)
{
struct fstCurrHier *chp;

free(xc->curr_flat_hier_nam); xc->curr_flat_hier_nam = NULL;
while(xc->curr_hier)
	{
	chp = xc->curr_hier->prev;
	free(xc->curr_hier);
	xc->curr_hier = chp;
	}
}


const char *fstReaderGetCurrentFlatScope(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc)
	{
	return(xc->curr_flat_hier_nam ? xc->curr_flat_hier_nam : "");
	}
	else
	{
	return(NULL);
	}
}


void *fstReaderGetCurrentScopeUserInfo(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc)
	{
	return(xc->curr_hier ? xc->curr_hier->user_info : NULL);
	}
	else
	{
	return(NULL);
	}
}


const char *fstReaderPopScope(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc && xc->curr_hier)   
        {
	struct fstCurrHier *ch = xc->curr_hier;
	if(xc->curr_hier->prev)
		{
		xc->curr_flat_hier_nam[xc->curr_hier->prev->len] = 0;
		}
		else
		{
		*xc->curr_flat_hier_nam = 0;
		}
	xc->curr_hier = xc->curr_hier->prev;
	free(ch);
	return(xc->curr_flat_hier_nam ? xc->curr_flat_hier_nam : "");
	}

return(NULL);
}


void fstReaderResetScope(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc)
	{
	while(fstReaderPopScope(xc)); /* remove any already-built scoping info */
	}
}


const char *fstReaderPushScope(void *ctx, const char *nam, void *user_info)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc)
	{
	struct fstCurrHier *ch = malloc(sizeof(struct fstCurrHier));
	int chl = xc->curr_hier ? xc->curr_hier->len : 0;
	int len = chl + 1 + strlen(nam);
	if(len >= xc->flat_hier_alloc_len)
		{
		xc->curr_flat_hier_nam = xc->curr_flat_hier_nam ? realloc(xc->curr_flat_hier_nam, len+1) : malloc(len+1);
		}

	if(chl)
		{
		xc->curr_flat_hier_nam[chl] = '.';
		strcpy(xc->curr_flat_hier_nam + chl + 1, nam);
		}
		else
		{
		strcpy(xc->curr_flat_hier_nam, nam);
		len--;
		}

	ch->len = len;
	ch->prev = xc->curr_hier;
	ch->user_info = user_info;
	xc->curr_hier = ch;
	return(xc->curr_flat_hier_nam);
	}

return(NULL);
}


int fstReaderGetCurrentScopeLen(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc && xc->curr_hier)
	{
	return(xc->curr_hier->len);
	}

return(0);
}


/*
 * iter mask manipulation util functions
 */
int fstReaderGetFacProcessMask(void *ctx, fstHandle facidx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
                                 
if(xc)
        {      
	facidx--;
	if(facidx<xc->maxhandle)
		{
		int process_idx = facidx/8;
		int process_bit = facidx&7;

		return( (xc->process_mask[process_idx]&(1<<process_bit)) != 0 );
		}
	}
return(0);
}


void fstReaderSetFacProcessMask(void *ctx, fstHandle facidx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
                                 
if(xc)
        { 
	facidx--;
	if(facidx<xc->maxhandle)
		{
		int idx = facidx/8;
		int bitpos = facidx&7;

		xc->process_mask[idx] |= (1<<bitpos);
		}
	}
}


void fstReaderClrFacProcessMask(void *ctx, fstHandle facidx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
                                 
if(xc)
        { 
	facidx--;
	if(facidx<xc->maxhandle)
		{
		int idx = facidx/8;
		int bitpos = facidx&7;

		xc->process_mask[idx] &= (~(1<<bitpos));
		}
	}
}


void fstReaderSetFacProcessMaskAll(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
                                 
if(xc)
        { 
	memset(xc->process_mask, 0xff, (xc->maxhandle+7)/8);
	}
}


void fstReaderClrFacProcessMaskAll(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
                                 
if(xc)
        { 
	memset(xc->process_mask, 0x00, (xc->maxhandle+7)/8);
	}
}


/*
 * various utility read/write functions
 */
signed char fstReaderGetTimescale(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->timescale : 0);
}


uint64_t fstReaderGetStartTime(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->start_time : 0);
}


uint64_t fstReaderGetEndTime(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->end_time : 0);
}


uint64_t fstReaderGetMemoryUsedByWriter(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->mem_used_by_writer : 0);
}


uint64_t fstReaderGetScopeCount(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->scope_count : 0);
}


uint64_t fstReaderGetVarCount(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->var_count : 0);
}


fstHandle fstReaderGetMaxHandle(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->maxhandle : 0);
}


uint64_t fstReaderGetAliasCount(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->num_alias : 0);
}


uint64_t fstReaderGetValueChangeSectionCount(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->vc_section_count : 0);
}


int fstReaderGetDoubleEndianMatchState(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->double_endian_match : 0);
}


const char *fstReaderGetVersionString(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->version : NULL);
}


const char *fstReaderGetDateString(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->date : NULL);
}

int64_t fstReaderGetTimezero(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->timezero : 0);
}


uint32_t fstReaderGetNumberDumpActivityChanges(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
return(xc ? xc->num_blackouts : 0);
}


uint64_t fstReaderGetDumpActivityChangeTime(void *ctx, uint32_t idx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc && (idx < xc->num_blackouts) && (xc->blackout_times))
	{
	return(xc->blackout_times[idx]);
	}
	else
	{
	return(0);
	}
}


unsigned char fstReaderGetDumpActivityChangeValue(void *ctx, uint32_t idx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc && (idx < xc->num_blackouts) && (xc->blackout_activity))
	{
	return(xc->blackout_activity[idx]);
	}
	else
	{
	return(0);
	}
}


void fstReaderSetLimitTimeRange(void *ctx, uint64_t start_time, uint64_t end_time)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc)
	{
	xc->limit_range_valid = 1;
	xc->limit_range_start = start_time;
	xc->limit_range_end = end_time;
	}
}


void fstReaderSetUnlimitedTimeRange(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc)
	{
	xc->limit_range_valid = 0;
	}
}


void fstReaderIterBlocksSetNativeDoublesOnCallback(void *ctx, int enable)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc)
	{
	xc->native_doubles_for_cb = (enable != 0);
	}
}

/*
 * hierarchy processing
 */
static char *fstVcdID(int value)
{
static char buf[16];
char *pnt = buf;
int vmod;

/* zero is illegal for a value...it is assumed they start at one */
for(;;)
        {
        if((vmod = (value % 94)))
                {
                *(pnt++) = (char)(vmod + 32);
                } 
                else
                {
                *(pnt++) = '~'; value -= 94;
                }
        value = value / 94;
        if(!value) { break; }
        }

*pnt = 0;
return(buf);
}

static char *fstVcdIDForFwrite(int value, int *len)
{
static char buf[16];
char *pnt = buf;
int vmod;

/* zero is illegal for a value...it is assumed they start at one */
for(;;)
        {
        if((vmod = (value % 94)))
                {
                *(pnt++) = (char)(vmod + 32);
                } 
                else
                {
                *(pnt++) = '~'; value -= 94;
                }
        value = value / 94;
        if(!value) { break; }
        }

*len = pnt-buf;
return(buf);
}


static int fstReaderRecreateHierFile(struct fstReaderContext *xc)
{
int pass_status = 1;

if(!xc->fh)
	{
	off_t offs_cache = ftello(xc->f);
	char *fnam = malloc(strlen(xc->filename) + 6 + 16 + 32 + 1);
	unsigned char *mem = malloc(FST_GZIO_LEN);
	off_t hl, uclen;
	gzFile zhandle;
	int zfd;

	sprintf(fnam, "%s.hier_%d_%p", xc->filename, getpid(), (void *)xc);
	fseeko(xc->f, xc->hier_pos, SEEK_SET);
	uclen = fstReaderUint64(xc->f);
	fflush(xc->f);
	zfd = dup(fileno(xc->f));
	zhandle = gzdopen(zfd, "rb");
	if(!zhandle)
		{
		close(zfd);
		free(mem);
		free(fnam);
		return(0);
		}

#ifndef __MINGW32__
	xc->fh = fopen(fnam, "w+b");
        if(!xc->fh)
#endif
                {
                xc->fh = tmpfile();  
                free(fnam); fnam = NULL;
                if(!xc->fh)
			{
			free(mem);
			return(0);
			}
                }    

#ifndef __MINGW32__
	if(fnam) unlink(fnam);
#endif

        for(hl = 0; hl < uclen; hl += FST_GZIO_LEN)
		{
                size_t len = ((uclen - hl) > FST_GZIO_LEN) ? FST_GZIO_LEN : (uclen - hl);
		size_t gzreadlen = gzread(zhandle, mem, len); /* rc should equal len... */
		size_t fwlen;

		if(gzreadlen != len)
			{
			pass_status = 0;
			break;
			}

		fwlen = fstFwrite(mem, len, 1, xc->fh);
		if(fwlen != 1)
			{
			pass_status = 0;
			break;
			}
                }
        gzclose(zhandle);
	free(mem);
	free(fnam);

	fseeko(xc->f, offs_cache, SEEK_SET);
	}

return(pass_status);
}


int fstReaderIterateHierRewind(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
int pass_status = 0;

if(xc)
	{
	pass_status = 1;
	if(!xc->fh)
		{
		pass_status = fstReaderRecreateHierFile(xc);
		}

	xc->do_rewind = 1;
	}

return(pass_status);
}


struct fstHier *fstReaderIterateHier(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
int isfeof;
fstHandle alias;
char *pnt;
int ch;

if(!xc) return(NULL);

if(!xc->fh)
	{
	if(!fstReaderRecreateHierFile(xc))
		{
		return(NULL);
		}
	}

if(xc->do_rewind)
	{
	xc->do_rewind = 0;
	xc->current_handle = 0;
	fseeko(xc->fh, 0, SEEK_SET);
	clearerr(xc->fh);
	}

if(!(isfeof=feof(xc->fh)))
	{
	int tag = fgetc(xc->fh);
	switch(tag)
		{
		case FST_ST_VCD_SCOPE:
			xc->hier.htyp = FST_HT_SCOPE;
			xc->hier.u.scope.typ = fgetc(xc->fh);
			xc->hier.u.scope.name = pnt = xc->str_scope_nam;
			while((ch = fgetc(xc->fh))) 
				{
				*(pnt++) = ch; 
				}; /* scopename */
			*pnt = 0;
			xc->hier.u.scope.name_length = pnt - xc->hier.u.scope.name;

			xc->hier.u.scope.component = pnt = xc->str_scope_comp;
			while((ch = fgetc(xc->fh))) 
				{
				*(pnt++) = ch; 
				}; /* scopecomp */
			*pnt = 0;
			xc->hier.u.scope.component_length = pnt - xc->hier.u.scope.component;
			break;

		case FST_ST_VCD_UPSCOPE:
			xc->hier.htyp = FST_HT_UPSCOPE;
			break;

		case FST_VT_VCD_EVENT:
		case FST_VT_VCD_INTEGER:
		case FST_VT_VCD_PARAMETER:
		case FST_VT_VCD_REAL:
		case FST_VT_VCD_REAL_PARAMETER:
		case FST_VT_VCD_REG:
		case FST_VT_VCD_SUPPLY0:
		case FST_VT_VCD_SUPPLY1:
		case FST_VT_VCD_TIME:
		case FST_VT_VCD_TRI:
		case FST_VT_VCD_TRIAND:
		case FST_VT_VCD_TRIOR:
		case FST_VT_VCD_TRIREG:
		case FST_VT_VCD_TRI0:
		case FST_VT_VCD_TRI1:
		case FST_VT_VCD_WAND:
		case FST_VT_VCD_WIRE:
		case FST_VT_VCD_WOR:
		case FST_VT_VCD_PORT:
		case FST_VT_VCD_ARRAY:
		case FST_VT_VCD_REALTIME:
		case FST_VT_GEN_STRING:
			xc->hier.htyp = FST_HT_VAR;

			xc->hier.u.var.typ = tag;
			xc->hier.u.var.direction = fgetc(xc->fh);
			xc->hier.u.var.name = pnt = xc->str_scope_nam;
			while((ch = fgetc(xc->fh))) 
				{
				*(pnt++) = ch; 
				}; /* varname */
			*pnt = 0;
			xc->hier.u.var.name_length = pnt - xc->hier.u.var.name;
			xc->hier.u.var.length = fstReaderVarint32(xc->fh);
			if(tag == FST_VT_VCD_PORT)
				{
				xc->hier.u.var.length -= 2; /* removal of delimiting spaces */
				xc->hier.u.var.length /= 3; /* port -> signal size adjust */
				}

			alias = fstReaderVarint32(xc->fh);

			if(!alias)
				{
                		xc->current_handle++;
				xc->hier.u.var.handle = xc->current_handle;
				xc->hier.u.var.is_alias = 0;
				}
				else
				{
				xc->hier.u.var.handle = alias;
				xc->hier.u.var.is_alias = 1;
				}
		
			break;

		default:
			isfeof = 1; 
			break;
		}
	}

return(!isfeof ? &xc->hier : NULL);
}


int fstReaderProcessHier(void *ctx, FILE *fv)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
char str[FST_ID_NAM_SIZ+1];
char *pnt;
int ch, scopetype;
int vartype;
uint32_t len, alias;
uint32_t maxvalpos=0;
int num_signal_dyn = 65536;

if(!xc) return(0);

xc->longest_signal_value_len = 32; /* arbitrarily set at 32...this is much longer than an expanded double */

if(!xc->fh)
	{
	if(!fstReaderRecreateHierFile(xc))
		{
		return(0);
		}
	}

if(fv)
	{
	char time_dimension[2] = {0, 0};
	int time_scale = 1;

	fprintf(fv, "$date\n\t%s\n$end\n", xc->date);
	fprintf(fv, "$version\n\t%s\n$end\n", xc->version);
	if(xc->timezero) fprintf(fv, "$timezero\n\t%"PRId64"\n$end\n", xc->timezero);
	
        switch(xc->timescale)
                {
                case  2:        time_scale = 100;               time_dimension[0] = ' '; break;
                case  1:        time_scale = 10;
                case  0:                                        time_dimension[0] = ' '; break;

                case -1:        time_scale = 100;               time_dimension[0] = 'm'; break;
                case -2:        time_scale = 10;
                case -3:                                        time_dimension[0] = 'm'; break;
         
                case -4:        time_scale = 100;               time_dimension[0] = 'u'; break;
                case -5:        time_scale = 10;
                case -6:                                        time_dimension[0] = 'u'; break;
        
                case -10:       time_scale = 100;               time_dimension[0] = 'p'; break;
                case -11:       time_scale = 10;
                case -12:                                       time_dimension[0] = 'p'; break;
        
                case -13:       time_scale = 100;               time_dimension[0] = 'f'; break;
                case -14:       time_scale = 10;
                case -15:                                       time_dimension[0] = 'f'; break;

                case -16:       time_scale = 100;               time_dimension[0] = 'a'; break;
                case -17:       time_scale = 10;
                case -18:                                       time_dimension[0] = 'a'; break;

                case -19:       time_scale = 100;               time_dimension[0] = 'z'; break;
                case -20:       time_scale = 10;
                case -21:                                       time_dimension[0] = 'z'; break;
        
                case -7:        time_scale = 100;               time_dimension[0] = 'n'; break;
                case -8:        time_scale = 10;
                case -9:
                default:                                        time_dimension[0] = 'n'; break;
                }

	if(fv) fprintf(fv, "$timescale\n\t%d%ss\n$end\n", time_scale, time_dimension);
	}

xc->maxhandle = 0;
xc->num_alias = 0;

free(xc->signal_lens);
xc->signal_lens = malloc(num_signal_dyn*sizeof(uint32_t));

free(xc->signal_typs);
xc->signal_typs = malloc(num_signal_dyn*sizeof(unsigned char));

fseeko(xc->fh, 0, SEEK_SET);
while(!feof(xc->fh))
	{
	int tag = fgetc(xc->fh);
	switch(tag)
		{
		case FST_ST_VCD_SCOPE:
			scopetype = fgetc(xc->fh);
			pnt = str;
			while((ch = fgetc(xc->fh))) 
				{
				*(pnt++) = ch; 
				}; /* scopename */
			*pnt = 0;
			while(fgetc(xc->fh)) { }; /* scopecomp */

			if(fv) fprintf(fv, "$scope %s %s $end\n", modtypes[scopetype], str);
			break;

		case FST_ST_VCD_UPSCOPE:
			if(fv) fprintf(fv, "$upscope $end\n");
			break;

		case FST_VT_VCD_EVENT:
		case FST_VT_VCD_INTEGER:
		case FST_VT_VCD_PARAMETER:
		case FST_VT_VCD_REAL:
		case FST_VT_VCD_REAL_PARAMETER:
		case FST_VT_VCD_REG:
		case FST_VT_VCD_SUPPLY0:
		case FST_VT_VCD_SUPPLY1:
		case FST_VT_VCD_TIME:
		case FST_VT_VCD_TRI:
		case FST_VT_VCD_TRIAND:
		case FST_VT_VCD_TRIOR:
		case FST_VT_VCD_TRIREG:
		case FST_VT_VCD_TRI0:
		case FST_VT_VCD_TRI1:
		case FST_VT_VCD_WAND:
		case FST_VT_VCD_WIRE:
		case FST_VT_VCD_WOR:
		case FST_VT_VCD_PORT:
		case FST_VT_VCD_ARRAY:
		case FST_VT_VCD_REALTIME:
		case FST_VT_GEN_STRING:
			vartype = tag;
			/* vardir = */ fgetc(xc->fh); /* unused in VCD reader, but need to advance read pointer */
			pnt = str;
			while((ch = fgetc(xc->fh))) 
				{
				*(pnt++) = ch; 
				}; /* varname */
			*pnt = 0;
			len = fstReaderVarint32(xc->fh);
			alias = fstReaderVarint32(xc->fh);

			if(!alias)
				{
				if(xc->maxhandle == num_signal_dyn)
					{
					num_signal_dyn *= 2;
					xc->signal_lens = realloc(xc->signal_lens, num_signal_dyn*sizeof(uint32_t));
					xc->signal_typs = realloc(xc->signal_typs, num_signal_dyn*sizeof(unsigned char));
					}
				xc->signal_lens[xc->maxhandle] = len;
				xc->signal_typs[xc->maxhandle] = vartype;

		                maxvalpos+=len;
				if(len > xc->longest_signal_value_len)
					{
					xc->longest_signal_value_len = len;
					}

				if((vartype == FST_VT_VCD_REAL) || (vartype == FST_VT_VCD_REAL_PARAMETER) || (vartype == FST_VT_VCD_REALTIME))
					{
					len = 64;
					xc->signal_typs[xc->maxhandle] = FST_VT_VCD_REAL;
					}
				if(fv) 
					{
					uint32_t modlen = (vartype != FST_VT_VCD_PORT) ? len : ((len - 2) / 3);
					fprintf(fv, "$var %s %"PRIu32" %s %s $end\n", vartypes[vartype], modlen, fstVcdID(xc->maxhandle+1), str);
					}
                		xc->maxhandle++;
				}
				else
				{
				if((vartype == FST_VT_VCD_REAL) || (vartype == FST_VT_VCD_REAL_PARAMETER) || (vartype == FST_VT_VCD_REALTIME))
					{
					len = 64;
					xc->signal_typs[xc->maxhandle] = FST_VT_VCD_REAL;
					}
				if(fv) 
					{
					uint32_t modlen = (vartype != FST_VT_VCD_PORT) ? len : ((len - 2) / 3);
					fprintf(fv, "$var %s %"PRIu32" %s %s $end\n", vartypes[vartype], modlen, fstVcdID(alias), str);
					}
				xc->num_alias++;
				}
		
			break;

		default:
			break;
		}
	}
if(fv) fprintf(fv, "$enddefinitions $end\n");

xc->signal_lens = realloc(xc->signal_lens, xc->maxhandle*sizeof(uint32_t));
xc->signal_typs = realloc(xc->signal_typs, xc->maxhandle*sizeof(unsigned char));

free(xc->process_mask);
xc->process_mask = calloc(1, (xc->maxhandle+7)/8);

free(xc->temp_signal_value_buf);
xc->temp_signal_value_buf = malloc(xc->longest_signal_value_len + 1);

xc->var_count = xc->maxhandle + xc->num_alias;

return(1);
}


/*
 * reader file open/close functions
 */
int fstReaderInit(struct fstReaderContext *xc)
{
off_t blkpos = 0;
off_t endfile;
uint64_t seclen;
int sectype;
uint64_t vc_section_count_actual = 0;
int hdr_incomplete = 0;
int hdr_seen = 0;
int gzread_pass_status = 1;

sectype = fgetc(xc->f);
if(sectype == FST_BL_ZWRAPPER)
	{
	FILE *fcomp;
	off_t offpnt, uclen;
	char gz_membuf[FST_GZIO_LEN];
	void *zhandle;
	int zfd;
        int flen = strlen(xc->filename);
        char *hf;

	seclen = fstReaderUint64(xc->f);
	uclen = fstReaderUint64(xc->f);

	if(!seclen) return(0); /* not finished compressing, this is a failed read */

        hf = calloc(1, flen + 16 + 32 + 1);

	sprintf(hf, "%s.upk_%d_%p", xc->filename, getpid(), (void *)xc);
	fcomp = fopen(hf, "w+b");
	if(!fcomp)
		{
		fcomp = tmpfile();
		free(hf); hf = NULL;
		if(!fcomp) return(0);
		}

#if defined(FST_MACOSX)
	setvbuf(fcomp, (char *)NULL, _IONBF, 0);   /* keeps gzip from acting weird in tandem with fopen */
#endif

#ifdef __MINGW32__
	setvbuf(fcomp, (char *)NULL, _IONBF, 0);   /* keeps gzip from acting weird in tandem with fopen */
	xc->filename_unpacked = hf;
#else
	if(hf) 
		{
		unlink(hf);
		free(hf);
		}
#endif

	fseeko(xc->f, 1+8+8, SEEK_SET);
	fflush(xc->f);

	zfd = dup(fileno(xc->f));
	zhandle = gzdopen(zfd, "rb");
	if(zhandle)
		{
		for(offpnt = 0; offpnt < uclen; offpnt += FST_GZIO_LEN)
			{
			size_t this_len = ((uclen - offpnt) > FST_GZIO_LEN) ? FST_GZIO_LEN : (uclen - offpnt);
			size_t gzreadlen = gzread(zhandle, gz_membuf, this_len);
			size_t fwlen;

	                if(gzreadlen != this_len)
	                        {
	                        gzread_pass_status = 0;
	                        break;
	                        }
			fwlen = fstFwrite(gz_membuf, this_len, 1, fcomp);
			if(fwlen != 1)
				{
				gzread_pass_status = 0;
				break;
				}
			}
		gzclose(zhandle);
		}
		else
		{
		close(zfd);
		}
	fflush(fcomp);
	fclose(xc->f);
	xc->f = fcomp;
	}

if(gzread_pass_status)
	{
	fseeko(xc->f, 0, SEEK_END);
	endfile = ftello(xc->f);

	while(blkpos < endfile)
		{
		fseeko(xc->f, blkpos, SEEK_SET);
		
		sectype = fgetc(xc->f);
		seclen = fstReaderUint64(xc->f);
	
		if(sectype == EOF) 
			{
			break;
			}
	
		if(!hdr_seen && (sectype != FST_BL_HDR)) 
			{
			break;
			}
	
		blkpos++;
		if(sectype == FST_BL_HDR)
			{
			if(!hdr_seen)
				{
				int ch;
				double dcheck;
	
				xc->start_time = fstReaderUint64(xc->f);
				xc->end_time = fstReaderUint64(xc->f); 
	
				hdr_incomplete = (xc->start_time == 0) && (xc->end_time == 0);
	
				fstFread(&dcheck, 8, 1, xc->f);
				xc->double_endian_match = (dcheck == FST_DOUBLE_ENDTEST);
				if(!xc->double_endian_match)
					{
					union	{
	  					unsigned char rvs_buf[8];
	  					double d;
	  					} vu;
	
					unsigned char *dcheck_alias = (unsigned char *)&dcheck;
					int rvs_idx;
	
					for(rvs_idx=0;rvs_idx<8;rvs_idx++)
						{
						vu.rvs_buf[rvs_idx] = dcheck_alias[7-rvs_idx];
						}
					if(vu.d != FST_DOUBLE_ENDTEST)
						{
						break; /* either corrupt file or wrong architecture (offset +33 also functions as matchword) */
						}
					}
	
				hdr_seen = 1;
	
				xc->mem_used_by_writer = fstReaderUint64(xc->f); 
				xc->scope_count = fstReaderUint64(xc->f); 
				xc->var_count = fstReaderUint64(xc->f); 
				xc->maxhandle = fstReaderUint64(xc->f); 
				xc->num_alias = xc->var_count - xc->maxhandle;
				xc->vc_section_count = fstReaderUint64(xc->f); 
				ch = fgetc(xc->f);
				xc->timescale = (signed char)ch;
				fstFread(xc->version, FST_HDR_SIM_VERSION_SIZE, 1, xc->f);
				xc->version[FST_HDR_SIM_VERSION_SIZE] = 0;
				fstFread(xc->date, FST_HDR_DATE_SIZE, 1, xc->f);
				xc->date[FST_HDR_DATE_SIZE] = 0;
				xc->timezero = fstReaderUint64(xc->f);
				}
			}
		else if((sectype == FST_BL_VCDATA) || (sectype == FST_BL_VCDATA_DYN_ALIAS))
			{
			if(hdr_incomplete)
				{
				uint64_t bt = fstReaderUint64(xc->f);
				xc->end_time = fstReaderUint64(xc->f);
		
				if(!vc_section_count_actual) { xc->start_time = bt; }
				}

			vc_section_count_actual++;
			}
		else if(sectype == FST_BL_GEOM)
			{
			if(!hdr_incomplete)
				{
				uint64_t clen = seclen - 24;
				uint64_t uclen = fstReaderUint64(xc->f);
				unsigned char *ucdata = malloc(uclen);
				unsigned char *pnt = ucdata;
				int i;
	
				xc->contains_geom_section = 1;
				xc->maxhandle = fstReaderUint64(xc->f);
				xc->longest_signal_value_len = 32; /* arbitrarily set at 32...this is much longer than an expanded double */

				free(xc->process_mask);
				xc->process_mask = calloc(1, (xc->maxhandle+7)/8);

				if(clen != uclen)
					{
					unsigned char *cdata = malloc(clen);
				        unsigned long destlen = uclen;
				        unsigned long sourcelen = clen;
					int rc;
	
					fstFread(cdata, clen, 1, xc->f);
					rc = uncompress(ucdata, &destlen, cdata, sourcelen);

					if(rc != Z_OK)
						{
						printf("geom uncompress rc = %d\n", rc);
						exit(255);
						}
					
					free(cdata);
					}
					else
					{
					fstFread(ucdata, uclen, 1, xc->f);
					}
		
				free(xc->signal_lens);
				xc->signal_lens = malloc(sizeof(uint32_t) * xc->maxhandle);
				free(xc->signal_typs);
				xc->signal_typs = malloc(sizeof(unsigned char) * xc->maxhandle);
	
				for(i=0;i<xc->maxhandle;i++)
					{
			                int skiplen;
		               		uint64_t val = fstGetVarint32(pnt, &skiplen);
	
					pnt += skiplen;
	
					if(val)
						{
						xc->signal_lens[i] = (val != 0xFFFFFFFF) ? val : 0;
						xc->signal_typs[i] = FST_VT_VCD_WIRE;
						if(xc->signal_lens[i] > xc->longest_signal_value_len)
	                                        	{
	                                        	xc->longest_signal_value_len = xc->signal_lens[i];
	                                        	}
						}
						else
						{
						xc->signal_lens[i] = 8; /* backpatch in real */
						xc->signal_typs[i] = FST_VT_VCD_REAL;
						/* xc->longest_signal_value_len handled above by overly large init size */
						}
					}

				free(xc->temp_signal_value_buf);
				xc->temp_signal_value_buf = malloc(xc->longest_signal_value_len + 1); 
	
				free(ucdata);
				}
			}
		else if(sectype == FST_BL_HIER)
			{
			xc->contains_hier_section = 1;
			xc->hier_pos = ftello(xc->f);
			}
		else if(sectype == FST_BL_BLACKOUT)
			{
			uint32_t i;
			uint64_t cur_bl = 0;
			uint64_t delta;

			xc->num_blackouts = fstReaderVarint32(xc->f);
			free(xc->blackout_times);
			xc->blackout_times = calloc(xc->num_blackouts, sizeof(uint64_t));
			free(xc->blackout_activity);
			xc->blackout_activity = calloc(xc->num_blackouts, sizeof(unsigned char));

			for(i=0;i<xc->num_blackouts;i++)
				{
				xc->blackout_activity[i] = fgetc(xc->f) != 0;
				delta = fstReaderVarint64(xc->f);
				cur_bl += delta;
				xc->blackout_times[i] = cur_bl;
				}
			}
	
		blkpos += seclen;
		if(!hdr_seen) break;
		}

	if(hdr_seen)
		{
		if(xc->vc_section_count != vc_section_count_actual)
			{
			xc->vc_section_count = vc_section_count_actual;
			}
	
		if(!xc->contains_geom_section)
			{
			fstReaderProcessHier(xc, NULL); /* recreate signal_lens/signal_typs info */
			}
		}
	}

return(hdr_seen);
}


void *fstReaderOpenForUtilitiesOnly(void)
{
struct fstReaderContext *xc = calloc(1, sizeof(struct fstReaderContext));

return(xc);
}


void *fstReaderOpen(const char *nam)
{
struct fstReaderContext *xc = calloc(1, sizeof(struct fstReaderContext));

if((!nam)||(!(xc->f=fopen(nam, "rb"))))
        {
        free(xc);
        xc=NULL;
        }
        else
        {
        int flen = strlen(nam);
        char *hf = calloc(1, flen + 6);
	int rc;

#if defined(__MINGW32__) || defined(FST_MACOSX)
	setvbuf(xc->f, (char *)NULL, _IONBF, 0);   /* keeps gzip from acting weird in tandem with fopen */
#endif

        memcpy(hf, nam, flen);
        strcpy(hf + flen, ".hier");
        xc->fh = fopen(hf, "rb");

	free(hf);
	xc->filename = strdup(nam);
	rc = fstReaderInit(xc);

	if((rc) && (xc->vc_section_count) && (xc->maxhandle) && ((xc->fh)||(xc->contains_hier_section)))
		{
		/* more init */
		xc->do_rewind = 1;
		}
		else
		{
		fstReaderClose(xc);
		xc = NULL;
		}
	}

return(xc);
}


static void fstReaderDeallocateRvatData(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
if(xc)
	{
	free(xc->rvat_chain_mem); xc->rvat_chain_mem = NULL;
	free(xc->rvat_frame_data); xc->rvat_frame_data = NULL;
	free(xc->rvat_time_table); xc->rvat_time_table = NULL;
	free(xc->rvat_chain_table); xc->rvat_chain_table = NULL;
	free(xc->rvat_chain_table_lengths); xc->rvat_chain_table_lengths = NULL;

	xc->rvat_data_valid = 0;
	}
}


void fstReaderClose(void *ctx)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

if(xc)
	{
	fstReaderDeallocateScopeData(xc);
	fstReaderDeallocateRvatData(xc);
	free(xc->rvat_sig_offs); xc->rvat_sig_offs = NULL;

	free(xc->process_mask); xc->process_mask = NULL;
	free(xc->blackout_times); xc->blackout_times = NULL;
	free(xc->blackout_activity); xc->blackout_activity = NULL;
	free(xc->temp_signal_value_buf); xc->temp_signal_value_buf = NULL;
	free(xc->signal_typs); xc->signal_typs = NULL;
	free(xc->signal_lens); xc->signal_lens = NULL;
	free(xc->filename); xc->filename = NULL;

	if(xc->fh) 
		{ 
		fclose(xc->fh); xc->fh = NULL;
		}

	if(xc->f) 
		{ 
		fclose(xc->f); xc->f = NULL; 
		if(xc->filename_unpacked)
			{
			unlink(xc->filename_unpacked);
			free(xc->filename_unpacked);
			}
		}

	free(xc);
	}
}


/*
 * read processing
 */

/* normal read which re-interleaves the value change data */
int fstReaderIterBlocks(void *ctx,
        void (*value_change_callback)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value),
        void *user_callback_data_pointer, FILE *fv)
{
return(fstReaderIterBlocks2(ctx, value_change_callback, NULL, user_callback_data_pointer, fv));
}


int fstReaderIterBlocks2(void *ctx,
        void (*value_change_callback)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value),
	void (*value_change_callback_varlen)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value, uint32_t len),
        void *user_callback_data_pointer, FILE *fv)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;

uint64_t previous_time = UINT64_MAX;
uint64_t *time_table = NULL;
uint64_t tsec_nitems;
int secnum = 0;
int blocks_skipped = 0;
off_t blkpos = 0;
uint64_t seclen, beg_tim;
#ifdef FST_DEBUG
uint64_t end_tim;
#endif
uint64_t frame_uclen, frame_clen, frame_maxhandle, vc_maxhandle; 
off_t vc_start;
off_t indx_pntr, indx_pos;
off_t *chain_table = NULL;
uint32_t *chain_table_lengths = NULL;
unsigned char *chain_cmem;
unsigned char *pnt;
long chain_clen;
fstHandle idx, pidx=0, i;
uint64_t pval;
uint64_t vc_maxhandle_largest = 0;
uint64_t tsec_uclen = 0, tsec_clen = 0;
int sectype;
uint64_t mem_required_for_traversal;
unsigned char *mem_for_traversal = NULL;
uint32_t traversal_mem_offs;
uint32_t *scatterptr, *headptr, *length_remaining;
uint32_t cur_blackout = 0;
int packtype;

if(!xc) return(0);

scatterptr = calloc(xc->maxhandle, sizeof(uint32_t));
headptr = calloc(xc->maxhandle, sizeof(uint32_t));
length_remaining = calloc(xc->maxhandle, sizeof(uint32_t));

for(;;)
	{
	uint32_t *tc_head = NULL;
	traversal_mem_offs = 0;

	fseeko(xc->f, blkpos, SEEK_SET);
	
	sectype = fgetc(xc->f);
	seclen = fstReaderUint64(xc->f);

	if((sectype == EOF) || (sectype == FST_BL_SKIP))
		{
#ifdef FST_DEBUG
		printf("<< EOF >>\n");
#endif
		break;
		}

	blkpos++;
	if((sectype != FST_BL_VCDATA) && (sectype != FST_BL_VCDATA_DYN_ALIAS))
		{
		blkpos += seclen;
		continue;
		}

	if(!seclen) break;

	beg_tim = fstReaderUint64(xc->f);
#ifdef FST_DEBUG
	end_tim = 
#endif
	fstReaderUint64(xc->f);

	if(xc->limit_range_valid)
		{
		if(beg_tim < xc->limit_range_start)
			{
			blocks_skipped++;
			blkpos += seclen;
			continue;
			}

		if(beg_tim > xc->limit_range_end) /* likely the compare in for(i=0;i<tsec_nitems;i++) below would do this earlier */
			{
			break;
			}
		}


	mem_required_for_traversal = fstReaderUint64(xc->f);
	mem_for_traversal = malloc(mem_required_for_traversal + 66); /* add in potential fastlz overhead */
#ifdef FST_DEBUG
	printf("sec: %d seclen: %d begtim: %d endtim: %d\n",
		secnum, (int)seclen, (int)beg_tim, (int)end_tim);
	printf("\tmem_required_for_traversal: %d\n", (int)mem_required_for_traversal);
#endif
	/* process time block */
	{
	unsigned char *ucdata;
	unsigned char *cdata;
	unsigned long destlen /* = tsec_uclen */; /* scan-build */
	unsigned long sourcelen /*= tsec_clen */; /* scan-build */
	int rc;
	unsigned char *tpnt;
	uint64_t tpval;
	int ti;

	fseeko(xc->f, blkpos + seclen - 24, SEEK_SET);
	tsec_uclen = fstReaderUint64(xc->f);
	tsec_clen = fstReaderUint64(xc->f);
	tsec_nitems = fstReaderUint64(xc->f);
#ifdef FST_DEBUG
	printf("\ttime section unc: %d, com: %d (%d items)\n", 
		(int)tsec_uclen, (int)tsec_clen, (int)tsec_nitems);
#endif		
	ucdata = malloc(tsec_uclen);
	destlen = tsec_uclen;
	sourcelen = tsec_clen;

	fseeko(xc->f, -24 - ((off_t)tsec_clen), SEEK_CUR);

	if(tsec_uclen != tsec_clen)
		{
		cdata = malloc(tsec_clen);
		fstFread(cdata, tsec_clen, 1, xc->f);
	
		rc = uncompress(ucdata, &destlen, cdata, sourcelen);
	                
		if(rc != Z_OK)
			{
			printf("tsec uncompress rc = %d\n", rc);
			exit(255);
			}
	
		free(cdata);
		}
		else
		{
		fstFread(ucdata, tsec_uclen, 1, xc->f);
		}
	
	free(time_table);
	time_table = calloc(tsec_nitems, sizeof(uint64_t));
	tpnt = ucdata;
	tpval = 0;
	for(ti=0;ti<tsec_nitems;ti++)
		{
		int skiplen;
		uint64_t val = fstGetVarint64(tpnt, &skiplen);
		tpval = time_table[ti] = tpval + val;
		tpnt += skiplen;	
		}

	tc_head = calloc(tsec_nitems, sizeof(uint32_t));
	free(ucdata);
	}

	fseeko(xc->f, blkpos+32, SEEK_SET);

	frame_uclen = fstReaderVarint64(xc->f);
	frame_clen = fstReaderVarint64(xc->f);
	frame_maxhandle = fstReaderVarint64(xc->f);

	if(secnum == 0)
		{
		if((beg_tim != time_table[0]) || (blocks_skipped))
			{
			unsigned char *mu = malloc(frame_uclen);
			uint32_t sig_offs = 0;

			if(fv)
				{
				if(beg_tim) { fprintf(fv, "#%"PRIu64"\n", beg_tim); }
				if((xc->num_blackouts)&&(cur_blackout != xc->num_blackouts))
					{
					if(beg_tim == xc->blackout_times[cur_blackout])
						{
						fprintf(fv, "$dump%s $end\n", (xc->blackout_activity[cur_blackout++]) ? "on" : "off");
						}
					}
				}

			if(frame_uclen == frame_clen)
				{
				fstFread(mu, frame_uclen, 1, xc->f);
				}
				else
				{
				unsigned char *mc = malloc(frame_clen);
				int rc;

				unsigned long destlen = frame_uclen;
				unsigned long sourcelen = frame_clen;

				fstFread(mc, sourcelen, 1, xc->f);
				rc = uncompress(mu, &destlen, mc, sourcelen);
				if(rc != Z_OK)
					{
					printf("rc: %d\n", rc);
					exit(255);
					}
				free(mc);
				}


			for(idx=0;idx<frame_maxhandle;idx++)
				{
		                int process_idx = idx/8;
		                int process_bit = idx&7;

		                if(xc->process_mask[process_idx]&(1<<process_bit))
					{
					if(xc->signal_lens[idx] <= 1)
						{
						if(xc->signal_lens[idx] == 1)
							{
							unsigned char val = mu[sig_offs];
							if(value_change_callback)
								{
								xc->temp_signal_value_buf[0] = val;
								xc->temp_signal_value_buf[1] = 0;
								value_change_callback(user_callback_data_pointer, beg_tim, idx+1, xc->temp_signal_value_buf);
								}
								else
								{
								if(fv)
									{
									int vcdid_len;
									const char *vcd_id = fstVcdIDForFwrite(idx+1, &vcdid_len);
									fputc(val, fv);
									fstFwrite(vcd_id, vcdid_len, 1, fv);
									fputc('\n', fv);
									}
								}
							}
							else
							{
							/* variable-length ("0" length) records have no initial state */
							}
						}
						else
						{
						if(xc->signal_typs[idx] != FST_VT_VCD_REAL)
							{
							if(value_change_callback)
								{
								memcpy(xc->temp_signal_value_buf, mu+sig_offs, xc->signal_lens[idx]);
								xc->temp_signal_value_buf[xc->signal_lens[idx]] = 0;
								value_change_callback(user_callback_data_pointer, beg_tim, idx+1, xc->temp_signal_value_buf);
								}
								else
								{
								if(fv)
									{
									int vcdid_len;
									const char *vcd_id = fstVcdIDForFwrite(idx+1, &vcdid_len);
									fputc((xc->signal_typs[idx] != FST_VT_VCD_PORT) ? 'b' : 'p', fv);
									fstFwrite(mu+sig_offs, xc->signal_lens[idx], 1, fv);
									fputc(' ', fv);
									fstFwrite(vcd_id, vcdid_len, 1, fv);
									fputc('\n', fv);
									}
								}
							}
							else
							{
							double d;
							unsigned char *clone_d;
							unsigned char *srcdata = mu+sig_offs;
		
							if(value_change_callback)
								{
								if(xc->native_doubles_for_cb)
									{
									if(xc->double_endian_match)
										{
										clone_d = srcdata;
										}
										else
										{
										int j;
		
										clone_d = (unsigned char *)&d;
										for(j=0;j<8;j++)
											{
											clone_d[j] = srcdata[7-j];
											}
										}
									value_change_callback(user_callback_data_pointer, beg_tim, idx+1, clone_d);
									}
									else
									{
									clone_d = (unsigned char *)&d;
									if(xc->double_endian_match)
										{
										memcpy(clone_d, srcdata, 8);
										}
										else
										{
										int j;
		
										for(j=0;j<8;j++)
											{
											clone_d[j] = srcdata[7-j];
											}
										}
									sprintf((char *)xc->temp_signal_value_buf, "%.16g", d);
									value_change_callback(user_callback_data_pointer, beg_tim, idx+1, xc->temp_signal_value_buf);
									}
								}
								else
								{
								if(fv)
									{
									clone_d = (unsigned char *)&d;
									if(xc->double_endian_match)
										{
										memcpy(clone_d, srcdata, 8);
										}
										else
										{
										int j;
		
										for(j=0;j<8;j++)
											{
											clone_d[j] = srcdata[7-j];
											}
										}
						
									fprintf(fv, "r%.16g %s\n", d, fstVcdID(idx+1));
									}
								}
							}
						}
					}	

				sig_offs += xc->signal_lens[idx];
				}

			free(mu);
			fseeko(xc->f, -((off_t)frame_clen), SEEK_CUR);
			}
		}

	fseeko(xc->f, (off_t)frame_clen, SEEK_CUR); /* skip past compressed data */

	vc_maxhandle = fstReaderVarint64(xc->f);
	vc_start = ftello(xc->f);	/* points to '!' character */
	packtype = fgetc(xc->f);

#ifdef FST_DEBUG
	printf("\tframe_uclen: %d, frame_clen: %d, frame_maxhandle: %d\n",
		(int)frame_uclen, (int)frame_clen, (int)frame_maxhandle);
	printf("\tvc_maxhandle: %d, packtype: %c\n", (int)vc_maxhandle, packtype);
#endif

	indx_pntr = blkpos + seclen - 24 -tsec_clen -8;
	fseeko(xc->f, indx_pntr, SEEK_SET);
	chain_clen = fstReaderUint64(xc->f);
	indx_pos = indx_pntr - chain_clen;
#ifdef FST_DEBUG
	printf("\tindx_pos: %d (%d bytes)\n", (int)indx_pos, (int)chain_clen);
#endif
	chain_cmem = malloc(chain_clen);
	if(!chain_cmem) goto block_err;
	fseeko(xc->f, indx_pos, SEEK_SET);
	fstFread(chain_cmem, chain_clen, 1, xc->f);
	
	if(vc_maxhandle > vc_maxhandle_largest)
		{
		free(chain_table);
		free(chain_table_lengths);

		vc_maxhandle_largest = vc_maxhandle;
		chain_table = calloc((vc_maxhandle+1), sizeof(off_t));
		chain_table_lengths = calloc((vc_maxhandle+1), sizeof(uint32_t));
		}

	if(!chain_table || !chain_table_lengths) goto block_err;

	pnt = chain_cmem;
	idx = 0;
	pval = 0;

	do
		{
		int skiplen;
		uint64_t val = fstGetVarint32(pnt, &skiplen);
		
		if(!val)
			{
			pnt += skiplen;
			val = fstGetVarint32(pnt, &skiplen);
			chain_table[idx] = 0;			/* need to explicitly zero as calloc above might not run */
			chain_table_lengths[idx] = -val;	/* because during this loop iter would give stale data! */
			idx++;
			}
		else 
		if(val&1)
			{
			pval = chain_table[idx] = pval + (val >> 1);
			if(idx) { chain_table_lengths[pidx] = pval - chain_table[pidx]; }
			pidx = idx++;
			}
		else
			{
			int loopcnt = val >> 1;
			for(i=0;i<loopcnt;i++)
				{
				chain_table[idx++] = 0;
				}
			}
		
		pnt += skiplen;
		} while (pnt != (chain_cmem + chain_clen));
	chain_table[idx] = indx_pos - vc_start;
	chain_table_lengths[pidx] = chain_table[idx] - chain_table[pidx];

	for(i=0;i<idx;i++)
		{
		int32_t v32 = chain_table_lengths[i];
		if((v32 < 0) && (!chain_table[i]))
			{
			v32 = -v32;
			v32--;
			if(((uint32_t)v32) < i) /* sanity check */
				{
				chain_table[i] = chain_table[v32];
				chain_table_lengths[i] = chain_table_lengths[v32];	
				}
			}
		}

#ifdef FST_DEBUG
	printf("\tdecompressed chain idx len: %"PRIu32"\n", idx);
#endif
	/* check compressed VC data */
	if(idx > xc->maxhandle) idx = xc->maxhandle;
	for(i=0;i<idx;i++)
		{
		if(chain_table[i])
			{
	                int process_idx = i/8;
	                int process_bit = i&7;

	                if(xc->process_mask[process_idx]&(1<<process_bit))
				{
				int rc = Z_OK;
				uint32_t val;
				uint32_t skiplen;
				uint32_t tdelta;
	
				fseeko(xc->f, vc_start + chain_table[i], SEEK_SET);
				val = fstReaderVarint32WithSkip(xc->f, &skiplen);
				if(val)
					{
					unsigned char *mu = mem_for_traversal + traversal_mem_offs;
					unsigned char *mc = malloc(chain_table_lengths[i]);
					unsigned long destlen = val;
					unsigned long sourcelen = chain_table_lengths[i];
	
					fstFread(mc, chain_table_lengths[i], 1, xc->f);
					if(packtype == 'F')
						{
						rc = fastlz_decompress(mc, sourcelen, mu, destlen);
						}
						else
						{
						rc = uncompress(mu, &destlen, mc, sourcelen);
						}
					free(mc);
					/* data to process is for(j=0;j<destlen;j++) in mu[j] */
					headptr[i] = traversal_mem_offs;
					length_remaining[i] = val;
					traversal_mem_offs += val;
					}
					else
					{
					int destlen = chain_table_lengths[i] - skiplen;
					unsigned char *mu = mem_for_traversal + traversal_mem_offs;
					fstFread(mu, destlen, 1, xc->f);
					/* data to process is for(j=0;j<destlen;j++) in mu[j] */
					headptr[i] = traversal_mem_offs;
					length_remaining[i] = destlen;
					traversal_mem_offs += destlen;
					}
	
				if(rc != Z_OK)
					{
					printf("\tfac: %d clen: %d (rc=%d)\n", (int)i, (int)val, rc);
					exit(255);
					}
	
				if(xc->signal_lens[i] == 1)
					{
					uint32_t vli = fstGetVarint32NoSkip(mem_for_traversal + headptr[i]);
					uint32_t shcnt = 2 << (vli & 1);
					tdelta = vli >> shcnt;
					}
					else
					{
					uint32_t vli = fstGetVarint32NoSkip(mem_for_traversal + headptr[i]);
					tdelta = vli >> 1;
					}
	
				scatterptr[i] = tc_head[tdelta];
				tc_head[tdelta] = i+1;
				}
			}
		}

	for(i=0;i<tsec_nitems;i++)
		{
		uint32_t tdelta;
		int skiplen, skiplen2;
		uint32_t vli;

		if(fv)
			{
			if(time_table[i] != previous_time)
				{
				if(xc->limit_range_valid)
					{
					if(time_table[i] > xc->limit_range_end)
						{
						break;
						}
					}

				fprintf(fv, "#%"PRIu64"\n", time_table[i]);
				if((xc->num_blackouts)&&(cur_blackout != xc->num_blackouts))
					{
					if(time_table[i] == xc->blackout_times[cur_blackout])
						{
						fprintf(fv, "$dump%s $end\n", (xc->blackout_activity[cur_blackout++]) ? "on" : "off");
						}
					}
				previous_time = time_table[i];
				}
			}
	
		while(tc_head[i])
			{
			idx = tc_head[i] - 1;
			vli = fstGetVarint32(mem_for_traversal + headptr[idx], &skiplen);

			if(xc->signal_lens[idx] <= 1)
				{
				if(xc->signal_lens[idx] == 1)
					{
					unsigned char val;
					if(!(vli & 1))
						{
						/* tdelta = vli >> 2; */ /* scan-build */
						val = ((vli >> 1) & 1) | '0'; 
						}
						else
						{
						/* tdelta = vli >> 4; */ /* scan-build */
						val = FST_RCV_STR[((vli >> 1) & 7)];
						}

					if(value_change_callback)
						{
						xc->temp_signal_value_buf[0] = val;
						xc->temp_signal_value_buf[1] = 0;
						value_change_callback(user_callback_data_pointer, time_table[i], idx+1, xc->temp_signal_value_buf);
						}
						else
						{
						if(fv) 
							{
							int vcdid_len;
							const char *vcd_id = fstVcdIDForFwrite(idx+1, &vcdid_len);
							fputc(val, fv);
							fstFwrite(vcd_id, vcdid_len, 1, fv);
							fputc('\n', fv);
							}
						}
					headptr[idx] += skiplen;
					length_remaining[idx] -= skiplen;
	
					tc_head[i] = scatterptr[idx];
					scatterptr[idx] = 0;
		
					if(length_remaining[idx])
						{
						int shamt;
						vli = fstGetVarint32NoSkip(mem_for_traversal + headptr[idx]);
						shamt = 2 << (vli & 1);
						tdelta = vli >> shamt;
	
						scatterptr[idx] = tc_head[i+tdelta];
						tc_head[i+tdelta] = idx+1;
						}
					}
					else
					{
					unsigned char *vdata;
					uint32_t len;

					vli = fstGetVarint32(mem_for_traversal + headptr[idx], &skiplen);
					len = fstGetVarint32(mem_for_traversal + headptr[idx] + skiplen, &skiplen2);
					/* tdelta = vli >> 1; */ /* scan-build */
					skiplen += skiplen2;
					vdata = mem_for_traversal + headptr[idx] + skiplen;

					if(!(vli & 1))
						{
						if(value_change_callback_varlen)
							{
							value_change_callback_varlen(user_callback_data_pointer, time_table[i], idx+1, vdata, len);
							}
							else
							{
							if(fv) 
								{
								int vcdid_len;
								const char *vcd_id = fstVcdIDForFwrite(idx+1, &vcdid_len);
	
								fputc('s', fv);
								{
								unsigned char *vesc = malloc(len*4 + 1);
								int vlen = fstUtilityBinToEsc(vesc, vdata, len);

								vesc[vlen] = 0;
								fstFwrite(vesc, vlen, 1, fv);
								free(vesc);
								}
								fputc(' ', fv);
								fstFwrite(vcd_id, vcdid_len, 1, fv);
								fputc('\n', fv);
								}
							}
						}

					skiplen += len;
					headptr[idx] += skiplen;
					length_remaining[idx] -= skiplen;

					tc_head[i] = scatterptr[idx];
					scatterptr[idx] = 0;
	
					if(length_remaining[idx])
						{
						vli = fstGetVarint32NoSkip(mem_for_traversal + headptr[idx]);
						tdelta = vli >> 1;
	
						scatterptr[idx] = tc_head[i+tdelta];
						tc_head[i+tdelta] = idx+1;
						}
					}
				}
				else
				{
				uint32_t len = xc->signal_lens[idx];
				unsigned char *vdata;

				vli = fstGetVarint32(mem_for_traversal + headptr[idx], &skiplen);
				/* tdelta = vli >> 1; */ /* scan-build */
				vdata = mem_for_traversal + headptr[idx] + skiplen;

				if(xc->signal_typs[idx] != FST_VT_VCD_REAL)
					{
					if(!(vli & 1))
						{
						int byte = 0;
						int bit;
						int j;

						for(j=0;j<len;j++)
							{
							unsigned char ch;
							byte = j/8;
							bit = 7 - (j & 7);
							ch = ((vdata[byte] >> bit) & 1) | '0';
							xc->temp_signal_value_buf[j] = ch;
							}
						xc->temp_signal_value_buf[j] = 0;

						if(value_change_callback)
							{
							value_change_callback(user_callback_data_pointer, time_table[i], idx+1, xc->temp_signal_value_buf);
							}
							else
							{
							if(fv)	{ 
								fputc((xc->signal_typs[idx] != FST_VT_VCD_PORT) ? 'b' : 'p', fv);
								fstFwrite(xc->temp_signal_value_buf, len, 1, fv);
								}
							}

						len = byte+1;
						}
						else
						{
						if(value_change_callback)
							{
							memcpy(xc->temp_signal_value_buf, vdata, len);
							xc->temp_signal_value_buf[len] = 0;
							value_change_callback(user_callback_data_pointer, time_table[i], idx+1, xc->temp_signal_value_buf);
							}
							else
							{
							if(fv)
								{
								fputc((xc->signal_typs[idx] != FST_VT_VCD_PORT) ? 'b' : 'p', fv);
								fstFwrite(vdata, len, 1, fv);
								}
							}
						}
					}
					else
					{
					double d;
					unsigned char *clone_d /*= (unsigned char *)&d */; /* scan-build */
					unsigned char buf[8];
					unsigned char *srcdata;

					if(!(vli & 1))	/* very rare case, but possible */
						{
						int bit;
						int j;

						for(j=0;j<8;j++)
							{
							unsigned char ch;
							bit = 7 - (j & 7);
							ch = ((vdata[0] >> bit) & 1) | '0';
							buf[j] = ch;
							}
	
						len = 1;
						srcdata = buf;
						}
						else
						{
						srcdata = vdata;
						}

					if(value_change_callback)
						{
						if(xc->native_doubles_for_cb)
							{
							if(xc->double_endian_match)
								{
								clone_d = srcdata;
								}
								else
								{
								int j;
		
								clone_d = (unsigned char *)&d;
								for(j=0;j<8;j++)
									{
									clone_d[j] = srcdata[7-j];
									}
								}
							value_change_callback(user_callback_data_pointer, time_table[i], idx+1, clone_d);
							}
							else
							{
							clone_d = (unsigned char *)&d;
							if(xc->double_endian_match)
								{
								memcpy(clone_d, srcdata, 8);
								}
								else
								{
								int j;
		
								for(j=0;j<8;j++)
									{
									clone_d[j] = srcdata[7-j];
									}
								}
							sprintf((char *)xc->temp_signal_value_buf, "%.16g", d);
							value_change_callback(user_callback_data_pointer, time_table[i], idx+1, xc->temp_signal_value_buf);
							}
						}
						else
						{
						if(fv)
							{
							clone_d = (unsigned char *)&d;
							if(xc->double_endian_match)
								{
								memcpy(clone_d, srcdata, 8);
								}
								else
								{
								int j;
		
								for(j=0;j<8;j++)
									{
									clone_d[j] = srcdata[7-j];
									}
								}
						
							fprintf(fv, "r%.16g", d);
							}
						}
					}

				if(fv) 
					{
					int vcdid_len;
					const char *vcd_id = fstVcdIDForFwrite(idx+1, &vcdid_len);
					fputc(' ', fv);
					fstFwrite(vcd_id, vcdid_len, 1, fv);
					fputc('\n', fv);
					}

				skiplen += len;
				headptr[idx] += skiplen;
				length_remaining[idx] -= skiplen;

				tc_head[i] = scatterptr[idx];
				scatterptr[idx] = 0;
	
				if(length_remaining[idx])
					{
					vli = fstGetVarint32NoSkip(mem_for_traversal + headptr[idx]);
					tdelta = vli >> 1;

					scatterptr[idx] = tc_head[i+tdelta];
					tc_head[i+tdelta] = idx+1;
					}
				}
			}
		}

block_err:
	free(tc_head);
	free(chain_cmem);
	free(mem_for_traversal);

	secnum++;
	if(secnum == xc->vc_section_count) break; /* in case file is growing, keep with original block count */
	blkpos += seclen;
	}

free(length_remaining);
free(headptr);
free(scatterptr);

if(chain_table)
	{
	free(chain_table);
	free(chain_table_lengths);
	}

free(time_table);

return(1);
}


/* rvat functions */

static char *fstExtractRvatDataFromFrame(struct fstReaderContext *xc, fstHandle facidx, char *buf)
{
if(facidx >= xc->rvat_frame_maxhandle)
	{
	return(NULL);
	}

if(xc->signal_lens[facidx] == 1)
	{
	buf[0] = (char)xc->rvat_frame_data[xc->rvat_sig_offs[facidx]];
	buf[1] = 0;
	}
	else
	{
	if(xc->signal_typs[facidx] != FST_VT_VCD_REAL)
		{
		memcpy(buf, xc->rvat_frame_data + xc->rvat_sig_offs[facidx], xc->signal_lens[facidx]);
		buf[xc->signal_lens[facidx]] = 0;
		}
		else
		{
		double d;
		unsigned char *clone_d = (unsigned char *)&d;
		unsigned char *srcdata = xc->rvat_frame_data + xc->rvat_sig_offs[facidx];

		if(xc->double_endian_match)
			{
			memcpy(clone_d, srcdata, 8);
			}
			else
			{
			int j;

			for(j=0;j<8;j++)
				{
				clone_d[j] = srcdata[7-j];
				}
			}
						
		sprintf((char *)buf, "%.16g", d);
		}
	}

return(buf);
}


char *fstReaderGetValueFromHandleAtTime(void *ctx, uint64_t tim, fstHandle facidx, char *buf)
{
struct fstReaderContext *xc = (struct fstReaderContext *)ctx;
off_t blkpos = 0, prev_blkpos;
uint64_t beg_tim, end_tim, beg_tim2, end_tim2;
int sectype;
int secnum = 0;
uint64_t seclen;
uint64_t tsec_uclen = 0, tsec_clen = 0;
uint64_t tsec_nitems;
uint64_t frame_uclen, frame_clen;
#ifdef FST_DEBUG
uint64_t mem_required_for_traversal;
#endif
off_t indx_pntr, indx_pos;
long chain_clen;
unsigned char *chain_cmem;
unsigned char *pnt;
fstHandle idx, pidx=0, i;
uint64_t pval;

if((!xc) || (!facidx) || (facidx > xc->maxhandle) || (!buf) || (!xc->signal_lens[facidx-1]))
	{
	return(NULL);
	}

if(!xc->rvat_sig_offs)
	{
	uint32_t cur_offs = 0;

	xc->rvat_sig_offs = calloc(xc->maxhandle, sizeof(uint32_t));
	for(i=0;i<xc->maxhandle;i++)
		{
		xc->rvat_sig_offs[i] = cur_offs;
		cur_offs += xc->signal_lens[i];
		}
	}

if(xc->rvat_data_valid)
	{
	if((xc->rvat_beg_tim <= tim) && (tim <= xc->rvat_end_tim))
		{
		goto process_value;
		}

	fstReaderDeallocateRvatData(xc);
	}

xc->rvat_chain_pos_valid = 0;

for(;;)
	{
	fseeko(xc->f, (prev_blkpos = blkpos), SEEK_SET);

	sectype = fgetc(xc->f);
	seclen = fstReaderUint64(xc->f);

	if((sectype == EOF) || (sectype == FST_BL_SKIP) || (!seclen))
		{
		return(NULL); /* if this loop exits on break, it's successful */
		}

	blkpos++;
	if((sectype != FST_BL_VCDATA) && (sectype != FST_BL_VCDATA_DYN_ALIAS))
		{
		blkpos += seclen;
		continue;
		}

	beg_tim = fstReaderUint64(xc->f);
	end_tim = fstReaderUint64(xc->f);

	if((beg_tim <= tim) && (tim <= end_tim))
		{
		if((tim == end_tim) && (tim != xc->end_time))
			{
			off_t cached_pos = ftello(xc->f);
			fseeko(xc->f, blkpos, SEEK_SET);

			sectype = fgetc(xc->f);
			seclen = fstReaderUint64(xc->f);

			beg_tim2 = fstReaderUint64(xc->f);
			end_tim2 = fstReaderUint64(xc->f);

			if(((sectype != FST_BL_VCDATA)&&(sectype != FST_BL_VCDATA_DYN_ALIAS)) || (!seclen) || (beg_tim2 != tim))
				{
				blkpos = prev_blkpos;
				break;
				}
			beg_tim = beg_tim2;
			end_tim = end_tim2;
			fseeko(xc->f, cached_pos, SEEK_SET);
			}
		break;
		}

	blkpos += seclen;
	secnum++;
	}

xc->rvat_beg_tim = beg_tim;
xc->rvat_end_tim = end_tim;

#ifdef FST_DEBUG
mem_required_for_traversal = 
#endif
	fstReaderUint64(xc->f);

#ifdef FST_DEBUG
printf("rvat sec: %d seclen: %d begtim: %d endtim: %d\n",
	secnum, (int)seclen, (int)beg_tim, (int)end_tim);
printf("\tmem_required_for_traversal: %d\n", (int)mem_required_for_traversal);
#endif

/* process time block */
{
unsigned char *ucdata;
unsigned char *cdata;
unsigned long destlen /* = tsec_uclen */; /* scan-build */
unsigned long sourcelen /* = tsec_clen */; /* scan-build */
int rc;
unsigned char *tpnt;
uint64_t tpval;
int ti;

fseeko(xc->f, blkpos + seclen - 24, SEEK_SET);
tsec_uclen = fstReaderUint64(xc->f);
tsec_clen = fstReaderUint64(xc->f);
tsec_nitems = fstReaderUint64(xc->f);
#ifdef FST_DEBUG
printf("\ttime section unc: %d, com: %d (%d items)\n", 
	(int)tsec_uclen, (int)tsec_clen, (int)tsec_nitems);
#endif		
ucdata = malloc(tsec_uclen);
destlen = tsec_uclen;
sourcelen = tsec_clen;
	
fseeko(xc->f, -24 - ((off_t)tsec_clen), SEEK_CUR);
if(tsec_uclen != tsec_clen)
	{
	cdata = malloc(tsec_clen);
	fstFread(cdata, tsec_clen, 1, xc->f);
	
	rc = uncompress(ucdata, &destlen, cdata, sourcelen);
	                
	if(rc != Z_OK)
		{
		printf("tsec uncompress rc = %d\n", rc);
		exit(255);
		}
	
	free(cdata);
	}
	else
	{
	fstFread(ucdata, tsec_uclen, 1, xc->f);
	}

xc->rvat_time_table = calloc(tsec_nitems, sizeof(uint64_t));
tpnt = ucdata;
tpval = 0;
for(ti=0;ti<tsec_nitems;ti++)
	{
	int skiplen;
	uint64_t val = fstGetVarint64(tpnt, &skiplen);
	tpval = xc->rvat_time_table[ti] = tpval + val;
	tpnt += skiplen;	
	}

free(ucdata);
}

fseeko(xc->f, blkpos+32, SEEK_SET);

frame_uclen = fstReaderVarint64(xc->f);
frame_clen = fstReaderVarint64(xc->f);
xc->rvat_frame_maxhandle = fstReaderVarint64(xc->f);
xc->rvat_frame_data = malloc(frame_uclen);

if(frame_uclen == frame_clen)
	{
	fstFread(xc->rvat_frame_data, frame_uclen, 1, xc->f);
	}
	else
	{
	unsigned char *mc = malloc(frame_clen);
	int rc;

	unsigned long destlen = frame_uclen;
	unsigned long sourcelen = frame_clen;

	fstFread(mc, sourcelen, 1, xc->f);
	rc = uncompress(xc->rvat_frame_data, &destlen, mc, sourcelen);
	if(rc != Z_OK)
		{
		printf("decompress rc: %d\n", rc);
		exit(255);
		}
	free(mc);
	}

xc->rvat_vc_maxhandle = fstReaderVarint64(xc->f);
xc->rvat_vc_start = ftello(xc->f);	/* points to '!' character */

#ifdef FST_DEBUG
printf("\tframe_uclen: %d, frame_clen: %d, frame_maxhandle: %d\n",
	(int)frame_uclen, (int)frame_clen, (int)xc->rvat_frame_maxhandle);
printf("\tvc_maxhandle: %d\n", (int)xc->rvat_vc_maxhandle);
#endif

indx_pntr = blkpos + seclen - 24 -tsec_clen -8;
fseeko(xc->f, indx_pntr, SEEK_SET);
chain_clen = fstReaderUint64(xc->f);
indx_pos = indx_pntr - chain_clen;
#ifdef FST_DEBUG
printf("\tindx_pos: %d (%d bytes)\n", (int)indx_pos, (int)chain_clen);
#endif
chain_cmem = malloc(chain_clen);
fseeko(xc->f, indx_pos, SEEK_SET);
fstFread(chain_cmem, chain_clen, 1, xc->f);
	
xc->rvat_chain_table = calloc((xc->rvat_vc_maxhandle+1), sizeof(off_t));
xc->rvat_chain_table_lengths = calloc((xc->rvat_vc_maxhandle+1), sizeof(uint32_t));

pnt = chain_cmem;
idx = 0;
pval = 0;
do
	{
	int skiplen;
	uint64_t val = fstGetVarint32(pnt, &skiplen);

        if(!val)
		{
		pnt += skiplen;
		val = fstGetVarint32(pnt, &skiplen);
		xc->rvat_chain_table[idx] = 0;
                xc->rvat_chain_table_lengths[idx] = -val;
                idx++;
                }
	else		
	if(val&1)
		{
		pval = xc->rvat_chain_table[idx] = pval + (val >> 1);
		if(idx) { xc->rvat_chain_table_lengths[pidx] = pval - xc->rvat_chain_table[pidx]; }
		pidx = idx++;
		}
		else
		{
		int loopcnt = val >> 1;
		for(i=0;i<loopcnt;i++)
			{
			xc->rvat_chain_table[idx++] = 0;
			}
		}
		
	pnt += skiplen;
	} while (pnt != (chain_cmem + chain_clen));

free(chain_cmem); 
xc->rvat_chain_table[idx] = indx_pos - xc->rvat_vc_start;
xc->rvat_chain_table_lengths[pidx] = xc->rvat_chain_table[idx] - xc->rvat_chain_table[pidx];

for(i=0;i<idx;i++)
	{
        int32_t v32 = xc->rvat_chain_table_lengths[i];
	if((v32 < 0) && (!xc->rvat_chain_table[i]))
        	{
                v32 = -v32;
		v32--;
		if(((uint32_t)v32) < i) /* sanity check */
			{
	                xc->rvat_chain_table[i] = xc->rvat_chain_table[v32];
	                xc->rvat_chain_table_lengths[i] = xc->rvat_chain_table_lengths[v32];
			}
                }
	}

#ifdef FST_DEBUG
printf("\tdecompressed chain idx len: %"PRIu32"\n", idx);
#endif

xc->rvat_data_valid = 1;

/* all data at this point is loaded or resident in fst cache, process and return appropriate value */ 
process_value:
if(facidx > xc->rvat_vc_maxhandle)
	{
	return(NULL);
	}

facidx--; /* scale down for array which starts at zero */


if(((tim == xc->rvat_beg_tim)&&(!xc->rvat_chain_table[facidx])) || (!xc->rvat_chain_table[facidx]))
	{
	return(fstExtractRvatDataFromFrame(xc, facidx, buf));
	}

if(facidx != xc->rvat_chain_facidx)
	{
	if(xc->rvat_chain_mem)
		{
		free(xc->rvat_chain_mem);
		xc->rvat_chain_mem = NULL;

		xc->rvat_chain_pos_valid = 0;
		}
	}

if(!xc->rvat_chain_mem)
	{
	uint32_t skiplen;
	fseeko(xc->f, xc->rvat_vc_start + xc->rvat_chain_table[facidx], SEEK_SET);
	xc->rvat_chain_len = fstReaderVarint32WithSkip(xc->f, &skiplen);
	if(xc->rvat_chain_len)
		{
		unsigned char *mu = malloc(xc->rvat_chain_len);
		unsigned char *mc = malloc(xc->rvat_chain_table_lengths[facidx]);
		unsigned long destlen = xc->rvat_chain_len;
		unsigned long sourcelen = xc->rvat_chain_table_lengths[facidx];
		int rc;
		
		fstFread(mc, xc->rvat_chain_table_lengths[facidx], 1, xc->f);
		rc = uncompress(mu, &destlen, mc, sourcelen);
		free(mc);
	
		if(rc != Z_OK)
			{
			printf("\tclen: %d (rc=%d)\n", (int)xc->rvat_chain_len, rc);
			exit(255);
			}
	
		/* data to process is for(j=0;j<destlen;j++) in mu[j] */
		xc->rvat_chain_mem = mu;
		}
		else
		{
		int destlen = xc->rvat_chain_table_lengths[facidx] - skiplen;
		unsigned char *mu = malloc(xc->rvat_chain_len = destlen);
		fstFread(mu, destlen, 1, xc->f);
		/* data to process is for(j=0;j<destlen;j++) in mu[j] */
		xc->rvat_chain_mem = mu;
		}

	xc->rvat_chain_facidx = facidx;
	}	

/* process value chain here */

{
uint32_t tidx = 0, ptidx = 0;
uint32_t tdelta;
int skiplen;
int iprev = xc->rvat_chain_len;
uint32_t pvli = 0;
int pskip = 0;

if((xc->rvat_chain_pos_valid)&&(tim >= xc->rvat_chain_pos_time))
	{
	i = xc->rvat_chain_pos_idx;
	tidx = xc->rvat_chain_pos_tidx;
	}
	else
	{
	i = 0;
	tidx = 0;
	xc->rvat_chain_pos_time = xc->rvat_beg_tim;
	}

if(xc->signal_lens[facidx] == 1)
	{
	while(i<xc->rvat_chain_len)
		{
	        uint32_t vli = fstGetVarint32(xc->rvat_chain_mem + i, &skiplen);
	        uint32_t shcnt = 2 << (vli & 1);
	        tdelta = vli >> shcnt;

		if(xc->rvat_time_table[tidx + tdelta] <= tim)
			{
			iprev = i;
			pvli = vli;
			ptidx = tidx;
			/* pskip = skiplen; */ /* scan-build */

			tidx += tdelta;
			i+=skiplen;
			}
			else
			{
			break;
			}
		}
	if(iprev != xc->rvat_chain_len)
		{
		xc->rvat_chain_pos_tidx = ptidx;
		xc->rvat_chain_pos_idx = iprev;
		xc->rvat_chain_pos_time = tim;
		xc->rvat_chain_pos_valid = 1;

		if(!(pvli & 1))
			{
			buf[0] = ((pvli >> 1) & 1) | '0'; 
			}
			else
			{
			buf[0] = FST_RCV_STR[((pvli >> 1) & 7)];
			}
		buf[1] = 0;
		return(buf);
		}
		else
		{
		return(fstExtractRvatDataFromFrame(xc, facidx, buf));
		}
        }
        else
        {
	while(i<xc->rvat_chain_len)
		{
	        uint32_t vli = fstGetVarint32(xc->rvat_chain_mem + i, &skiplen);
	        tdelta = vli >> 1;

		if(xc->rvat_time_table[tidx + tdelta] <= tim)
			{
			iprev = i;
			pvli = vli;
			ptidx = tidx;
			pskip = skiplen;

			tidx += tdelta;
			i+=skiplen;

			if(!(pvli & 1))
				{
				i+=((xc->signal_lens[facidx]+7)/8);
				}
				else
				{
				i+=xc->signal_lens[facidx];
				}
			}
			else
			{
			break;
			}
		}

	if(iprev != xc->rvat_chain_len)
		{
		unsigned char *vdata = xc->rvat_chain_mem + iprev + pskip;

		xc->rvat_chain_pos_tidx = ptidx;
		xc->rvat_chain_pos_idx = iprev;
		xc->rvat_chain_pos_time = tim;
		xc->rvat_chain_pos_valid = 1;

		if(xc->signal_typs[facidx] != FST_VT_VCD_REAL)
			{
			if(!(pvli & 1))
				{
				int byte = 0;
				int bit;
				int j;

				for(j=0;j<xc->signal_lens[facidx];j++)
					{
					unsigned char ch;
					byte = j/8;
					bit = 7 - (j & 7);
					ch = ((vdata[byte] >> bit) & 1) | '0';
					buf[j] = ch;
					}
				buf[j] = 0;

				return(buf);
				}
				else
				{
				memcpy(buf, vdata, xc->signal_lens[facidx]);
				buf[xc->signal_lens[facidx]] = 0;
				return(buf);
				}
			}
			else
			{
			double d;
			unsigned char *clone_d = (unsigned char *)&d;
			unsigned char bufd[8];
			unsigned char *srcdata;

			if(!(pvli & 1))	/* very rare case, but possible */
				{
				int bit;
				int j;

				for(j=0;j<8;j++)
					{
					unsigned char ch;
					bit = 7 - (j & 7);
					ch = ((vdata[0] >> bit) & 1) | '0';
					bufd[j] = ch;
					}
	
				srcdata = bufd;
				}
				else
				{
				srcdata = vdata;
				}

			if(xc->double_endian_match)
				{
				memcpy(clone_d, srcdata, 8);
				}
				else
				{
				int j;

				for(j=0;j<8;j++)
					{
					clone_d[j] = srcdata[7-j];
					}
				}

			sprintf(buf, "r%.16g", d);
			return(buf);
			}
		}
		else
		{
		return(fstExtractRvatDataFromFrame(xc, facidx, buf));
		}
        }               
}

/* return(NULL); */
}



/**********************************************************************/
#ifndef FST_DYNAMIC_ALIAS_DISABLE
#ifndef _WAVE_HAVE_JUDY

/***********************/
/***                 ***/
/***  jenkins hash   ***/
/***                 ***/
/***********************/

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
j_hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (uint8_t **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

static uint32_t j_hash(uint8_t *k, uint32_t length, uint32_t initval)
{
   uint32_t a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
      b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
      c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((uint32_t)k[10]<<24);
   case 10: c+=((uint32_t)k[9]<<16);
   case 9 : c+=((uint32_t)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((uint32_t)k[7]<<24);
   case 7 : b+=((uint32_t)k[6]<<16);
   case 6 : b+=((uint32_t)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((uint32_t)k[3]<<24);
   case 3 : a+=((uint32_t)k[2]<<16);
   case 2 : a+=((uint32_t)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return(c);
}

/********************************************************************/

/***************************/
/***                     ***/
/***  judy HS emulation  ***/
/***                     ***/
/***************************/

struct collchain_t
{
struct collchain_t *next;
void *payload;
uint32_t fullhash, length;
unsigned char mem[1];
};


void **JenkinsIns(void *base_i, unsigned char *mem, uint32_t length, uint32_t hashmask)
{
struct collchain_t ***base = (struct collchain_t ***)base_i;
uint32_t hf, h;
struct collchain_t **ar;
struct collchain_t *chain, *pchain;

if(!*base)
	{
	*base = calloc(1, (hashmask + 1) * sizeof(void *));
	}
ar = *base;

h = (hf = j_hash(mem, length, length)) & hashmask;
pchain = chain = ar[h];
while(chain)
	{
	if((chain->fullhash == hf) && (chain->length == length) && !memcmp(chain->mem, mem, length))
		{
		if(pchain != chain) /* move hit to front */
			{
			pchain->next = chain->next;
			chain->next = ar[h];
			ar[h] = chain;
			}
		return(&(chain->payload));
		}

	pchain = chain;
	chain = chain->next;
	}

chain = calloc(1, sizeof(struct collchain_t) + length - 1);
memcpy(chain->mem, mem, length);
chain->fullhash = hf;	
chain->length = length;	
chain->next = ar[h];
ar[h] = chain;
return(&(chain->payload));
}


void JenkinsFree(void *base_i, uint32_t hashmask)
{
struct collchain_t ***base = (struct collchain_t ***)base_i;
uint32_t h;
struct collchain_t **ar;
struct collchain_t *chain, *chain_next;

if(base && *base)
	{
	ar = *base;
	for(h=0;h<=hashmask;h++)
		{
		chain = ar[h];
		while(chain)
			{
			chain_next = chain->next;
			free(chain);
			chain = chain_next;
			}
		}

	free(*base);
	*base = NULL;
	}
}

#endif
#endif

/**********************************************************************/

/************************/
/***                  ***/
/*** utility function ***/
/***                  ***/
/************************/

int fstUtilityBinToEsc(unsigned char *d, unsigned char *s, int len)
{
unsigned char *src = s;
unsigned char *dst = d;
unsigned char val;
int i;

for(i=0;i<len;i++)
	{
	switch(src[i])
		{
		case '\a':	*(dst++) = '\\'; *(dst++) = 'a'; break;
		case '\b':	*(dst++) = '\\'; *(dst++) = 'b'; break;
		case '\f':	*(dst++) = '\\'; *(dst++) = 'f'; break;
		case '\n':	*(dst++) = '\\'; *(dst++) = 'n'; break;
		case '\r':	*(dst++) = '\\'; *(dst++) = 'r'; break;
		case '\t':	*(dst++) = '\\'; *(dst++) = 't'; break;
		case '\v':	*(dst++) = '\\'; *(dst++) = 'v'; break;
		case '\'':	*(dst++) = '\\'; *(dst++) = '\''; break;
		case '\"':	*(dst++) = '\\'; *(dst++) = '\"'; break;
		case '\\':	*(dst++) = '\\'; *(dst++) = '\\'; break;
		case '\?':	*(dst++) = '\\'; *(dst++) = '\?'; break;
		default:	if((src[i] > ' ') && (src[i] <= '~')) /* no white spaces in output */
					{
					*(dst++) = src[i];
					}
					else
					{
					val = src[i];
					*(dst++) = '\\';
					*(dst++) = (val/64) + '0'; val = val & 63;
					*(dst++) = (val/8)  + '0'; val = val & 7;
					*(dst++) = (val) + '0';
					}
				break;
		}
	}

return(dst - d);
}


/*
 * this overwrites the original string if the destination pointer is NULL
 */
int fstUtilityEscToBin(unsigned char *d, unsigned char *s, int len)
{
unsigned char *src = s;
unsigned char *dst = (!d) ? s : (s = d);
unsigned char val[3];
int i;

for(i=0;i<len;i++)
	{
	if(src[i] != '\\')
		{
		*(dst++) = src[i];
		}
		else
		{
		switch(src[++i])
			{
			case 'a':	*(dst++) = '\a'; break;
			case 'b':	*(dst++) = '\b'; break;
			case 'f':	*(dst++) = '\f'; break;
			case 'n':	*(dst++) = '\n'; break;
			case 'r':	*(dst++) = '\r'; break;
			case 't':	*(dst++) = '\t'; break;
			case 'v':	*(dst++) = '\v'; break;
			case '\'':	*(dst++) = '\''; break;
			case '\"':	*(dst++) = '\"'; break;
			case '\\':	*(dst++) = '\\'; break;
			case '\?':	*(dst++) = '\?'; break;

			case 'x':	val[0] = toupper(src[++i]);
					val[1] = toupper(src[++i]);
					val[0] = ((val[0]>='A')&&(val[0]<='F')) ? (val[0] - 'A' + 10) : (val[0] - '0');
					val[1] = ((val[1]>='A')&&(val[1]<='F')) ? (val[1] - 'A' + 10) : (val[1] - '0');
					*(dst++) = val[0] * 16 + val[1];
					break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':	val[0] = src[  i] - '0';
					val[1] = src[++i] - '0';
					val[2] = src[++i] - '0';
					*(dst++) = val[0] * 64 + val[1] * 8 + val[2];
					break;

			default:	*(dst++) = src[i]; break;
			}
		}
	}

return(dst - s);
}
