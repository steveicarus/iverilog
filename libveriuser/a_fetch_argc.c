/* vi:sw=6
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_fetch_argc.c,v 1.2 2002/08/12 01:35:02 steve Exp $"
#endif

# include  <vpi_user.h>
# include  <veriuser.h>

/*
 * acc_fetch_argc implemented using VPI interface
 */
int acc_fetch_argc(void)
{
      s_vpi_vlog_info vpi_vlog_info;

      /* get command line info */
      if (! vpi_get_vlog_info(&vpi_vlog_info))
	    return 0;

      /* return argc */
      return vpi_vlog_info.argc;
}

/*
 * $Log: a_fetch_argc.c,v $
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/11 15:19:12  steve
 *  Add acc_fetch_argc/argv/version (mruff)
 *
 */
