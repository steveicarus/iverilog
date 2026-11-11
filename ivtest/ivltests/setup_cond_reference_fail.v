// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module nested_cond;wire E;endmodule
module tb(C,D,E,R);
 input C,D,E,R;wire [1:0] V;wire A[0:1];parameter P=1;reg n;
 nested_cond nested();
 specify
  $setup(D, posedge C &&& (E && R),10,n);
  $setup(D, posedge C &&& (E == 1'b1),10,n);
  $setup(D, posedge C &&& (E != 1'b0),10,n);
  $setup(D, posedge C &&& (E !== 1'b0),10,n);
  $setup(D, posedge C &&& (1'b1 === E),10,n);
  $setup(D, posedge C &&& (!E),10,n);
  $setup(D, posedge C &&& V,10,n);
  $setup(D, posedge C &&& V[0],10,n);
  $setup(D, posedge C &&& nested.E,10,n);
  $setup(D, posedge C &&& P,10,n);
  $setup(D, posedge C &&& (E === 1'bx),10,n);
  $setup(D, posedge C &&& (E === 1'bz),10,n);
  $setup(D, posedge C &&& (E === 1),10,n);
  $setup(D, posedge C &&& (E === 2'b01),10,n);
  $setup(D, posedge C &&& (E === 1'sb1),10,n);
  $setup(D, posedge C &&& 1'b1,10,n);
  $setup(D, posedge C &&& A[0],10,n);
 endspecify
endmodule
