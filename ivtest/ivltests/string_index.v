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


// Tests accessing individual characters in a string

module string_index();

initial begin
    int i;
    string str = "that is a test string";

    for(i = 0; i < $size(str); ++i)
    begin
        if(str[i] == "t")
            str[i] = "w";
    end

    if(str != "whaw is a wesw swring")
    begin
        $display("FAILED");
        $finish();
    end

    $display("PASSED");
end

endmodule

