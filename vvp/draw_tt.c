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
#ident "$Id: draw_tt.c,v 1.5 2001/04/15 16:37:48 steve Exp $"
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

static void draw_BUF(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_BUF[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i0 == 1)
				    val = 1;
			      else if (i0 == 0)
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

static void draw_NOR(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_NOR[64] = {");

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
				    val = 0;
			      else if ((i0 == 0) && (i1 == 0) && 
				       (i2 == 0) && (i3 == 0))
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

static void draw_NOT(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_NOT[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i0 == 1)
				    val = 0;
			      else if (i0 == 0)
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

static void draw_XNOR(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_XNOR[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;

			      if ((i0 > 1) || (i1 > 1)
				  || (i2 > 1) || (i3 > 1))
				    val = 2;
			      else
				    val = (i0 + i1 + i2 + i3) % 2 ^ 1;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_XOR(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_XOR[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;

			      if ((i0 > 1) || (i1 > 1)
				  || (i2 > 1) || (i3 > 1))
				    val = 2;
			      else
				    val = (i0 + i1 + i2 + i3) % 2;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

/*
 * The hex_digits table is not a functor truth table per say, but a
 * map of a 4-vbit value to a hex digit. The table handles the display
 * of x, X, z, Z, etc.
 */
static void draw_hex_table()
{
      unsigned idx;

      printf("extern const char hex_digits[256] = {\n");
      for (idx = 0 ;  idx < 256 ;  idx += 1) {
	    unsigned cnt_z = 0, cnt_x = 0;
	    unsigned bv = 0, bdx;

	    for (bdx = 0 ;  bdx < 4 ;  bdx += 1) {
		  switch ((idx >> (bdx * 2)) & 3) {
		      case 0:
			break;
		      case 1:
			bv |= 1<<bdx;
			break;
		      case 2:
			cnt_x += 1;
			break;
		      case 3:
			cnt_z += 1;
			break;
		  }
	    }

	    if (cnt_z == 4)
		  printf(" 'z',");

	    else if (cnt_x == 4)
		  printf(" 'x',");

	    else if ((cnt_z > 0) && (cnt_x == 0))
		  printf(" 'Z',");

	    else if (cnt_x > 0)
		  printf(" 'X',");

	    else
		  printf(" '%c',", "0123456789abcdef"[bv]);

	    if (((idx+1) % 8) == 0)
		  printf("\n");
      }
      printf("};\n");
}

static void draw_oct_table()
{
      unsigned idx;

      printf("extern const char oct_digits[64] = {\n");
      for (idx = 0 ;  idx < 64 ;  idx += 1) {
	    unsigned cnt_z = 0, cnt_x = 0;
	    unsigned bv = 0, bdx;

	    for (bdx = 0 ;  bdx < 3 ;  bdx += 1) {
		  switch ((idx >> (bdx * 2)) & 3) {
		      case 0:
			break;
		      case 1:
			bv |= 1<<bdx;
			break;
		      case 2:
			cnt_x += 1;
			break;
		      case 3:
			cnt_z += 1;
			break;
		  }
	    }

	    if (cnt_z == 3)
		  printf(" 'z',");

	    else if (cnt_x == 3)
		  printf(" 'x',");

	    else if ((cnt_z > 0) && (cnt_x == 0))
		  printf(" 'Z',");

	    else if (cnt_x > 0)
		  printf(" 'X',");

	    else
		  printf(" '%c',", "01234567"[bv]);

	    if (((idx+1) % 8) == 0)
		  printf("\n");
      }
      printf("};\n");
}

main()
{
      printf("# include  \"functor.h\"\n");
      draw_AND();
      draw_BUF();
      draw_NOR();
      draw_NOT();
      draw_OR();
      draw_XNOR();
      draw_XOR();
      draw_hex_table();
      draw_oct_table();
      return 0;
}

/*
 * $Log: draw_tt.c,v $
 * Revision 1.5  2001/04/15 16:37:48  steve
 *  add XOR support.
 *
 * Revision 1.4  2001/04/01 21:31:46  steve
 *  Add the buf functor type.
 *
 * Revision 1.3  2001/03/25 20:45:09  steve
 *  Add vpiOctStrVal access to signals.
 *
 * Revision 1.2  2001/03/25 19:37:26  steve
 *  Calculate NOR and NOT tables, and also the hex_digits table.
 *
 * Revision 1.1  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 */

