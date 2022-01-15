/*
 * Copyright (c) 2014 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This source code is free software; you can redistribute it
 * and/or modify it in source code form under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

// Test for VPI functions handling dynamic arrays

module main();
initial
  begin
    int int_darray[];
    real real_darray[];
    bit [63:0] bit_darray[];
    string string_darray[];

    int_darray = new[4];
    int_darray = '{1, 2, 3, 4};
    $display_array(int_darray);
    $increase_array_vals(int_darray);
    $display_array(int_darray);

    real_darray = new[2];
    real_darray = '{2.2, 2.3};
    $increase_array_vals(real_darray);
    $display_array(real_darray);

    bit_darray = new[4];
    bit_darray = '{64'hdeadbeefcafebabe, 64'h0badc0dec0dec0de,
                   64'h0123456789abcdef, 64'hfedcba9876543210};
    $increase_array_vals(bit_darray);
    $display_array(bit_darray);

    string_darray = new[4];
    string_darray = '{"test string", "another one", "yet one more", "the last one"};
    $increase_array_vals(string_darray);
    $display_array(string_darray);
  end
endmodule
