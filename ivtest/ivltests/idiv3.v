/*
 * Copyright (c) 2002 Simon Denman
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
//
// File:         DivBug.v
// Author:       Simon Denman
// Created:      28/3/02
// Description:  integer division bug test

module DivBug ;

	integer     intX, intY;

    initial
    begin
        intX = -8;
        intY = intX / 8;
	$display("%5d %5d", intX, intY);
    end

endmodule
