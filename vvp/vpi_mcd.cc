/*
 * Copyright (c) 2000-2012 Stephen G. Tell <steve@telltronics.org>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "vpi_priv.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cassert>
# include  <cstdarg>
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>

extern FILE* vpi_trace;

/*
 * This table keeps track of the MCD files. Note that there may be
 * only 31 such files, and mcd bit0 (32'h00_00_00_01) is the special
 * standard output file, which may be replicated to a logfile
 * depending on flags to the command line.
 */

/*
 * MCD/FD manipulation macros
 */
#define IS_MCD(mcd)	!((mcd)>>31&1)
#define FD_IDX(fd)	((fd)&~(1U<<31))
#define FD_INCR		32

typedef struct mcd_entry {
	FILE *fp;
	char *filename;
} mcd_entry_s;
static mcd_entry_s mcd_table[31];
static mcd_entry_s *fd_table = NULL;
static unsigned fd_table_len = 0;

static FILE* logfile;

/* Initialize mcd portion of vpi.  Must be called before
 * any vpi_mcd routines can be used.
 */
void vpip_mcd_init(FILE *log)
{
      fd_table_len = FD_INCR;
      fd_table = (mcd_entry_s *) malloc(fd_table_len*sizeof(mcd_entry_s));
      for (unsigned idx = 0; idx < fd_table_len; idx += 1) {
	    fd_table[idx].fp = NULL;
	    fd_table[idx].filename = NULL;
      }

      mcd_table[0].fp = stdout;
      mcd_table[0].filename = strdup("stdout");

      fd_table[0].fp = stdin;
      fd_table[0].filename = strdup("stdin");
      fd_table[1].fp = stdout;
      fd_table[1].filename = strdup("stdout");
      fd_table[2].fp = stderr;
      fd_table[2].filename = strdup("stderr");

      logfile = log;
}

#ifdef CHECK_WITH_VALGRIND
void vpi_mcd_delete(void)
{
      free(mcd_table[0].filename);
      mcd_table[0].filename = NULL;
      mcd_table[0].fp = NULL;

      free(fd_table[0].filename);
      fd_table[0].filename = NULL;
      fd_table[0].fp = NULL;

      free(fd_table[1].filename);
      fd_table[1].filename = NULL;
      fd_table[1].fp = NULL;

      free(fd_table[2].filename);
      fd_table[2].filename = NULL;
      fd_table[2].fp = NULL;

      free(fd_table);
      fd_table = NULL;
      fd_table_len = 0;
}
#endif

/*
 * close one or more channels.  we silently refuse to close the preopened ones.
 */
extern "C" PLI_UINT32 vpi_mcd_close(PLI_UINT32 mcd)
{
	int rc = 0;

	if (IS_MCD(mcd)) {
		for(int i = 1; i < 31; i++) {
			if(((mcd>>i) & 1) && mcd_table[i].fp) {
				if(fclose(mcd_table[i].fp)) rc |= 1<<i;
				free(mcd_table[i].filename);
				mcd_table[i].fp = NULL;
				mcd_table[i].filename = NULL;
			} else {
				rc |= 1<<i;
			}
		}
	} else {
		unsigned idx = FD_IDX(mcd);
		if (idx > 2 && idx < fd_table_len && fd_table[idx].fp) {
			rc = fclose(fd_table[idx].fp);
			free(fd_table[idx].filename);
			fd_table[idx].fp = NULL;
			fd_table[idx].filename = NULL;
		}
	}
	return rc;
}

extern "C" char *vpi_mcd_name(PLI_UINT32 mcd)
{
	if (IS_MCD(mcd)) {
		for(int i = 0; i < 31; i++) {
			if((mcd>>i) & 1)
				return mcd_table[i].filename;
		}
	} else {
		unsigned idx = FD_IDX(mcd);
		if (idx < fd_table_len)
			return fd_table[idx].filename;
	}
	return NULL;
}

extern "C" PLI_UINT32 vpi_mcd_open(char *name)
{
	int i;

	for(i = 0; i < 31; i++) {
		if(mcd_table[i].filename == NULL)
			goto got_entry;
	}
	return 0;  /* too many open mcd's */

got_entry:
	mcd_table[i].fp = fopen(name, "w");
	if(mcd_table[i].fp == NULL)
		return 0;
	mcd_table[i].filename = strdup(name);

	if (vpi_trace) {
	      fprintf(vpi_trace, "vpi_mcd_open(%s) --> 0x%08x\n",
		      name, 1 << i);
	}

	return 1<<i;
}

