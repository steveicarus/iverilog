module top;
  initial begin
    #(32'hf0000000);
    $display("$simtime: ",$simtime, ", $time: ",$time, ", $stime: ",$stime);
    #(32'h10000000);
    $display("$simtime: ",$simtime, ", $time: ",$time, ", $stime: ",$stime);
    #(32'hf0000000);
    $display("$simtime: ",$simtime, ", $time: ",$time, ", $stime: ",$stime);
  end
endmodule
