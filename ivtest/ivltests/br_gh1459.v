module test;
reg passed;

string a = "A";
string b = "B";

task t(input int verbosity);

    string color;
    color = (verbosity >= 3) ? a : b;

    if ((verbosity >= 3) && (color != "A")) begin
	$display("Failed, expected 'A', got '%s'", color);
	passed = 1'b0;
    end
    if ((verbosity < 3) && (color != "B")) begin
	$display("Failed, expected 'B', got '%s'", color);
	passed = 1'b0;
    end

endtask

initial begin
    passed = 1'b1;

    t(1);
    t(3);

    if (passed) $display("PASSED");
end

endmodule
