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
#if !defined(WINNT)
#ident "$Id: draw_tt.c,v 1.1 2001/03/11 22:42:11 steve Exp $"
#endif

# include  <stdio.h>

static void draw_AND(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_AND[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if ((i0 == 0) || (i1 == 0) || 
				  (i2 == 0) || (i3 == 0))
				    val = 0;
			      else if ((i0 == 1) && (i1 == 1) && 
				       (i2 == 1) && (i3 == 1))
				    val = 1;
			      else
				    val = 2;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_OR(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_OR[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if ((i0 == 1) || (i1 == 1) || 
				  (i2 == 1) || (i3 == 1))
				    val = 1;
			      else if ((i0 == 0) && (i1 == 0) && 
				       (i2 == 0) && (i3 == 0))
				    val = 0;
			      else
				    val = 2;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

main()
{
      printf("# include  \"functor.h\"\n");
      draw_AND();
      draw_OR();
      return 0;
}

/*
 * $Log: draw_tt.c,v $
 * Revision 1.1  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 */

