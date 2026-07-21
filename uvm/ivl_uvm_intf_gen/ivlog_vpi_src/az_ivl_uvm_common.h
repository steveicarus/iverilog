/**********************************************************************
 * IVL_UVM common ulitiy functions
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
 *
 *********************************************************************/

#include <stdio.h>     /* ANSI C standard input/output library */
#include "vpi_user.h"  /* IEEE 1364 PLI VPI routine library  */

void ivl_uvm_pr_copyright () {

  char * ivl_uvm_cright_msg;

  ivl_uvm_cright_msg 
    = "\tWelcome to IVL_UVM (http://https://github.com/svenka3/ivl_uvm)\n\
\tVersion 2020.01 Copyright 2019-2022 AumzDA LLC\n\
\t\tAll Rights Reserved. \n";
  
  vpi_printf("\t--------------------------------------------------------------------\n");
  vpi_printf ("%s\n", ivl_uvm_cright_msg);

  vpi_printf("\t--------------------------------------------------------------------\n");
 
}


