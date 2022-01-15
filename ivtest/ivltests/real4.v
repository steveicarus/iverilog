// Copyright (c) 2003   Michael Ruff (mruff at chiaro.com)
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
// Test basic functionality of convertion system VPI functions.
//
module test;

    integer	err, i;
    real	r;
    reg [63:0]	b;

    parameter PI = 3.1415926535_8979323846_2643383279;

    initial begin
	err = 0;

	//
	// $rtoi()
	//
	i = $rtoi(0.1);
	if (i != 0) begin
	    err = 1;
	    $display("$rtoi(0.1): %0d != 0", i);
	end
	i = $rtoi(9.6);
	if (i != 9) begin
	    err = 1;
	    $display("$rtoi(9.6): %0d != 9", i);
	end

	//
	// $realtobits()
	//
	b = $realtobits(PI);
	if (b != 64'h400921FB54442D18) begin
	    err = 1;
	    $display("$realtobits(PI): 'h%x != 'h400921FB54442D18", b);
	end
	b = $realtobits(1.1);
	if (b != 64'h3ff199999999999a) begin
	    err = 1;
	    $display("$realtobits(1.1): 'h%x != 'h400921FB54442D18", b);
	end

	//
	// $bitstoreal()
	//
	r = $bitstoreal(64'h400921FB54442D18);
	if (r != PI) begin
	    err = 1;
	    $display("$realtobits(PI): %20.17f != %20.17f", r, PI);
	end
	r = $bitstoreal(64'h3FF4CCCCCCCCCCCD);
	if (r != 1.3) begin
	    err = 1;
	    $display("$realtobits(1.3): %20.17f != 1.3", r);
	end

	//
	// $itor()
	//
	r = $itor(1);
	if (r != 1.0) begin
	    err = 1;
	    $display("$itor(1): %20.1f != 1.0", r);
	end
	r = $itor(123456789);
	if (r != 123456789.0) begin
	    err = 1;
	    $display("$itor(123456789): %20.1f != 123456789.0", r);
	end

	if (err)
	    $display("FAILED");
	else
	    $display("PASSED");

	$finish;
    end

endmodule
