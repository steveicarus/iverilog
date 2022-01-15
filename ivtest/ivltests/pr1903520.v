module test();
wire [1:0] b;

assign b[0] = 0;

a a(~b[0]);

endmodule

module a(b);
input b;
initial #1 if (b) $display("PASSED"); else $display("FAILED");
endmodule
