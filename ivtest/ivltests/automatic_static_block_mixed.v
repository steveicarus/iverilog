module automatic_static_block_mixed;
  initial begin
    static int retained = 0;
    automatic int fresh = 0;
    retained = retained + fresh;
  end
endmodule
