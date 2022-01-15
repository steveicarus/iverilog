/*
 * Copyright (c) 2000 Intrinsity, Inc.
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
module test_notif0 ();

reg gnd, vdd, x, z;
wire t0, t1, t2, t3, t4, t5, t6, t7,
     t8, t9, ta, tb, tc, td, te, tf;

reg failed;

wire StH, StL;

assign (strong1, highz0)  StH = 1'bx;
assign (highz1,  strong0) StL = 1'bx;


notif0 n0 ( t0,  gnd,  gnd);
notif0 n1 ( t1,  gnd,  vdd);
notif0 n2 ( t2,  gnd,  x);
notif0 n3 ( t3,  gnd,  z);

notif0 n4 ( t4,  vdd,  gnd);
notif0 n5 ( t5,  vdd,  vdd);
notif0 n6 ( t6,  vdd,  x);
notif0 n7 ( t7,  vdd,  z);

notif0 n8 ( t8,  x,  gnd);
notif0 n9 ( t9,  x,  vdd);
notif0 na ( ta,  x,  x);
notif0 nb ( tb,  x,  z);

notif0 nc ( tc,  z,  gnd);
notif0 nd ( td,  z,  vdd);
notif0 ne ( te,  z,  x);
notif0 nf ( tf,  z,  z);

initial begin

  assign gnd = 1'b1;
  assign vdd = 1'b0;
  assign x = 1'b0;
  assign z = 1'b0;
  #10;

  assign gnd = 1'b0;
  assign vdd = 1'b1;
  assign x = 1'b1;
  assign z = 1'b1;
  #10;

  assign gnd = 1'b0;
  assign vdd = 1'b1;
  assign x = 1'bx;
  assign z = 1'bz;
  #10;

  failed = 0;

  if (t0 !== vdd)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:1", gnd, gnd, t0 );
  end

  if (t1 !== z)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:z", gnd, vdd, t1 );
  end
  if (t2 !== StH)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:StH", gnd, x, t2 );
  end
  if (t3 !== StH)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:StH", gnd, z, t3 );
  end

  if (t4 !== 1'b0)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:0", vdd, gnd, t4 );
  end
  if (t5 !== z)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:z", vdd, vdd, t5 );
  end
  if (t6 !== StL)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:Stl", vdd, x, t6 );
  end
  if (t7 !== StL)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:Stl", vdd, z, t7 );
  end

  if (t8 !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", x, gnd, t8 );
  end
  if (t9 !== 1'bz)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:z", x, vdd, t9 );
  end
  if (ta !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", x, x, ta );
  end
  if (tb !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", x, z, tb );
  end

  if (tc !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", z, gnd, tc );
  end
  if (td !== 1'bz)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:z", z, vdd, td );
  end
  if (te !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", z, x, te );
  end
  if (tf !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif0 s:%d g:%d d:%d expected:x", z, z, tf );
  end

  if (failed == 0)
      $display ("PASSED");
end
endmodule
