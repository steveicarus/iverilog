module signed_assignment_bug();

reg   [7:0] a;
wire [15:0] y;
wire [15:0] z;

initial begin
    // Example vector
    a = 8'b10110110;

    // Wait for results to be calculated
    #1;

    // Display results
    $display("a = %b", a);
    $display("y = %b", y);
    $display("z = %b", z);

    // Finished
    $finish(0);
end

// Calculate signed logical OR
manually_extended_assignment INST1(.a(a), .y(y));
signed_assignment            INST2(.a(a), .y(z));

endmodule

module manually_extended_assignment(a, y);

input   [7:0] a;
output [15:0] y;

// Manually sign extend before assignment
assign y = {{8{a[7]}}, a};

endmodule

module signed_assignment(a, y);

input   [7:0] a;
output [15:0] y;

// $signed() sign extends before assignment
assign y = $signed(a);

endmodule
