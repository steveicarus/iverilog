module dut(output logic [7:0] op[1:0]);

assign op[0] = 8'd1;
assign op[1] = 8'd2;

endmodule

module test();

logic [7:0] v[1:0];

dut dut(v);

initial begin
  #0 $display("%b %b", v[0], v[1]);
  if ((v[0] === 8'd1) && (v[1] === 8'd2))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
