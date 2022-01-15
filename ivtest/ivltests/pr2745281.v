`define MERROR(code, msg) if (code == 0)  begin $display(msg); end

module top;
  integer return_code;
  integer msg_out;

  initial begin
    // This shows that the macro is okay for the simple case
    `MERROR(0, "This message works")

    // This one gives a syntax error.
    `MERROR($value$plusargs("msgOut=%d", msg_out), "This message does not work")

    // This was a workaround
    return_code = $value$plusargs("msgOut=%d", msg_out);
    `MERROR(return_code, "This last message works")

    $display("PASSED");
  end
endmodule
