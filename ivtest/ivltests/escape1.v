//
// Copyright (c) 2001  Steven Wilson (stevew@home.com)
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
//  SDW -  Tests defparam \$ in module name and if()

module test ();

parameter init = 16'h1234;
reg [15:0] rcv;

initial
  begin
          #10;
          rcv = init ;
          $display("Init value is %h",rcv);
  end

endmodule

module \$I178 ();

defparam \$I178 .a.init = 16'h0040;
defparam \$I178 .b.init = 16'h0041;
defparam \$I178 .c.init = 16'h0042;


test a ();
test b ();
test c ();

reg error;

initial
  begin
     error = 0;
     #20;
     if(\$I178 .a.rcv !== 16'h0040)
         begin
             $display("FAILED");
	     error = 1;
         end
     if(\$I178 .b.rcv !== 16'h0041)
         begin
             $display("FAILED");
	     error = 1;
         end
     if(\$I178 .c.rcv !== 16'h0042)
         begin
             $display("FAILED");
	     error = 1;
         end
     if(error === 0)
        $display("PASSED");
  end

endmodule
