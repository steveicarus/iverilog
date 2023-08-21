// SPDX-FileCopyrightText: 2023 Lars-Peter Clausen <lars@metafoo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MAP_NAMED_ARGS_H
#define MAP_NAMED_ARGS_H

#include <vector>
#include "pform_types.h"

class PExpr;
class Design;
class NetBaseDef;

std::vector<PExpr*> map_named_args(Design *des,
			           const std::vector<perm_string> &names,
			           const std::vector<named_pexpr_t> &parms);

std::vector<PExpr*> map_named_args(Design *des, NetBaseDef *def,
			           const std::vector<named_pexpr_t> &parms,
				   unsigned int off);

#endif
