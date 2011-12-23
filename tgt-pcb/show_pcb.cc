/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "pcb_config.h"
# include  "pcb_priv.h"

using namespace std;

static void show_pcb_header(FILE*fpcb)
{
      fprintf(fpcb, "PCB[\"\" 400000 220000]\n");
      fprintf(fpcb, "Grid[100.0 0 0 1]\n");
}


void show_pcb(const char*pcb_path)
{
      assert(pcb_path);
      FILE*fpcb = fopen(pcb_path, "w");
      if (fpcb == 0) {
	    perror(pcb_path);
	    return;
      }

      show_pcb_header(fpcb);

	// Draw the collected elements
      for (map<string,element_data_t*>::const_iterator cur = element_list.begin()
		 ; cur != element_list.end() ; ++ cur) {

	    const string&refdes = cur->first;
	    const string&descr  = cur->second->description;
	    const string&value  = cur->second->value;
	    fprintf(fpcb, "Element[\"\" \"%s\" \"%s\" \"%s\"",
		    descr.c_str(), refdes.c_str(), value.c_str());

	      // Mark-X Mark-Y
	    fprintf(fpcb, " 0 0");
	      // Text-X Text-Y text-direction Text-scale Text-flags
	    fprintf(fpcb, " 0 0 0 100 \"\"");
	    fprintf(fpcb, "]\n");

	      // Fill in the contents of the element. Should get this
	      // from a library.
	    fprintf(fpcb, "(\n");
	    fprintf(fpcb, ")\n");
      }

      fclose(fpcb);
}
