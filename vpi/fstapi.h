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

#ifndef FST_API_H
#define FST_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>

#define FST_RDLOAD "FSTLOAD | "

typedef uint32_t fstHandle;

enum fstBlockType {
    FST_BL_HDR		       = 0,
    FST_BL_VCDATA              = 1,
    FST_BL_BLACKOUT	       = 2,
    FST_BL_GEOM                = 3,
    FST_BL_HIER                = 4,
    FST_BL_VCDATA_DYN_ALIAS    = 5,

    FST_BL_ZWRAPPER	       = 254,   /* indicates that whole trace is gz wrapped */
    FST_BL_SKIP		       = 255	/* used while block is being written */
};

enum fstScopeType {
    FST_ST_VCD_MIN             = 0,
    FST_ST_VCD_MODULE          = 0,
    FST_ST_VCD_TASK            = 1,
    FST_ST_VCD_FUNCTION        = 2,
    FST_ST_VCD_BEGIN           = 3,
    FST_ST_VCD_FORK            = 4,
    FST_ST_VCD_MAX             = 4,

    FST_ST_MAX                 = 4,

    FST_ST_VCD_SCOPE           = 254,
    FST_ST_VCD_UPSCOPE         = 255
};

enum fstVarType {
    FST_VT_VCD_MIN             = 0,	/* start of VCD datatypes */
    FST_VT_VCD_EVENT           = 0,
    FST_VT_VCD_INTEGER         = 1,
    FST_VT_VCD_PARAMETER       = 2,
    FST_VT_VCD_REAL            = 3,
    FST_VT_VCD_REAL_PARAMETER  = 4,
    FST_VT_VCD_REG             = 5,
    FST_VT_VCD_SUPPLY0         = 6,
    FST_VT_VCD_SUPPLY1         = 7,
    FST_VT_VCD_TIME            = 8,  
    FST_VT_VCD_TRI             = 9,
    FST_VT_VCD_TRIAND          = 10,
    FST_VT_VCD_TRIOR           = 11,
    FST_VT_VCD_TRIREG          = 12,
    FST_VT_VCD_TRI0            = 13,
    FST_VT_VCD_TRI1            = 14,
    FST_VT_VCD_WAND            = 15,
    FST_VT_VCD_WIRE            = 16,
    FST_VT_VCD_WOR             = 17,
    FST_VT_VCD_PORT            = 18,
    FST_VT_VCD_ARRAY           = 19,	/* used to define the rownum (index) port on the array */
    FST_VT_VCD_REALTIME        = 20,
    FST_VT_GEN_STRING	       = 21,	/* generic string type   (max len is defined dynamically via fstWriterEmitVariableLengthValueChange) */

    FST_VT_VCD_MAX             = 21	/* end of VCD datatypes */
};

enum fstVarDir {
    FST_VD_IMPLICIT    = 0,
    FST_VD_INPUT       = 1,
    FST_VD_OUTPUT      = 2,
    FST_VD_INOUT       = 3,

    FST_VD_MAX         = 3
};

enum fstHierType {
    FST_HT_SCOPE       = 0,
    FST_HT_UPSCOPE     = 1,
    FST_HT_VAR         = 2,

    FST_HT_MAX         = 2
};

struct fstHier
{
unsigned char htyp;

union {
	/* if htyp == FST_HT_SCOPE */
	struct fstHierScope {
		unsigned char typ; /* FST_ST_VCD_MODULE ... FST_ST_VCD_FORK */
		const char *name;
		const char *component;
		uint32_t name_length;		/* strlen(u.scope.name) */
		uint32_t component_length;	/* strlen(u.scope.component) */
		} scope;

	/* if htyp == FST_HT_VAR */
	struct fstHierVar {
		unsigned char typ; /* FST_VT_VCD_EVENT ... FST_VT_VCD_REALTIME */
		unsigned char direction; /* FST_VD_IMPLICIT ... FST_VD_INOUT */
		const char *name;
		uint32_t length;
		fstHandle handle;
		uint32_t name_length; /* strlen(u.var.name) */
		unsigned is_alias : 1;
		} var;
	} u;
};


/*
 * writer functions
 */
fstHandle fstWriterCreateVar(void *ctx, enum fstVarType vt, enum fstVarDir vd,
        uint32_t len, const char *nam, fstHandle aliasHandle);

void fstWriterSetPackType(void *ctx, int typ); 		/* type = 0 (libz), 1 (fastlz) */
void fstWriterSetRepackOnClose(void *ctx, int enable); 	/* type = 0 (none), 1 (libz) */
void fstWriterSetParallelMode(void *ctx, int enable);
void fstWriterSetDumpSizeLimit(void *ctx, uint64_t numbytes);
int fstWriterGetDumpSizeLimitReached(void *ctx);

