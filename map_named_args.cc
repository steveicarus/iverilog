// SPDX-FileCopyrightText: 2023 Lars-Peter Clausen <lars@metafoo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PExpr.h"
#include "ivl_assert.h"
#include "map_named_args.h"
#include "netlist.h"

#include <iostream>

std::vector<PExpr*> map_named_args(Design *des,
			           const std::vector<perm_string> &names,
			           const std::vector<named_pexpr_t> &parms)
{
      std::vector<PExpr*> args(names.size());

      bool has_named = false;
      for (size_t i = 0; i < parms.size(); i++) {
	    if (parms[i].name.nil()) {
		  if (!parms[i].parm)
			continue;

		  if (has_named) {
		      std::cerr << parms[i].get_fileline() << ": error: "
		           << "Positional argument must preceded "
			   << "named arguments."
			   << std::endl;
		  } else if (i < args.size()) {
			args[i] = parms[i].parm;
		  }

		  continue;
	    }
	    has_named = true;

	    bool found = false;
	    for (size_t j = 0; j < names.size(); j++) {
		  if (names[j] == parms[i].name) {
			if (args[j]) {
			      std::cerr << parms[i].get_fileline() << ": error: "
			           << "Argument `"
				   << parms[i].name
				   << "` has already been specified."
				   << std::endl;
			      des->errors++;
			} else {
			      args[j] = parms[i].parm;
			}
			found = true;
			break;
		  }
	    }
	    if (!found) {
		  std::cerr << parms[i].get_fileline() << ": error: "
		       << "No argument called `"
		       << parms[i].name << "`."
		       << std::endl;
		  des->errors++;
	    }
      }

      return args;
}

std::vector<PExpr*> map_named_args(Design *des, NetBaseDef *def,
				   const std::vector<named_pexpr_t> &parms,
				   unsigned int off)
{
      std::vector<perm_string> names;

      for (size_t j = off; j < def->port_count(); j++)
	    names.push_back(def->port(j)->name());

      return map_named_args(des, names, parms);
}
