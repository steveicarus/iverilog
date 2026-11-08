// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module nested_cond;wire E;endmodule
module tb(C,D,E,R);
 input C,D,E,R;wire [1:0] V;wire A[0:1];parameter P=1;reg n;
 nested_cond nested();
 specify
  $hold(posedge C, D &&& (E && R),10,n);
  $hold(posedge C, D &&& (E == 1'b1),10,n);
  $hold(posedge C, D &&& (E != 1'b0),10,n);
  $hold(posedge C, D &&& (E !== 1'b0),10,n);
  $hold(posedge C, D &&& (1'b1 === E),10,n);
  $hold(posedge C, D &&& (!E),10,n);
  $hold(posedge C, D &&& V,10,n);
  $hold(posedge C, D &&& V[0],10,n);
  $hold(posedge C, D &&& nested.E,10,n);
  $hold(posedge C, D &&& P,10,n);
  $hold(posedge C, D &&& (E === 1'bx),10,n);
  $hold(posedge C, D &&& (E === 1'bz),10,n);
  $hold(posedge C, D &&& (E === 1),10,n);
  $hold(posedge C, D &&& (E === 2'b01),10,n);
  $hold(posedge C, D &&& (E === 1'sb1),10,n);
  $hold(posedge C, D &&& 1'b1,10,n);
  $hold(posedge C, D &&& A[0],10,n);
 endspecify
endmodule
