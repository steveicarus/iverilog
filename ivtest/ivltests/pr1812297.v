module ttop;
  reg tpass = 0, fpass = 0;
  task ttop;
    #1 tpass = 1;
  endtask
  initial begin
    ttop;
    #2;
    case ({tpass, fpass})
      2'b00: $display("FAILED - both task and function test");
      2'b01: $display("FAILED - task test");
      2'b10: $display("FAILED - function test");
      2'b11: $display("PASSED");
    endcase
  end
endmodule

module ftop;
  function ftop;
    input a;
    ftop = ~a;
  endfunction

  initial if (ftop(0)) #1 ttop.fpass = 1;
endmodule
