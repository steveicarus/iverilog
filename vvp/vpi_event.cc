/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_event.cc,v 1.1 2002/05/18 02:34:11 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "functor.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

struct __vpiNamedEvent {
      struct __vpiHandle base;
	/* base name of the event object */
      char*name;
	/* Parent scope of this object. */
      struct __vpiScope*scope;
};

static const struct __vpirt vpip_named_event_rt = {
      vpiNamedEvent,

      0,
      0,
      0,
      0,

      0,
      0,
      0,

      0
};

vpiHandle vpip_make_named_event(char*name)
{
      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)
	    malloc(sizeof(struct __vpiNamedEvent));

      obj->base.vpi_type = &vpip_named_event_rt;
      obj->name = name;
      obj->scope = vpip_peek_current_scope();

      return &obj->base;
}

/*
 * $Log: vpi_event.cc,v $
 * Revision 1.1  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 */

