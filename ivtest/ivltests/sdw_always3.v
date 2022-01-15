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
// SDW - always block with @(value) and (posedge OR negedge)
//
// D: New test used to validate always @(value), and
// D: always @(val1 or val2), and always @(posedge val1 or negedge val2)
// D: statements.
//
//

module main ();
reg working;
reg reset;
reg clock ;
reg test_var;

initial	// Used to generate timing of events
  begin
    clock = 0;
    reset = 0;

    // Nothing sent yet...

    #5 ;
    $display("time=%d, c=%b, r=%b, reg=%b",$time,clock,reset,test_var);
    reset = 1;	// 5 ns

    #5 ;
    $display("time=%d, c=%b, r=%b, reg=%b",$time,clock,reset,test_var);
    reset = 0;	// 10ns

    #5 ;
    $display("time=%d, c=%b, r=%b, reg=%b",$time,clock,reset,test_var);
    clock = 1;	// 15 ns

    #5 ;
    $display("time=%d, c=%b, r=%b, reg=%b",$time,clock,reset,test_var);
    clock = 0; // 20 ns

    #5 ;	// 25 ns
    $display("time=%d, c=%b, r=%b, reg=%b",$time,clock,reset,test_var);
  end

//
// This is the statement being verified...
//
// This LOOKS like a race between the posedge and the reset value
// but is a standard method for reseting f/f's.
//

always @(posedge clock or posedge reset)
  if(reset)
     test_var = 0;
  else
     test_var = ~test_var;

initial // This is the validation block
  begin
    # 3;	// 3 ns Check always @(val)
    if(test_var != 1'bx)
      begin
        $display("FAILED - initial condition of reg variable not x\n");
        $finish ;
      end
    # 5;	// 8 ns Check posedge of always @(posedge val or negedge)
    if(test_var == 1'bx)
      begin
        $display("FAILED - Reset didn't reset var \n");
        $finish ;
      end
    if(test_var == 1'b1)
      begin
        $display("FAILED - Reset set it to 1?? \n");
        $finish ;
      end

    # 5;	// 12 ns
    if(test_var == 1'bx)
      begin
        $display("FAILED - Reset didn't reset var \n");
        $finish ;
      end
    if(test_var == 1'b1)
      begin
        $display("FAILED - Reset set it to 1?? \n");
        $finish ;
      end
    # 5;	// 17 ns - have received the clock by now
    if(test_var == 1'bx)
      begin
        $display("FAILED -  The clock caused an x??\n");
        $finish ;
      end
    if(test_var == 1'b0)
      begin
        $display("FAILED - The clock didn't have any effect?? \n");
        $finish ;
      end
    # 30;

    $display("PASSED\n");
    $finish;
  end

endmodule
