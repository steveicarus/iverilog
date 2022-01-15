module test();

struct packed {
  logic [15:0] value;
} data;

initial begin
  data.value[7:0] = 8'h55;
  data.value[15:8] = 8'haa;
  if (data !== 16'haa55) begin
    $display("FAILED -- data=%h", data);
    $finish;
  end
  $display("PASSED");
end

endmodule
