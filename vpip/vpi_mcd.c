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
#ident "$Id: vpi_mcd.c,v 1.6 2002/08/12 01:35:05 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <stdarg.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>

struct mcd_entry {
	FILE *fp;
	char *filename;
};

static struct mcd_entry mcd_table[32];

/* Initialize mcd portion of vpi.  Must be called before
 * any vpi_mcd routines can be used.
 */
void vpi_mcd_init(void)
{
	mcd_table[0].fp = stdout;
	mcd_table[0].filename = "<stdout>";
	mcd_table[1].fp = stderr;
	mcd_table[1].filename = "<stderr>";
	mcd_table[2].fp = 0;  /* TODO: initialize this to log file */
	mcd_table[2].filename = "<stdlog>";
}

/*
 * close one or more channels.  we silently refuse to close the preopened ones.
 */
unsigned int vpi_mcd_close(unsigned int mcd)
{
	int i;
	int rc;
	rc = 0;
	for(i = 3; i < 31; i++) {
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

char *vpi_mcd_name(unsigned int mcd)
{
	int i;
	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1)
			return mcd_table[i].filename;
	}
	return NULL;
}

unsigned int vpi_mcd_open_x(char *name, char *mode)
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

unsigned int vpi_mcd_open(char *name)
{
	return vpi_mcd_open_x(name, "w");
}

extern int vpi_mcd_vprintf(unsigned int mcd, const char*fmt, va_list ap)
{
	int i;
	int len;
	int rc;

	rc = len = 0;
	for(i = 0; i < 31; i++) {
		if( (mcd>>i) & 1) {
			if(mcd_table[i].fp)
				len = vfprintf(mcd_table[i].fp, fmt, ap);
			else
				rc = EOF;
		}
	}
	if(rc)
		return rc;
	else
		return len;
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
 * $Log: vpi_mcd.c,v $
 * Revision 1.6  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 */
