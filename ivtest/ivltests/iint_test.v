// Three basic tests in here:
// 1. byte must be initialised before any initial or always block
// 2. assignments to (unsigned) bytes with random numbers
// 3. assignments to (unsigned) bytes with random values including X and Z


module ibyte_test;
  parameter TRIALS = 100;
  parameter MAX = 'h7fffffff;
  reg [31:0] ar;              // should it be "reg unsigned [7:0] aw"?
  reg [31:0] ar_xz;           // same as above here?
  reg [31:0] ar_expected;     // and here
  int unsigned bu;
  int unsigned bu_xz;

  integer i;

  assign bu = ar;
  assign bu_xz = ar_xz;

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if (bu !== 32'b0 | bu_xz != 32'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // random numbers
    for (i = 0; i< TRIALS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect assigment to byte: %b", bu);
            $finish;
        end
      end
    # 1;
    // with 'x injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< TRIALS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to byte (when 'x): %b", bu);
            $finish;
        end
      end
    # 1;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [31:0] xz_inject (input [31:0] value); // should it be "input unsigned [7:0]" instead?
      integer i, temp;
      begin
        temp = {$random} % MAX;
        for (i=0; i<32; i=i+1)
          begin
             if (temp[i] == 1'b1)
               begin
                 temp = $random % MAX;
                 if (temp <= 0)
                    value[i] = 1'bx;  // 'x noise
                 else
                    value[i] = 1'bz;  // 'z noise
               end
          end
          xz_inject = value;
      end
  endfunction

  // this function returns bit positions with either X or Z to 0 for an input value
  function [31:0] xz_expected (input [31:0] value_xz); // should it be "input unsigned [7:0] instead?
      integer i;
      begin
        for (i=0; i<32; i=i+1)
          begin
             if (value_xz[i] === 1'bx || value_xz[i] === 1'bz )
                 value_xz[i] = 1'b0;  // forced to zero
          end
          xz_expected = value_xz;
      end
  endfunction


endmodule
