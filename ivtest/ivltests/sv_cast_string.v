// Copyright (c) 2014 CERN
// Maciej Suminski <maciej.suminski@cern.ch>
//
// This source code is free software; you can redistribute it
// and/or modify it in source code form under the terms of the GNU
// General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


// Test for casting a string to a vector type.

module sv_cast_string();
   string str;
   typedef logic [55:0] strbits;
   strbits chars;

 initial begin
    int i;
    str = "0123456";
    chars = strbits'(str);
    if(chars != 56'h30313233343536)
    begin
        $display("FAILED 1 chars = %x", chars);
        $finish();
    end

    str = "6543210";
    chars = strbits'(str);
    if(chars != "6543210")
    begin
        $display("FAILED 2 chars = %x", chars);
        $finish();
    end

    str = "wrong string";
    // Vector to string casting
    str = string'(chars);
    if(str != "6543210")
    begin
        $display("FAILED 3 str = %s", str);
        $finish();
    end

    $display("PASSED");
end
endmodule
