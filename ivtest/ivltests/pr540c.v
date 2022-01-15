module top;

  integer fail;
  reg cmd, reset;

  initial begin
    #1;
    reset = 0;
    fail = 0;
    #1;
    cmd = 0;
    #2;
    reset = 1;
    #2;
    cmd = 1;
    #2;
    cmd = 0;
    #2;
    reset = 0;
    #2;
    reset = 1;
    #4;
    if(fail) $display("***** disable test FAILED *****");
    else     $display("***** disable test PASSED *****");
    $finish(0);
  end

  always @(cmd) begin: command_block
    fork
      begin
        #0; // avoid fork race
	disable command_block_reset;
      end
      begin: command_block_reset
	@(reset);
	fail = 1;
	disable command_block;
      end
    join
  end

endmodule
