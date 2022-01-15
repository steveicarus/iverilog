module top;
  reg passed;
  reg pevt;
  reg evt;
  reg pedge;
  reg nedge;

  initial begin
    passed = 1'b1;

    #1; // Check X to 0
    {pedge, nedge} = 2'b01;
    evt = 1'b0;

    #1; // Check 0 to X
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'bx;

    #1; // Check X to 1
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'b1;

    #1; // Check 1 to X
    pevt = evt;
    {pedge, nedge} = 2'b01;
    evt = 1'bx;

    #1; // Check X to Z
    pevt = evt;
    {pedge, nedge} = 2'b00;
    evt = 1'bz;

    #1; // Check Z to X
    pevt = evt;
    {pedge, nedge} = 2'b00;
    evt = 1'bx;

    #1; // Check X to Z (again)
    pevt = evt;
    {pedge, nedge} = 2'b00;
    evt = 1'bz;

    #1; // Check Z to 0
    pevt = evt;
    {pedge, nedge} = 2'b01;
    evt = 1'b0;

    #1; // Check 0 to Z
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'bz;

    #1; // Check Z to 1
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'b1;

    #1; // Check 1 to Z
    pevt = evt;
    {pedge, nedge} = 2'b01;
    evt = 1'bz;

    #1; // Check Z to 1 (again)
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'b1;

    #1; // Check 1 to 0
    pevt = evt;
    {pedge, nedge} = 2'b01;
    evt = 1'b0;

    #1; // Check 0 to 1
    pevt = evt;
    {pedge, nedge} = 2'b10;
    evt = 1'b1;

    #1;

    if (passed) $display("PASSED");
  end

  always @(posedge evt) begin
    if (!pedge) begin
      $display("Error: posedge detected for %b -> %b", pevt, evt);
      passed = 1'b0;
    end
  end

  always @(negedge evt) begin
    if (!nedge) begin
      $display("Error: negedge detected for %b -> %b", pevt, evt);
      passed = 1'b0;
    end
  end

  always @(edge evt) begin
    if (!nedge && !pedge) begin
      $display("Error: edge detected for %b -> %b", pevt, evt);
      passed = 1'b0;
    end
  end

  always @(evt)
    $display("Checking the %b -> %b event", pevt, evt);
endmodule
