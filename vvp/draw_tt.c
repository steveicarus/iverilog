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
#ident "$Id: draw_tt.c,v 1.15 2003/07/30 01:13:29 steve Exp $"
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

static void draw_NAND(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_NAND[64] = {");

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
				    val = 1;
			      else if ((i0 == 1) && (i1 == 1) && 
				       (i2 == 1) && (i3 == 1))
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

static void draw_BUFZ(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_BUFZ[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val = i0;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_BUFIF0(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_BUFIF0[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i2 == 0)
				    val = 3;
			      else if (i0 == 1)
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

static void draw_BUFIF1(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_BUFIF1[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i2 == 1)
				    val = 3;
			      else if (i0 == 1)
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

static void draw_PMOS(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_PMOS[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i2 == 0 || i0 == 3)
				    val = 3;
			      else if (i0 == 1)
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

static void draw_NMOS(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_NMOS[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i2 == 1 || i0 == 3)
				    val = 3;
			      else if (i0 == 1)
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

static void draw_MUXX(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_MUXX[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i3 == 0)
				    val = 3;
			      else if (i3 == 2)
				    val = 2;
			      else if (i3 == 3)
				    val = 2;
			      else if (i2 >= 2) {
				    val = 2;
			      } else if (i2 == 0)
				    val = i0;
			      else
				    val = i1;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_MUXZ(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_MUXZ[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i3 == 0)
				    val = 3;
			      else if (i3 == 2)
				    val = 2;
			      else if (i3 == 3)
				    val = 2;
			      else if (i2 >= 2) {
				    if (i0 == i1)
					  val = i0;
				    else
					  val = 2;
			      } else if (i2 == 0)
				    val = i0;
			      else
				    val = i1;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_EEQ(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_EEQ[64] = {");

      for (i3 = 0 ;  i3 < 4 ;  i3 += 1)
	    for (i2 = 0 ;  i2 < 4 ;  i2 += 1) {
		  printf("\n    ");
		  for (i1 = 0 ;  i1 < 4 ;  i1 += 1) {
			unsigned idx = (i3 << 4) | (i2 << 2) | i1;
			unsigned char byte = 0;

			for (i0 = 0 ; i0 < 4 ;  i0 += 1) {
			      unsigned val;
			      if (i3 != i2)
				    val = 0;
			      else if (i1 != i0)
				    val = 0;
			      else
				    val = 1;

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

static void draw_TRIAND(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_TRIAND[64] = {");

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
			      else if ((i0 == 2) || (i1 == 2) || 
				       (i2 == 2) || (i3 == 2))
				    val = 2;
			      else if ((i0 == 3) && (i1 == 3) && 
				       (i2 == 3) && (i3 == 3))
				    val = 3;
			      else
				    val = 1;

			      byte |= val << (i0*2);
			}

			printf("0x%02x, ", byte);
		  }
	    }

      printf("};\n");
}

static void draw_TRIOR(void)
{
      unsigned i0, i1, i2, i3;

      printf("const unsigned char ft_TRIOR[64] = {");

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
			      else if ((i0 == 2) || (i1 == 2) || 
				       (i2 == 2) || (i3 == 2))
				    val = 2;
			      else if ((i0 == 3) && (i1 == 3) && 
				       (i2 == 3) && (i3 == 3))
				    val = 3;
			      else
				    val = 0;

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
      printf("# include  \"logic.h\"\n");
      draw_AND();
      draw_BUF();
      draw_BUFIF0();
      draw_BUFIF1();
      draw_BUFZ();
      draw_PMOS();
      draw_NMOS();
      draw_MUXX();
      draw_MUXZ();
      draw_EEQ();
      draw_NAND();
      draw_NOR();
      draw_NOT();
      draw_OR();
      draw_TRIAND();
      draw_TRIOR();
      draw_XNOR();
      draw_XOR();
      draw_hex_table();
      draw_oct_table();
      return 0;
}

/*
 * $Log: draw_tt.c,v $
 * Revision 1.15  2003/07/30 01:13:29  steve
 *  Add support for triand and trior.
 *
 * Revision 1.14  2002/08/29 03:04:01  steve
 *  Generate x out for x select on wide muxes.
 *
 * Revision 1.13  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.12  2002/01/12 04:02:16  steve
 *  Support the BUFZ logic device.
 *
 * Revision 1.11  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 * Revision 1.10  2001/10/09 02:28:17  steve
 *  Add the PMOS and NMOS functor types.
 *
 * Revision 1.9  2001/06/19 03:01:10  steve
 *  Add structural EEQ gates (Stephan Boettcher)
 *
 * Revision 1.8  2001/04/29 23:13:34  steve
 *  Add bufif0 and bufif1 functors.
 *
 * Revision 1.7  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.6  2001/04/21 02:04:01  steve
 *  Add NAND and XNOR functors.
 *
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

