// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module nested_cond;wire E;endmodule
module tb(C,D,E,R);
 input C,D,E,R;wire [1:0] V;wire A[0:1];parameter P=1;reg n;
 nested_cond nested();
 specify
  $setup(D &&& (E && R), posedge C,10,n);
  $setup(D &&& (E == 1'b1), posedge C,10,n);
  $setup(D &&& (E != 1'b0), posedge C,10,n);
  $setup(D &&& (E !== 1'b0), posedge C,10,n);
  $setup(D &&& (1'b1 === E), posedge C,10,n);
  $setup(D &&& (!E), posedge C,10,n);
  $setup(D &&& V, posedge C,10,n);
  $setup(D &&& V[0], posedge C,10,n);
  $setup(D &&& nested.E, posedge C,10,n);
  $setup(D &&& P, posedge C,10,n);
  $setup(D &&& (E === 1'bx), posedge C,10,n);
  $setup(D &&& (E === 1'bz), posedge C,10,n);
  $setup(D &&& (E === 1), posedge C,10,n);
  $setup(D &&& (E === 2'b01), posedge C,10,n);
  $setup(D &&& (E === 1'sb1), posedge C,10,n);
  $setup(D &&& 1'b1, posedge C,10,n);
  $setup(D &&& A[0], posedge C,10,n);
 endspecify
endmodule
