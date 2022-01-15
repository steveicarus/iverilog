module top;
  int mbx[$];

  initial begin
    $display("mbx.size() == %0d", mbx.size());
    wait(mbx.size());
    $display("mbx.size() == %0d", mbx.size());
    $display("PASSED");
  end

  initial begin
     #100 $display ("Push an item");
     mbx.push_back(1);
  end
endmodule

