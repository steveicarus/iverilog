module test2 ();

    generate
    begin : a
        reg b;
    end
    endgenerate

  initial begin
    a.b = 1'b1;
    if (a.b) $display("PASSED");
    else $display("FAILED");
  end
endmodule
