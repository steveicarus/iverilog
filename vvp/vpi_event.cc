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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_event.cc,v 1.5 2002/08/12 01:35:08 steve Exp $"
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

static char* named_event_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      char *bn = vpi_get_str(vpiFullName, &obj->scope->base);
      const char *nm = obj->name;

      char *rbuf = need_result_buf(strlen(bn) + strlen(nm) + 1, RBUF_STR);

      switch (code) {

	  case vpiFullName:
	    sprintf(rbuf, "%s.%s", bn, nm);
	    return rbuf;

	  case vpiName:
	    strcpy(rbuf, nm);
	    return rbuf;

      }

      return 0;
}

static vpiHandle named_event_get_handle(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      switch (code) {

	  case vpiScope:
	    return &obj->scope->base;
      }

      return 0;
}

/*
 * We keep the object around, in case we need it again. It's all we
 * can do, because it cannot be recreated.
 */
static int named_event_free_object(vpiHandle)
{
      return 0;
}

static const struct __vpirt vpip_named_event_rt = {
      vpiNamedEvent,

      0,
      named_event_str,
      0,
      0,

      named_event_get_handle,
      0,
      0,

      named_event_free_object
};

vpiHandle vpip_make_named_event(const char*name)
{
      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)
	    malloc(sizeof(struct __vpiNamedEvent));

      obj->base.vpi_type = &vpip_named_event_rt;
      obj->name = vpip_string(name);
      obj->scope = vpip_peek_current_scope();
      obj->callbacks = 0;

      return &obj->base;
}

void vpip_run_named_event_callbacks(vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      struct __vpiCallback*cur = obj->callbacks;
      while (cur) {
	    struct __vpiCallback*next = cur->next;
	    callback_execute(cur);
	    cur = next;
      }
}

/*
 * $Log: vpi_event.cc,v $
 * Revision 1.5  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/07/12 18:23:30  steve
 *  Use result buf for event and scope names.
 *
 * Revision 1.3  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.2  2002/05/19 05:18:16  steve
 *  Add callbacks for vpiNamedEvent objects.
 *
 * Revision 1.1  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 */

