module automatic_static_block_unsupported;
`ifdef DETACHED
  initial begin
    automatic int value = 1;
    fork
      #1 $display("%0d", value);
    join_none
  end
`elsif NESTED
  initial begin
    automatic int outer = 1;
    begin
      automatic int inner = outer;
      $display("%0d", inner);
    end
  end
`elsif ARRAY
  initial begin
    automatic int values [0:1];
    values[0] = 1;
  end
`elsif FUNCTION
  function int f();
    begin
      automatic int value = 1;
      f = value;
    end
  endfunction
  localparam value = f();
`elsif NBA
  initial begin
    automatic int value = 1;
    value <= 2;
  end
`elsif EVENT
  initial begin
    automatic int value = 1;
    event ready;
    -> ready;
  end
`else
  initial begin : named_block
    automatic int value = 1;
    #1;
  end
  initial $display("%0d", named_block.value);
`endif
endmodule
