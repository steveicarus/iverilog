`define typ(n) n

module br_gh1323_top #(parameter `typ(X) = 1) ();

br_gh1323_lib #(X) u0();

endmodule
