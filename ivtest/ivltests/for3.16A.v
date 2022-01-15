//
// Copyright (c) 2000 Steve Wilson (stevew@home.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//    for3.16A -  Template 1 - for(val1=0; val1 <= expr ; val1 = val1 + 1) some_action
//

module test ;

reg [3:0] val1;
reg [3:0] val2;

initial
  begin
   val2 = 0;
   for(val1 = 0; val1 <= 4'ha; val1 = val1+1)
     begin
       val2 = val2 + 1;
     end
   if(val2 === 4'hb)
     $display("PASSED");
   else
     begin
       $display("FAILED val2 s/b 4'ha, but is %h",val2);
     end

  end

endmodule
