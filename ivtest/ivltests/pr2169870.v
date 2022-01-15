// Copyright 2008, Martin Whitaker.
// This file may be copied freely for any purpose. No attribution required.

module pr2169870();

task automatic count;

integer i;

begin
  i = 0;
  while (i < 10) begin
    #1 $display("%0d", i);
    i = i + 1;
  end
end

endtask

initial count;

initial count;

endmodule