void *fstWriterCreate(const char *nam, int use_compressed_hier);
void fstWriterClose(void *ctx);
void fstWriterSetDate(void *ctx, const char *dat);
void fstWriterSetVersion(void *ctx, const char *vers);
void fstWriterSetTimescale(void *ctx, int ts);
void fstWriterSetTimescaleFromString(void *ctx, const char *s);
void fstWriterSetTimezero(void *ctx, int64_t tim);
void fstWriterSetScope(void *ctx, enum fstScopeType scopetype,
                const char *scopename, const char *scopecomp);
void fstWriterSetUpscope(void *ctx);
void fstWriterEmitValueChange(void *ctx, fstHandle handle, const void *val);
void fstWriterEmitVariableLengthValueChange(void *ctx, fstHandle handle, const void *val, uint32_t len);
void fstWriterEmitDumpActive(void *ctx, int enable);
void fstWriterEmitTimeChange(void *ctx, uint64_t tim);
void fstWriterFlushContext(void *ctx);

/*
 * reader functions
 */
void *fstReaderOpen(const char *nam);
void *fstReaderOpenForUtilitiesOnly(void);
void fstReaderClose(void *ctx);

int fstReaderProcessHier(void *ctx, FILE *vcdhandle);
int fstReaderIterateHierRewind(void *ctx);
struct fstHier *fstReaderIterateHier(void *ctx);

void fstReaderResetScope(void *ctx);
const char *fstReaderPopScope(void *ctx);
const char *fstReaderPushScope(void *ctx, const char *nam, void *user_info);
const char *fstReaderGetCurrentFlatScope(void *ctx);
void *fstReaderGetCurrentScopeUserInfo(void *ctx);
int fstReaderGetCurrentScopeLen(void *ctx);

signed char fstReaderGetTimescale(void *ctx);
int64_t fstReaderGetTimezero(void *ctx);
uint64_t fstReaderGetStartTime(void *ctx);
uint64_t fstReaderGetEndTime(void *ctx);
uint64_t fstReaderGetMemoryUsedByWriter(void *ctx);
uint64_t fstReaderGetScopeCount(void *ctx);
uint64_t fstReaderGetVarCount(void *ctx);
fstHandle fstReaderGetMaxHandle(void *ctx);
uint64_t fstReaderGetAliasCount(void *ctx);
uint64_t fstReaderGetValueChangeSectionCount(void *ctx);
int fstReaderGetDoubleEndianMatchState(void *ctx);
const char *fstReaderGetVersionString(void *ctx);
const char *fstReaderGetDateString(void *ctx);

void fstReaderSetLimitTimeRange(void *ctx, uint64_t start_time, uint64_t end_time);
void fstReaderSetUnlimitedTimeRange(void *ctx);

uint32_t fstReaderGetNumberDumpActivityChanges(void *ctx);
uint64_t fstReaderGetDumpActivityChangeTime(void *ctx, uint32_t idx);
unsigned char fstReaderGetDumpActivityChangeValue(void *ctx, uint32_t idx);

int fstReaderGetFacProcessMask(void *ctx, fstHandle facidx);
void fstReaderSetFacProcessMask(void *ctx, fstHandle facidx);
void fstReaderClrFacProcessMask(void *ctx, fstHandle facidx);
void fstReaderSetFacProcessMaskAll(void *ctx);
void fstReaderClrFacProcessMaskAll(void *ctx);

void fstReaderIterBlocksSetNativeDoublesOnCallback(void *ctx, int enable);

int fstReaderIterBlocks(void *ctx,
        void (*value_change_callback)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value),
        void *user_callback_data_pointer, FILE *vcdhandle);

int fstReaderIterBlocks2(void *ctx,
        void (*value_change_callback)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value),
        void (*value_change_callback_varlen)(void *user_callback_data_pointer, uint64_t time, fstHandle facidx, const unsigned char *value, uint32_t len),
        void *user_callback_data_pointer, FILE *vcdhandle);

char *fstReaderGetValueFromHandleAtTime(void *ctx, uint64_t tim, fstHandle facidx, char *buf);

/*
 * utility functions
 */
int fstUtilityBinToEsc(unsigned char *d, unsigned char *s, int len);
int fstUtilityEscToBin(unsigned char *d, unsigned char *s, int len);

#ifdef __cplusplus
}
#endif

#endif
