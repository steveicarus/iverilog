module top;
  initial fork : named_begin
    $display("FAILED");
  join : wrong_name
endmodule
