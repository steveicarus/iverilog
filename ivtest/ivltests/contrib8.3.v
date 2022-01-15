/*
 * Copyright (c) 1998 Philips Semiconductors (Stefan.Thiede@sv.sc.philips.com)
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

module test();

MUX_REG_8x8 PAGE_REG_B3 (
			.CLK	(CLK),
			/*
			.IN	(DATA_RES[31:24]),
			.OUT	(PAGE[31:24]),
			.EN_IN	(EN_B3),
			.EN_OUT	(PAGE_SEL),
			*/
			.TC	(),
			.TD	(),
			.TQ	());

endmodule
