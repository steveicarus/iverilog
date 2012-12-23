/*
 * Copyright (c) 2008-2011 Stephen Williams <steve@icarus.com>
 * Copyright (c) 2002 Larry Doolittle (larry@doolittle.boa.org)
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

# include  "config.h"
# include  "vpi_priv.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cstring>
# include  <climits>     /* for CHAR_BIT */
# include  <cstdlib>
# include  <cctype>
# include  <cassert>
# include  "ivl_alloc.h"

/* If you are allergic to malloc, you can set a stack memory allocation
 * here.  Otherwise, malloc() is used for the temporary array, so the
 * conversion length is unlimited. */
/* #define MAX_DIGITS 20 */


#if SIZEOF_UNSIGNED_LONG * CHAR_BIT >= 64
#define BDIGITS 9
#define BASE 1000000000
#define BBITS 32
#define BMASK 0xffffffff
#else
#if SIZEOF_UNSIGNED_LONG * CHAR_BIT >= 32
#define BDIGITS 4
#define BASE 10000
#define BBITS 16
#define BMASK 0xffff
#else
#error apparent non-conforming word length
#endif
#endif

#define B_IS0(x)  ((x) == 0)
#define B_IS1(x)  ((x) == 1)
#define B_ISX(x)  ((x) == 2)
#define B_ISZ(x)  ((x) == 3)

/* The program works by building a base BASE representation of the number
 * in the valv array.  BBITS bits of the number can be put in at a time.
 * Previous values of each valv element are always less than BASE, the
 * input val is less than or equal to 2^BBITS, so (valv[i]<<BBITS)+val
 * is guaranteed less than or equal to BASE<<BBITS, which is configured
 * less than ULONG_MAX.  When this number divided by BASE, to get the amount
 * propagated as a "carry" to the next array element, the result is again
 * less than or equal to 2^BBITS.  BBITS and BASE are configured above
 * to depend on the "unsigned long" length of the host, for efficiency.
 */
static inline void shift_in(unsigned long *valv, unsigned int vlen, unsigned long val)
{
	unsigned int i;
	/* printf("shift in %u\n",val); */
	for (i=0; i<vlen; i++) {
		val=(valv[i]<<BBITS)+val;
		valv[i]=val%BASE;
		val=val/BASE;
	}
	if (val!=0)
	      fprintf(stderr,"internal error: carry out %lu in " __FILE__ "\n",val);
}

/* Since BASE is a power of ten, conversion of each element of the
 * valv array to decimal is easy.  sprintf(buf,"%d",v) could be made
 * to work, I suppose, but for speed and control I prefer to write
 * the steps out longhand.
 */
static inline int write_digits(unsigned long v, char **buf,
                               unsigned int *nbuf, int zero_suppress)
{
	char segment[BDIGITS];
	int i;
	for (i=BDIGITS-1; i>=0; --i) {
		segment[i] = '0' + v%10;
		v=v/10;
	}
	for (i=0; i<BDIGITS; ++i) {
		if (!(zero_suppress&=(segment[i]=='0'))) {
			*(*buf)++=segment[i]; --(*nbuf);
		}
	}
	return zero_suppress;
}

/* Jump through some hoops so we don't have to malloc/free valv
 * on every call, and implement an optional malloc-less version. */
static unsigned long *valv=NULL;
static unsigned int vlen_alloc=0;

#ifdef CHECK_WITH_VALGRIND
void dec_str_delete(void)
{
      free(valv);
      valv = 0;
      vlen_alloc = 0;
}
#endif