extern "C" PLI_INT32
vpi_mcd_vprintf(PLI_UINT32 mcd, const char*fmt, va_list ap)
{
      char buffer[4096];
      char *buf_ptr = buffer;
      int rc = 0;
      bool need_free = false;
      va_list saved_ap;

      if (!IS_MCD(mcd)) return 0;

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_mcd_vprintf(0x%08x, %s, ...);\n",
		    (unsigned int)mcd, fmt);
      }

      va_copy(saved_ap, ap);
#ifdef __MINGW32__
	/*
	 * The MinGW runtime (version 3.14) fixes some things, but breaks
	 * %f for us, so we have to us the underlying version.
	 */
      rc = _vsnprintf(buffer, sizeof buffer, fmt, ap);
	/*
	 * Windows returns -1 to indicate the result was truncated (thanks for
	 * following the standard!). Since we don't know how big to make the
	 * buffer just keep doubling it until it works.
	 */
      if (rc == -1) {
	    size_t buf_size = sizeof buffer;
	    buf_ptr = NULL;
	    need_free = true;
	    while (rc == -1) {
		  va_list tmp_ap;
		  va_copy(tmp_ap, saved_ap);
		  buf_size *= 2;
		  buf_ptr = (char *)realloc(buf_ptr, buf_size);
		  rc = vsnprintf(buf_ptr, buf_size, fmt, tmp_ap);
		  va_end(tmp_ap);
	    }
      }
#else
      rc = vsnprintf(buffer, sizeof buffer, fmt, ap);
      assert(rc >= 0);
	/*
	 * If rc is greater than sizeof buffer then the result was truncated
	 * so the print needs to be redone with a larger buffer (very rare).
	 */
      if ((unsigned) rc >= sizeof buffer) {
	    buf_ptr = (char *)malloc(rc + 1);
	    need_free = true;
	    rc = vsnprintf(buf_ptr, rc+1, fmt, saved_ap);
      }
#endif
      va_end(saved_ap);

      for(int i = 0; i < 31; i++) {
	    if((mcd>>i) & 1) {
		  if(mcd_table[i].fp) {
			  // echo to logfile
			if (i == 0 && logfile)
			      fputs(buf_ptr, logfile);
			fputs(buf_ptr, mcd_table[i].fp);
		  } else {
			rc = EOF;
		  }
	    }
      }
      if (need_free) free(buf_ptr);

      return rc;
}

extern "C" PLI_INT32 vpi_mcd_printf(PLI_UINT32 mcd, const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      int r = vpi_mcd_vprintf(mcd,fmt,ap);
      va_end(ap);
      return r;
}

extern "C" PLI_INT32 vpi_mcd_flush(PLI_UINT32 mcd)
{
	int rc = 0;

	if (IS_MCD(mcd)) {
		for(int i = 0; i < 31; i++) {
			if((mcd>>i) & 1) {
				if (i == 0 && logfile) fflush(logfile);
				if (fflush(mcd_table[i].fp)) rc |= 1<<i;
			}
		}
	} else {
		unsigned idx = FD_IDX(mcd);
		if (idx < fd_table_len) rc = fflush(fd_table[idx].fp);
	}
	return rc;
}

/*
 * MCD/FD Extensions
 */

/*
 * The vpi_fopen function opens a file with the given path, and
 * returns a file descriptor that includes bit 31 set. This is to
 * differentiate the fd from a mcd descriptor. Note that these
 * descriptors are distinct from the mcd descriptors, so uses a
 * different fd table.
 */
extern "C" PLI_INT32 vpi_fopen(const char*name, const char*mode)
{
      unsigned i;
      for (i = 0; i < fd_table_len; i += 1) {
	    if (fd_table[i].filename == NULL) goto got_entry;
      }
	/* We need to allocate more table entries, but to keep things */
	/* sane we'll hard limit this to 1024 file descriptors total. */
      if (fd_table_len >= 1024) {
	    vpi_printf("WARNING: Icarus only supports 1024 open files!\n");
	    return 0;
      }
      fd_table_len += FD_INCR;
      fd_table = (mcd_entry_s *) realloc(fd_table,
                                         fd_table_len*sizeof(mcd_entry_s));
      for (unsigned idx = i; idx < fd_table_len; idx += 1) {
	    fd_table[idx].fp = NULL;
	    fd_table[idx].filename = NULL;
      }

got_entry:
      fd_table[i].fp = fopen(name, mode);
      if (fd_table[i].fp == NULL) return 0;
      fd_table[i].filename = strdup(name);
      return ((1U<<31)|i);
}

extern "C" FILE *vpi_get_file(PLI_INT32 fd)
{
	// Only deal with FD's
      if (IS_MCD(fd)) return NULL;

	// Only know about fd_table_len indices
      if (FD_IDX(fd) >= fd_table_len) return NULL;

      return fd_table[FD_IDX(fd)].fp;
}
