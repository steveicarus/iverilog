// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module nested_cond;wire E;endmodule
module tb(R,C,E,G);
 input R,C,E,G;wire [1:0] V;wire A[0:1];parameter P=1;reg n;
 nested_cond nested();
 specify
  $recrem(posedge R &&& (E && G), posedge C,10,6,n);
  $recrem(posedge R &&& (E == 1'b1), posedge C,10,6,n);
  $recrem(posedge R &&& (E != 1'b0), posedge C,10,6,n);
  $recrem(posedge R &&& (E !== 1'b0), posedge C,10,6,n);
  $recrem(posedge R &&& (1'b1 === E), posedge C,10,6,n);
  $recrem(posedge R &&& (!E), posedge C,10,6,n);
  $recrem(posedge R &&& V, posedge C,10,6,n);
  $recrem(posedge R &&& V[0], posedge C,10,6,n);
  $recrem(posedge R &&& nested.E, posedge C,10,6,n);
  $recrem(posedge R &&& P, posedge C,10,6,n);
  $recrem(posedge R &&& (E === 1'bx), posedge C,10,6,n);
  $recrem(posedge R &&& (E === 1'bz), posedge C,10,6,n);
  $recrem(posedge R &&& (E === 1), posedge C,10,6,n);
  $recrem(posedge R &&& (E === 2'b01), posedge C,10,6,n);
  $recrem(posedge R &&& (E === 1'sb1), posedge C,10,6,n);
  $recrem(posedge R &&& 1'b1, posedge C,10,6,n);
  $recrem(posedge R &&& A[0], posedge C,10,6,n);
 endspecify
endmodule
