/*
 * Copyright (c) 2000 Adrian Lewis (indproj@yahoo.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpi_vlog_info.c,v 1.2 2000/09/12 01:17:40 steve Exp $"
#endif

#include <vpi_user.h>

// STORAGE FOR COMMAND LINE ARGUMENTS

static int    vpip_argc;
static char** vpip_argv;

// ROUTINE: vpi_get_vlog_info
//
// ARGUMENT: vlog_info_p
//		Pointer to a structure containing simulation information.
//
// RETURNS:
//		Boolean: true on success and false on failure.
//
// DESCRIPTION:
//		Retrieve information about Verilog simulation execution.

int
vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p)
{
	// AUTOMATICALLY UPDATING THE VERSION NUMBER WOULD BE A GOOD IDEA

	static char* version = "$Name:  $";
	static char* product = "Icarus Verilog";

	// CHECK THAT THE USER DIDN'T PASS A NULL POINTER

	if (vlog_info_p != 0)
	{
		// FILL IN INFORMATION FIELDS

		vlog_info_p->product = product;
		vlog_info_p->version = version;
		vlog_info_p->argc    = vpip_argc;
		vlog_info_p->argv    = vpip_argv;

		return 1==1;
	}
	else
		return 1==0;
}

// ROUTINE: vpip_set_vlog_info
//
// ARGUMENTS: argc, argv
//		Standard command line arguments.
//
// DESCRIPTION:
//		Saves command line arguments to retrieval by vpi_get_vlog_info.

void
vpip_set_vlog_info(int argc, char** argv)
{
	// SAVE COMMAND LINE ARGUMENTS IN STATIC VARIABLES

	vpip_argc = argc;
	vpip_argv = argv;
}
