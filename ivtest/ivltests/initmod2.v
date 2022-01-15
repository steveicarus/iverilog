/*
 * Copyright (c) 2000 Yasuhisa Kato <ykato@mac.com>
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
// Modified my stevew@home.com to be self-checking per the comments.

module  main;

    reg     clk ;
    initial begin clk = 0 ; forever #5 clk = ~clk ; end
    initial #20 $finish;

    wire    w,   ww, wr, w1, wwr, ww1, wr1, wwro, ww1o, wr1o ;
    reg     r,   rw ;
    reg    error;

                                //  z <- (z) = z
    assign  ww  =   w       ;   //  z <- (z) = z
    assign  wr  =   r       ;   //  x <- (z) = x
    assign  w1  =   'b1     ;   //  1 <- (z) = 1
    assign  wwr =   w & r   ;   //  x <-  z  & x
    assign  ww1 =   w & 'b1 ;   //  x <-  z  & 1
    assign  wr1 =   r & 'b1 ;   //  x <-  x  & 1

    assign  wwro=   w | r   ;   //  x <-  z  | x
    assign  ww1o=   w | 'b1 ;   //  1 <-  z  | 1
    assign  wr1o=   r | 'b1 ;   //  1 <-  x  | 1

    always @(posedge clk)
          rw <= w ;               //  x <- (x) = z

    always @(posedge clk)
      begin
        #1;
        $display("%b %b %b %b %b %b %b : %b %b %b : %b %b",
        w, ww, wr, w1, wwr, ww1, wr1, wwro, ww1o, wr1o, r, rw );
      end

initial
   begin
      error = 0;
      #19;
      if(ww !== 1'bz)  begin
        error = 1;
        $display("FAILED - ww s/b z, is %h",ww);
      end
      if(wr !== 1'bx)  begin
        error = 1;
        $display("FAILED - wr s/b x, is %h",wr);
      end
      if(w1 !== 1'b1)  begin
        error = 1;
        $display("FAILED - wr s/b 1, is %h",wr);
      end
      if(wwr !== 1'bx)  begin
        error = 1;
        $display("FAILED - wwr s/b x, is %h",wwr);
      end
      if(ww1 !== 1'bx)  begin
        error = 1;
        $display("FAILED - ww1 s/b x, is %h",ww1);
      end
      if(wr1 !== 1'bx)  begin
        error = 1;
        $display("FAILED - wr1 s/b x, is %h",wr1);
      end
      if(wwro !== 1'bx)  begin
        error = 1;
        $display("FAILED - wwro s/b 1, is %h",wwro);
      end
      if(wr1o !== 1'b1)  begin
        error = 1;
        $display("FAILED - wr1o s/b 1, is %h",wr1o);
      end
      if(r !== 1'bx)  begin
        error = 1;
        $display("FAILED - r s/b x, is %h",r);
      end
      if(r !== 1'bx)  begin
        error = 1;
        $display("FAILED - r s/b x, is %h",r);
      end
      if(rw !== 1'bz)  begin
        error = 1;
        $display("FAILED - rw s/b z, is %h",r);
      end
      if(error === 0)
        $display("PASSED");
      $finish(0);
   end

endmodule

// *Initial Value Test*

//  expected output - This according to XL
//  z z x 1 x x x : x 1 1 : x x
//  z z x 1 x x x : x 1 1 : x z

//  ivl current result
//  z z x 1 x z x : x 1 1 : x x
//  z z x 1 x z x : x 1 1 : x z
