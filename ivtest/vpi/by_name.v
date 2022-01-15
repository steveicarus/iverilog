// Copyright (c) 2002	Michael Ruff (mruff at chiaro.com)
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
module lvl2;
    reg r;
    time t;
    event e;
    integer i;

    task t_my;
	r = 1'b0;
    endtask

    function f_my;
	input in;
    begin
	f_my = !in;
    end
    endfunction

    initial begin: init
	t_my;
	r = f_my(r);
	r = f_my(i);
	r = f_my(t);
	->e;
    end
endmodule

module lvl1;
    lvl2 lvl2();
endmodule

module top0;
    reg r;
    time t;
    event e;
    integer i;

    task t_my;
	r = 1'b0;
    endtask

    function f_my;
	input in;
    begin
	f_my = !in;
    end
    endfunction

    initial begin: init
	t_my;
	r = f_my(r);
	r = f_my(i);
	r = f_my(t);
	->e;
    end

    lvl1 lvl1_0();
    lvl1 lvl1_1();
endmodule


module top1;
    initial begin: init
	$test;
    end
endmodule