unsigned vpip_vec4_to_dec_str(const vvp_vector4_t&vec4,
			      char *buf, unsigned int nbuf,
			      int signed_flag)
{
      unsigned int idx, vlen;
      unsigned int mbits=vec4.size();   /* number of non-sign bits */
      unsigned count_x = 0, count_z = 0;

      unsigned long val=0;
      int comp=0;
      if (signed_flag) {
	    switch (vec4.value(vec4.size()-1)) {
		case BIT4_X:
		  count_x += 1;
		  break;
		case BIT4_Z:
		  count_z += 1;
		  break;
		case BIT4_1:
		  comp=1;
		  break;
		case BIT4_0:
		  break;
	    }
	    mbits -= 1;
      }
      assert(mbits<(UINT_MAX-92)/28);
      vlen = ((mbits*28+92)/93+BDIGITS-1)/BDIGITS;
	/* printf("vlen=%d\n",vlen); */

#define ALLOC_MARGIN 4
      if (!valv || vlen > vlen_alloc) {
	    if (valv) free(valv);
	    valv = (unsigned long*) calloc(vlen+ALLOC_MARGIN, sizeof (*valv));
	    vlen_alloc=vlen+ALLOC_MARGIN;
      } else {
	    memset(valv,0,vlen*sizeof(valv[0]));
      }

      for (idx = 0; idx < mbits; idx += 1) {
	      /* printf("%c ",bits[mbits-idx-1]); */
	    switch (vec4.value(mbits-idx-1)) {
		case BIT4_Z:
		  count_z += 1;
		  break;
		case BIT4_X:
		  count_x += 1;
		  break;
		case BIT4_1:
		  if (! comp)
			val += 1;
		  break;
		case BIT4_0:
		  if (comp)
			val += 1;
		  break;
	    }

	    if ((mbits-idx-1)%BBITS==0) {
		    /* make negative 2's complement, not 1's complement */
		  if (comp && idx==mbits-1) ++val;
		  shift_in(valv,vlen,val);
		  val=0;
	    } else {
		  val=val+val;
	    }
      }

	if (count_x == vec4.size()) {
	      buf[0] = 'x';
	      buf[1] = 0;
	} else if (count_x > 0) {
	      buf[0] = 'X';
	      buf[1] = 0;
	} else if (count_z == vec4.size()) {
	      buf[0] = 'z';
	      buf[1] = 0;
	} else if (count_z > 0) {
	      buf[0] = 'Z';
	      buf[1] = 0;
	} else {
	      int i;
	      int zero_suppress=1;
	      if (comp) {
		    *buf++='-';
		    nbuf--;
		      /* printf("-"); */
	      }
	      for (i=vlen-1; i>=0; i--) {
		    zero_suppress = write_digits(valv[i],
						 &buf,&nbuf,zero_suppress);
		      /* printf(",%.4u",valv[i]); */
	      }
		/* Awkward special case, since we don't want to
		 * zero suppress down to nothing at all.  The only
		 * way we can still have zero_suppress on in the
		 * comp=1 case is if mbits==0, and therefore vlen==0.
		 * We represent 1'sb1 as "-1". */
	      if (zero_suppress) *buf++='0'+comp;
		/* printf("\n"); */
	      *buf='\0';
	}
	return 0;
}

void vpip_dec_str_to_vec4(vvp_vector4_t&vec, const char*buf)
{
	/* Support for [xX]_*. */
      if (buf[0] == 'x' || buf[0] == 'X') {
	    for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
		  vec.set_bit(idx, BIT4_X);
	    }
	    const char*tbuf = buf+1;
	      /* See if this is a valid constant. */
	    while (*tbuf) {
		  if (*tbuf != '_') {
			fprintf(stderr, "Warning: Invalid decimal \"x\" "
			                "value \"%s\".\n", buf);
			return;
		  }
		  tbuf += 1;
	    }
	    return;
      }

	/* Support for [zZ]_*. */
      if (buf[0] == 'z' || buf[0] == 'Z') {
	    const char*tbuf = buf+1;
	      /* See if this is a valid constant, if not return "x". */
	    while (*tbuf) {
		  if (*tbuf != '_') {
			fprintf(stderr, "Warning: Invalid decimal \"z\" "
			                "value \"%s\".\n", buf);
			for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
			      vec.set_bit(idx, BIT4_X);
			}
			return;
		  }
		  tbuf += 1;
	    }
	    for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
		  vec.set_bit(idx, BIT4_Z);
	    }
	    return;
      }

	/* The str string is the decimal value with the least
	   significant digit first. This loop creates that string by
	   reversing the order of the buf string. For example, if the
	   input is "1234", str gets "4321". */
      unsigned slen = strlen(buf);
      char*str = new char[slen + 1];
      bool is_negative = false;
      for (unsigned idx = 0 ;  idx < slen ;  idx += 1) {
	    if (idx == slen-1 && buf[slen-idx-1] == '-') {
	          is_negative = true;
		  slen--;
		  continue;
	    }
	    while (buf[slen-idx-1] == '_') {
		  slen--;
	    }
	    if (isdigit(buf[slen-idx-1]))
		  str[idx] = buf[slen-idx-1];
            else {
		    /* Return "x" if there are invalid digits in the string. */
		  fprintf(stderr, "Warning: Invalid decimal digit %c(%d) in "
		          "\"%s.\"\n", buf[slen-idx-1], buf[slen-idx-1], buf);
		  for (unsigned jdx = 0 ;  jdx < vec.size() ;  jdx += 1) {
			vec.set_bit(jdx, BIT4_X);
		  }
		  return;
            }
      }

      str[slen] = 0;

      for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
	    vvp_bit4_t val4 = BIT4_0;

	    switch (str[0]) {
		case '1':
		case '3':
		case '5':
		case '7':
		case '9':
		  val4 = BIT4_1;
		  break;
	    }

	    vec.set_bit(idx, val4);

	      /* Divide the str string by 2 in decimal. */
	    char*cp = str;
	    while (*cp) {
		  unsigned val = cp[0] - '0';
		  if ((val&1) && (cp > str))
			cp[-1] += 5;

		  cp[0] = '0' + val/2;
		  cp += 1;
	    }

      }

      if (is_negative) {
            vec.invert();
            vec += (int64_t) 1;
      }

      delete[]str;
}
