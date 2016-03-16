/*
 * Copyright (c) 2001-2009 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * This is a simple program to make a dosified copy of the
 * original. That is, it converts Unix style line ends to DOS
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
	    fclose(ifile);
	    return 2;
      }

      pr = 0;
      while ((ch = fgetc(ifile)) != EOF) {

	    if ((ch == '\n') && (pr != '\r'))
		  fputc('\r', ofile);

	    fputc(ch, ofile);
	    pr = ch;
      }

      fclose(ifile);
      fclose(ofile);
      return 0;
}
