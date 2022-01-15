// Note: The for is translated to a begin/while is it tests the while.

module main;
  reg val = 1'b0;
  reg cond = 1'b1;
  reg [1:0] cval;
  integer idx;
  integer dly = 1;

  // Simple assign (error).
  always val = 1'b1;

  // A zero delay assign (error).
  always #0 val = 1'b1;

  // A variable delay assign (warning).
  always #dly val = 1'b1;

  // Non-blocking assign (error).
  always val <= #1 1'b1;

  // No delay if (error).
  always if (cond) val = 1'b1;

  // No delay if/else (error).
  always if (cond) val = 1'b1; else val = 1'b0;

  // Delay if/no delay else (warning).
  always if (cond) #1 val = 1'b1; else val = 1'b0;

  // Delay if/no delay else (warning).
  always #0 if (cond) #1 val = 1'b1; else val = 1'b0;

  // No delay if/delay else (warning).
  always if (cond) val = 1'b1; else #1 val = 1'b0;

  // No delay forever (error).
  always forever val = 1'b1;

  // Zero delay forever (error).
  always forever #0 val = 1'b1;

  // Possible delay forever (warning).
  always forever if (cond) #1 val = 1'b1; else val = 1'b0;

  // No delay for (error).
  always for(idx=0; idx<1; idx=idx+1) val = 1'b1;

  // Zero delay for (error).
  always for(idx=0; idx<1; idx=idx+1) #0 val = 1'b1;

  // Possible delay for (warning).
  always for(idx=0; idx<1; idx=idx+1) if (cond) #1 val = 1'b1; else val = 1'b0;

  // Never run for (error).
  always for(idx=0; 0; idx=idx+1) #1 val = 1'b1;

  // Always run for (error).
  always for(idx=0; 1; idx=idx+1) #0 val = 1'b1;

  // An empty bock (error).
  always begin end

  // Block with no delay (error).
  always begin val = 1'b1; end

  // Block with zero delay (error).
  always begin #0 val = 1'b1; end

  // Block with zero delay (warning).
  always begin #0; if (cond) #1 val = 1'b1; else val = 1'b0; end

  // Never run repeat (error).
  always repeat(0) #1 val = 1'b1;

  // Always run repeat (error).
  always repeat(1) #0 val = 1'b1;

  // Possibly run repeat (warning).
  always repeat(cond) #1 val = 1'b1;

  // No wait (error).
  always wait(1) val = 1'b1;

  // May wait (warning).
  always wait(cond) val = 1'b1;

  // Not all paths covered (warning).
  always case(cval)
    2'b00: #1 val = 1'b1;
    2'b10: #1 val = 1'b1;
  endcase

  // Not all paths have delay (warning).
  always case(cval)
    2'b00: #1 val = 1'b1;
    2'b10: #1 val = 1'b1;
    default: #0 val = 1'b1;
  endcase

  // Check task calls (error, error, warning).
  always no_delay;
  always zero_delay;
  always possible_delay;

  task no_delay;
    val = 1'b1;
  endtask

  task zero_delay;
    #0 val = 1'b1;
  endtask

  task possible_delay;
    #dly val = 1'b1;
  endtask

  // Check a function call (error).
  always val = func(1'b1);

  function func;
    input in;
    func = in;
  endfunction

endmodule
