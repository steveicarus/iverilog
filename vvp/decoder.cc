/*
 * Copyright (c) 2006 Stephen Williams (steve@icarus.com)
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
#ident "$Id: decoder.cc,v 1.1.2.2 2006/03/12 07:34:21 steve Exp $"

# include  "compile.h"
# include  "functor.h"
# include  "symbols.h"
# include  <stdlib.h>
# include  <malloc.h>
# include  <limits.h>
# include  <assert.h>

struct vvp_decode_adr_s;
struct vvp_decode_en_s;

static symbol_table_t decoder_table = 0;

static struct vvp_decode_adr_s* decoder_find(char *label)
{
      if (decoder_table == 0)
	    return 0;

      symbol_value_t v = sym_get_value(decoder_table, label);
      return (vvp_decode_adr_s*)v.ptr;
}

static void decoder_save(char*label, struct  vvp_decode_adr_s*obj)
{
      if (! decoder_table)
	    decoder_table = new_symbol_table();

      assert(! decoder_find(label));

      symbol_value_t v;
      v.ptr = obj;
      sym_set_value(decoder_table, label, v);
}

/*
 * This structure represents a decoder address object. We receive the
 * value changes of address bits, and accumulate the current address value.
 */
struct vvp_decode_adr_s : public functor_s {

      vvp_decode_adr_s(vvp_ipoint_t i, unsigned w);

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

	// functor index for this node, for finding all the input
	// functors related to me.
      vvp_ipoint_t ix;
      const unsigned width;
	// Keep a list of decode_en objects that reference me.
      struct vvp_decode_en_s* enable_list;
};

struct vvp_decode_en_s : public functor_s {

      vvp_decode_en_s(vvp_decode_adr_s*dec, unsigned s);

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

      void propagate_decoder_address(unsigned adr);

      struct vvp_decode_adr_s*decoder;
      struct vvp_decode_en_s* enable_next;

      const unsigned self;
	// This is the enable calculated from the decoder
      bool decode_en;
	// This is the enable that arrives from the extra input.
      bool extra_en;
	// This is the mass-enable that enables independent of address
      bool mass_en;
};

vvp_decode_adr_s::vvp_decode_adr_s(vvp_ipoint_t me, unsigned w)
: ix(me), width(w), enable_list(0)
{
}

void vvp_decode_adr_s::set(vvp_ipoint_t i, bool push,
			   unsigned val, unsigned str)
{
      functor_t ifu = functor_index(i);
      ifu->put(i, val);
      unsigned cur_adr = 0;
      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
	    int abval = functor_get_input(ix + idx);
	    unsigned mask = 1 << idx;
	    if (abval > 1)
		  cur_adr |= UINT_MAX;
	    else if (abval == 1)
		  cur_adr |= mask;

      }

      for (vvp_decode_en_s*cur = enable_list ; cur ; cur = cur->enable_next)
	    cur->propagate_decoder_address(cur_adr);
}

vvp_decode_en_s::vvp_decode_en_s(vvp_decode_adr_s*dec, unsigned s)
: self(s)
{
      enable_next = 0;
      decode_en = false;
      extra_en = true;
      mass_en = false;

      decoder = dec;
      enable_next = decoder->enable_list;
      decoder->enable_list = this;
}

void vvp_decode_en_s::set(vvp_ipoint_t i, bool push,
			   unsigned val, unsigned str)
{
      functor_t ifu = functor_index(i);
      ifu->put(i, val);

      int abval = functor_get_input(i);
      unsigned port = ipoint_port(i);
      switch (port) {
	  case 0: // regular enable
	    if (abval == 1) {
		  extra_en = true;
	    } else {
		  extra_en = false;
	    }
	    break;
	  case 1: // mass enable
	    if (abval == 1) {
		  mass_en = true;
	    } else {
		  mass_en = false;
	    }
	    break;
      }

      if (extra_en && decode_en || mass_en)
	    put_oval(1, true);
      else
	    put_oval(0, true);
}

void vvp_decode_en_s::propagate_decoder_address(unsigned adr)
{
      if (adr == self) {
	    decode_en = true;
      } else {
	    decode_en = false;
      }

      if (extra_en && decode_en)
	    put_oval(1, true);
      else
	    put_oval(0, true);
}

void compile_decode_adr(char*label, unsigned argc, struct symb_s*argv)
{
      unsigned nfun = (argc + 3)/4;
      vvp_ipoint_t ix = functor_allocate(nfun);

      vvp_decode_adr_s*a = new struct vvp_decode_adr_s(ix, argc);

      functor_define(ix, a);

      if (nfun > 0) {
	    extra_ports_functor_s *fu = new extra_ports_functor_s[nfun-1];
	    for (unsigned idx = 1 ;  idx < nfun ;  idx += 1) {
		  fu[idx-1].base_ = ix;
		  functor_define(ipoint_index(ix, idx), fu+idx-1);
	    }
      }

      inputs_connect(ix, argc, argv);
      free(argv);

      decoder_save(label, a);
      free(label);
}

void compile_decode_en(char*label, char*decoder, int slice,
		       struct symb_s enable,
		       struct symb_s mass_enable)
{
      vvp_decode_adr_s*adr = decoder_find(decoder);
      vvp_decode_en_s*a = new struct vvp_decode_en_s(adr, slice);

      vvp_ipoint_t ix = functor_allocate(1);
      functor_define(ix, a);

      symb_s argv[2];
      argv[0] = enable;
      argv[1] = mass_enable;
      inputs_connect(ix, 2, argv);

      define_functor_symbol(label, ix);
      free(label);
}

/*
 * $Log: decoder.cc,v $
 * Revision 1.1.2.2  2006/03/12 07:34:21  steve
 *  Fix the memsynth1 case.
 *
 * Revision 1.1.2.1  2006/02/19 00:11:36  steve
 *  Handle synthesis of FF vectors with l-value decoder.
 *
 */

