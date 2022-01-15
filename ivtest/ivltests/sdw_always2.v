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
//
// SDW - Validate always with OR, posedge constructs.
//
// D: New test used to validate always @(value), and
// D: always @(val1 or val2), and always @(posedge val1 or negedge val2)
// D: statements.
//
//

module main ();
reg working;
reg val1, val2, val3, val4, val5;
reg clock ;
reg test_var;

initial	// Used to generate timing of events
  begin
    val2 = 0;
    val3 = 0;
    val4 = 1;
    # 1;
    test_var = 1;
    # 1;
    val1 = 1;		// Cause a change in val1 -> test_var to 0.
    # 2 ;		// 4ns
    test_var = 1;
    # 1 ;		// 5ns
    val2 = 1;		// Cause posedge on val2 -> test_var to 0
    # 2;		// 7ns
    test_var = 1;
    # 1;		// 8ns
    val4 = 0;		// Cause negedge on val4 -> test_var to 0
    # 2;		// 10ns
    test_var = 1;
    # 1;		// 11 ns
    val3 = 1;		// Cause val3 change for always @(a or b)
    # 2;		// 13 ns
    test_var = 1;
    # 1;		// 14 ns
    val5 = 1;		// Cause val5 cahnge for always @(a or b)
    # 2;		// 16 ns
  end

always @(val1)	// Val1 changing clears test_var
  test_var = 0;

always @(posedge val2 or negedge val4)
  test_var = 0;

always @(val3 or val5)
  test_var = 0;


initial // This is the validation block
  begin
    # 3;	// 3 ns Check always @(val)
    if(test_var)
      begin
        $display("FAILED - always @(val)  wrong \n");
        $finish ;
      end
    # 3;	// 6 ns Check posedge of always @(posedge val or negedge)
    if(test_var)
      begin
        $display("FAILED - posedge of always @(posedge or negedge) wrong \n");
        $finish ;
      end
    # 3;	// 9 ns Check negedge of always @(posedge val or negedge)
    if(test_var)
      begin
        $display("FAILED - negedge of always @(posedge or negedge) wrong \n");
        $finish ;
      end
    # 3;	// 12 ns Check a of always @(a or b)
    if(test_var)
      begin
        $display("FAILED - a of always @(a or b) wrong \n");
        $finish ;
      end
    # 3;	// 15 ns Check b of always @(a or b)
    if(test_var)
      begin
        $display("FAILED - b of always @(a or b) wrong \n");
        $finish ;
      end

    $display("PASSED\n");
    $finish;
  end

always @ (posedge clock)
    working = 1;

always @ (negedge clock)
    working = 1;

endmodule
