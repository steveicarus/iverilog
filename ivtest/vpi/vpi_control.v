module main();

initial begin
    $test_control;
    $display("Error: simulation should have finished");
    $display("FAILED");
end

endmodule
