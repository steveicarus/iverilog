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
module test_notif1 ( );

reg gnd, vdd, x, z;

wire t0, t1, t2, t3, t4, t5, t6, t7,
     t8, t9, ta, tb, tc, td, te, tf;

reg failed;

wire StH, StL;

assign (strong1, highz0)  StH = 1'bx;
assign (highz1,  strong0) StL = 1'bx;

notif1 b0 ( t0,  gnd,  gnd);
notif1 b1 ( t1,  gnd,  vdd);
notif1 b2 ( t2,  gnd,  x);
notif1 b3 ( t3,  gnd,  z);

notif1 b4 ( t4,  vdd,  gnd);
notif1 b5 ( t5,  vdd,  vdd);
notif1 b6 ( t6,  vdd,  x);
notif1 b7 ( t7,  vdd,  z);

notif1 b8 ( t8,  x,  gnd);
notif1 b9 ( t9,  x,  vdd);
notif1 ba ( ta,  x,  x);
notif1 bb ( tb,  x,  z);

notif1 bc ( tc,  z,  gnd);
notif1 bd ( td,  z,  vdd);
notif1 be ( te,  z,  x);
notif1 bf ( tf,  z,  z);

initial begin

  //
  // work around initial state assignment bug
  failed = 0;
  assign gnd = 1'b1;
  assign vdd = 1'b0;
  assign x = 1'b1;
  assign z = 1'b1;
  #10;

  assign gnd = 1'b0;
  assign vdd = 1'b1;
  assign x = 1'bx;
  assign z = 1'bz;
  #10;

  if (t0 !== z)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:z", gnd, gnd, t0 );
  end

  if (t1 !== 1)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:1", gnd, vdd, t1 );
  end
  if (t2 !== StH)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:StH", gnd, x, t2 );
  end
  if (t3 !== StH)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:StH", gnd, z, t3 );
  end

  if (t4 !== 1'bz)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:z", vdd, gnd, t4 );
  end
  if (t5 !== 0)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:0", vdd, vdd, t5 );
  end
  if (t6 !== StL)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:StL", vdd, x, t6 );
  end
  if (t7 !== StL)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:StL", vdd, z, t7 );
  end

  if (t8 !== 1'bz)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:z", x, gnd, t8 );
  end
  if (t9 !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", x, vdd, t9 );
  end
  if (ta !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", x, x, ta );
  end
  if (tb !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", x, z, tb );
  end

  if (tc !== 1'bz)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:z", z, gnd, tc );
  end
  if (td !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", z, vdd, td );
  end
  if (te !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", z, x, te );
  end
  if (tf !== 1'bx)
  begin
      failed = 1;
      $display ("FAILED: notif1 s:%d g:%d d:%d expected:x", z, z, tf );
  end

  if (failed == 0)
      $display ("PASSED");
end
endmodule
