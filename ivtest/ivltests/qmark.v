//
// Copyright (c) 1999 Steven Wilson (stevew@home.com)
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
//  SDW - Validate the ? operator


module main;

reg globvar;

reg [3:0] bvec,var1,var2,var3;
reg cond, a,b,out1,out2;
reg error;

initial
  begin
    error = 0;
    bvec = 4'bzx10 ;
    for(var1 = 0; var1 <= 3; var1 = var1 + 1)
      for(var2 = 0; var2 <= 3; var2 = var2 + 1)
         for(var3 = 0; var3 <= 3; var3 = var3 + 1)
           begin
              cond = bvec[var1];
              a =    bvec[var2];
              b =    bvec[var3];
              out1 = cond ? a: b ;
              if(cond) out2 = a ;
              else out2 = b;
              if(out1 != out2)
                 begin
                   $display("FAILED - %b %b %b %b %b",cond,a,b,out1,out2);
                    error = 1;
                 end
           end
    if(error == 0)
       $display("PASSED");
  end

endmodule // main
