/*
 * Copyright (c) 2000 Stephen G. Tell <steve@telltronics.org>
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_mcd.cc,v 1.9 2003/05/15 16:51:09 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <stdarg.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>

/*
 * This table keeps track of the MCD files. Note that there may be
 * only 31 such files, and mcd bit0 (32'h00_00_00_01) is the special
 * standard output file, which may be replicated to a logfile
 * depending on flags to the command line.
 */
struct mcd_entry {
	FILE *fp;
	char *filename;
};

static struct mcd_entry mcd_table[32];
static FILE* logfile;

/* Initialize mcd portion of vpi.  Must be called before
 * any vpi_mcd routines can be used.
 */
void vpi_mcd_init(FILE *log)
{
	mcd_table[0].fp = stdout;
	mcd_table[0].filename = "<stdout>";

	logfile = log;
}

/*
 * close one or more channels.  we silently refuse to close the preopened ones.
 */
extern "C" PLI_UINT32 vpi_mcd_close(unsigned int mcd)
{
	int i;
	int rc;
	rc = 0;
	for(i = 1; i < 31; i++) {
		if( ((mcd>>i) & 1) && mcd_table[i].filename) {
			if(fclose(mcd_table[i].fp) != 0)
				rc |= 1<<i;				
			free(mcd_table[i].filename);
			mcd_table[i].fp = NULL;
			mcd_table[i].filename = NULL;
		} else {
			rc |= 1<<i;
		}
	}
	return rc;
}

extern "C" char *vpi_mcd_name(unsigned int mcd)
{
	int i;
	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1)
			return mcd_table[i].filename;
	}
	return NULL;
}

extern "C" PLI_UINT32 vpi_mcd_open_x(char *name, char *mode)
{
	int i;
	for(i = 0; i < 31; i++) {
		if(mcd_table[i].filename == NULL)
			goto got_entry;
	}
	return 0;  /* too many open mcd's */

got_entry:
	mcd_table[i].fp = fopen(name, mode);
	if(mcd_table[i].fp == NULL)
		return 0;
	mcd_table[i].filename = strdup(name);
	return 1<<i;
}

extern "C" PLI_UINT32 vpi_mcd_open(char *name)
{
	return vpi_mcd_open_x(name,"w");
}

extern "C" PLI_INT32
vpi_mcd_vprintf(unsigned int mcd, const char*fmt, va_list ap)
{
	int i;
	int len;
	int rc;

	rc = len = 0;
	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1) {
			if(mcd_table[i].fp) {
				// echo to logfile
				if (i == 0 && logfile)
				      vfprintf(logfile, fmt, ap);
				len = vfprintf(mcd_table[i].fp, fmt, ap);
			} else
				rc = EOF;
		}
	}

	if(rc)
		return rc;
	else
		return len;
}

extern "C" PLI_INT32 vpi_mcd_printf(unsigned int mcd, const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      int r = vpi_mcd_vprintf(mcd,fmt,ap);
      va_end(ap);
      return r;
}

extern "C" PLI_INT32 vpi_mcd_flush(unsigned int mcd)
{
	int i, rc = 0;
	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1)
		        if (fflush(mcd_table[i].fp)) rc |= 1<<i;
	}
	return rc;
}

int vpi_mcd_fputc(unsigned int mcd, unsigned char x)
{
	int i;

	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1) {
			return fputc(x, mcd_table[i].fp);
		}
	}
	return 0;
}

int vpi_mcd_fgetc(unsigned int mcd)
{
	int i;

	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1) {
			return fgetc(mcd_table[i].fp);
		}
	}
	return 0;
}

/*
 * $Log: vpi_mcd.cc,v $
 * Revision 1.9  2003/05/15 16:51:09  steve
 *  Arrange for mcd id=00_00_00_01 to go to stdout
 *  as well as a user specified log file, set log
 *  file to buffer lines.
 *
 *  Add vpi_flush function, and clear up some cunfused
 *  return codes from other vpi functions.
 *
 *  Adjust $display and vcd/lxt messages to use the
 *  standard output/log file.
 *
 * Revision 1.8  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2002/08/11 23:47:05  steve
 *  Add missing Log and Ident strings.
 *
 */
