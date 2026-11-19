// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module nested_cond;wire E;endmodule
module tb(C,D,E,R);
 input C,D,E,R;wire [1:0] V;wire A[0:1];parameter P=1;reg n;
 nested_cond nested();
 specify
  $hold(posedge C &&& (E && R), D,10,n);
  $hold(posedge C &&& (E == 1'b1), D,10,n);
  $hold(posedge C &&& (E != 1'b0), D,10,n);
  $hold(posedge C &&& (E !== 1'b0), D,10,n);
  $hold(posedge C &&& (1'b1 === E), D,10,n);
  $hold(posedge C &&& (!E), D,10,n);
  $hold(posedge C &&& V, D,10,n);
  $hold(posedge C &&& V[0], D,10,n);
  $hold(posedge C &&& nested.E, D,10,n);
  $hold(posedge C &&& P, D,10,n);
  $hold(posedge C &&& (E === 1'bx), D,10,n);
  $hold(posedge C &&& (E === 1'bz), D,10,n);
  $hold(posedge C &&& (E === 1), D,10,n);
  $hold(posedge C &&& (E === 2'b01), D,10,n);
  $hold(posedge C &&& (E === 1'sb1), D,10,n);
  $hold(posedge C &&& 1'b1, D,10,n);
  $hold(posedge C &&& A[0], D,10,n);
 endspecify
endmodule
