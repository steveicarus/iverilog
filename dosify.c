/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: dosify.c,v 1.5 2003/07/15 16:17:47 steve Exp $"
#endif

/*
 * This is a simple program to make a dosified copy of the
 * original. That is, it converts unix style line ends to DOS
 * style. This is useful for installing text files.
 *
 * The exact substitution is to replace \n with \r\n. If the line
 * already ends with \r\n then it is not changed to \r\r\n.
 */

# include  <stdio.h>

int main(int argc, char*argv[])
{
      FILE*ifile;
      FILE*ofile;
      int ch, pr;

      if (argc != 3) {
	    fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
	    return 1;
      }

      ifile = fopen(argv[1], "rb");
      if (ifile == 0) {
	    fprintf(stderr, "Unable to open %s for input.\n", argv[1]);
	    return 2;
      }

      ofile = fopen(argv[2], "wb");
      if (ofile == 0) {
	    fprintf(stderr, "Unable to open %s for output.\n", argv[2]);
	    return 2;
      }

      pr = 0;
      while ((ch = fgetc(ifile)) != EOF) {

	    if ((ch == '\n') && (pr != '\r'))
		  fputc('\r', ofile);

	    fputc(ch, ofile);
	    pr = ch;
      }

      return 0;
}

/*
 * $Log: dosify.c,v $
 * Revision 1.5  2003/07/15 16:17:47  steve
 *  Fix spelling of ifdef.
 *
 * Revision 1.4  2003/07/15 03:49:22  steve
 *  Spelling fixes.
 *
 * Revision 1.3  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 * Revision 1.1  2001/08/03 17:06:47  steve
 *  Add install of examples for Windows.
 *
 */

