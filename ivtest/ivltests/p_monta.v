// From: Peter Monta <pmonta@pmonta.com>
// Subject: verilog: vvp bug, function or concat related?
// Message-Id: <20010726071414.1CEF41C5@www.pmonta.com>
// Date: Thu, 26 Jul 2001 00:14:14 -0700 (PDT)

module main();

function [7:0] f;
  input [7:0] r;
  f = {
    r[0]^r[1]^r[2]^r[3]^r[7],
    r[3]^r[6]^r[7],
    r[2]^r[5]^r[6],
    r[1]^r[4]^r[5]^r[7],
    r[0]^r[3]^r[4]^r[6]^r[7],
    r[0]^r[1]^r[5]^r[6],
    r[1]^r[2]^r[3]^r[4]^r[5],
    r[0]^r[1]^r[2]^r[3]^r[4] };
endfunction

  reg [7:0] data_in;
  reg [7:0] r;
  reg start_in;

  initial begin
    data_in = 8'h23;
    r = 0;
    start_in = 0;
    #2;
    r <= #1 start_in ? 0 : f(data_in);
    #2;
    $display("%b",r);
     if (r === 8'b00101100)
       $display("PASSED");
     else
       $display("FAILED");
  end

endmodule
