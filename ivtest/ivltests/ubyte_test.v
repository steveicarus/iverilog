// Ten basic tests in here:
// 1. byte must be initialised before any initial or always block
// 2. assignments to (unsigned) bytes with random numbers
// 3. assignments to (unsigned) bytes with random values including X and Z
// 4. converting unsigned integers to unsigned bytes
// 5. converting signed integers to unsigned bytes
// 6. converting integers including X and Z states to unsigned bytes
// 7. trying unsigned sums (procedural, function, task and module)
// 8. trying unsigned (truncated) mults (procedural, function and task)
// 9. trying relational operators
// 10. smaller signed number to unsigned bytes (sign extension)

module mu_add (input byte unsigned a, b, output byte unsigned sc, ss);
   assign sc = a + b;
   always @(a, b) ss = a + b;
endmodule


module main;
  parameter N_REPS = 500;                 // repetition with random numbers
  parameter XZ_REPS = 500;                // repetition with 'x 'z values
  parameter MAX = 256;
  parameter LEN = 8;
  // variables used as golden references
  reg unsigned [LEN-1:0] ar;              // holds numbers
  reg unsigned [LEN-1:0] ar_xz;           // holds 'x and/or 'z in random positions
  reg unsigned [LEN-1:0] ar_expected;
  integer unsigned       ui;
  integer signed         si;
  reg signed [LEN/2-1:0]       slice;
  // types to be tested
  byte unsigned bu;                       // holds numbers
  byte unsigned bu_xz;                    // 'x and 'z are attempted on this
  byte unsigned bresult;                  // hold results from sums and mults
  byte unsigned mcaresult;                // this is permanently wired to a module
  byte unsigned mabresult;                // this is permanently wired to a module


  integer i;

  // continuous assigments
  // type LHS      type RHS
  // ---------     ---------
  // byte          4-value logic
  assign bu = ar;
  assign bu_xz = ar_xz;

  // module instantiation
  mu_add duv (.a(bu), .b(bu_xz), .sc(mcaresult), .ss(mabresult) );

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if ( bu !== 8'b0 || bu_xz !== 8'b0 || bresult !== 8'b0 || mcaresult !== 8'b0 || mabresult !== 8'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // driving byte type with unsigned random numbers from a variable
    for (i = 0; i< N_REPS; i = i+1)
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
    // attempting to drive variables having 'x 'z values into type unsigned bytes
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to byte (when 'x 'z): %b", bu);
            $finish;
        end
      end
    // converting unsigned integers to unsigned bytes
    // truncation expected (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ui = {$random};
        #1;
        force bu = ui;
        #1;
        if (bu !== ui[LEN-1:0])
          begin
            $display ("FAILED - incorrect truncation from unsigned integer to byte: %b", bu);
            $finish;
        end
      end
    release bu;
    // converting signed integers to unsigned bytes
    // truncation expected (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        si = $random % MAX/2;
        #1;
        force bu = si;
        #1;
        if (bu !== si[LEN-1:0])
          begin
            $display ("FAILED - incorrect truncation from signed integer to byte: %b mismatchs %b", bu, si[7:0]);
            $finish;
        end
      end
    release bu;
    // converting signed integers having 'x 'z values into type unsigned bytes
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    // truncation and coercion to zero expected
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        si = $random;
        ar_xz = xz_inject (si[LEN-1:0]);
        si = {si[31:LEN], ar_xz};
        ar_expected = xz_expected (ar_xz);
        #1;
        force bu_xz = si;
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect conversion from integer (with 'x 'z) to byte: %b mismatchs %b", bu_xz, ar_expected);
            $finish;
        end
      end
    release bu_xz;
    // trying unsigned sums
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        ar_xz = {$random} % MAX;
        #1;
        bresult = bu + bu_xz;
        #1;
        if ( bresult !== u_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned bytes: %0d mismatchs %0d", bresult, u_sum(ar, ar_xz));
            $finish;
          end
        // invoking byte sum function
        if ( fu_sum (bu, bu_xz) !== u_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned bytes in function");
            $finish;
          end
        // invoking byte sum task
        tu_sum (bu, bu_xz, bresult);
        if ( bresult !== u_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned bytes in task: %0d mismatchs %0d", bresult, u_sum(ar, ar_xz));
            $finish;
          end
        // checking byte sum from module
        if ( mcaresult !== u_sum(ar, ar_xz) || mabresult !== u_sum(ar, ar_xz))
          begin
            $display ("FAILED - incorrect addition of unsigned bytes from module");
            $finish;
          end
      end
    // trying unsigned mults: trucation is forced
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = ({$random} % MAX) << LEN/2;
        ar_xz = ({$random} % MAX) << (LEN/2 -1);
        #1;
        bresult = bu * bu_xz;
        #1;
        if ( bresult !== u_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned bytes: %0d mismatchs %0d", bresult, u_mul(ar, ar_xz));
            $finish;
          end
        // invoking byte mult function
        if ( fu_mul (bu, bu_xz) !== u_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect product of unsigned bytes in function");
            $finish;
          end
        // invoking byte mult task
        tu_mul (bu, bu_xz, bresult);
        if ( bresult !== u_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect product of unsigned bytes in task: %0d mismatchs %0d", bresult, u_mul(ar, ar_xz));
            $finish;
          end
      end
    // trying relational operators
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        ar_xz = {$random} % MAX;
        #1;
        if ( (bu < bu_xz ) !== (ar < ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than' on unsigned bytes");
            $finish;
          end
        if ( (bu <= bu_xz ) !== (ar <= ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than or equal' on unsigned bytes");
            $finish;
          end
        if ( (bu > bu_xz ) !== (ar > ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than' on unsigned bytes");
            $finish;
          end
        if ( (bu >= bu_xz ) !== (ar >= ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than or equal' than on unsigned bytes");
            $finish;
          end
        if ( (bu == bu_xz ) !== (ar == ar_xz) )
          begin
            $display ("FAILED - incorrect 'equal to' on unsigned bytes");
            $finish;
          end
        if ( (bu != bu_xz ) !== (ar != ar_xz) )
          begin
            $display ("FAILED - incorrect 'not equal to' on unsigned bytes");
            $finish;
          end
      end
    # 1;
    // signed small number to unsigned byte
    for (i = 0; i < (1<<LEN/2); i = i+1)
      begin
        #1;
        slice = $random % 'h7;
        force bu = slice;
        ar = slice;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect signed extend to unsigned bytes");
            $finish;
          end
      end
    release bu;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [LEN-1:0] xz_inject (input unsigned [LEN-1:0] value);
      integer i, temp;
      begin
        temp = {$random} % MAX;
        for (i=0; i<LEN; i=i+1)
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
  function [LEN-1:0] xz_expected (input unsigned [LEN-1:0] value_xz);
      integer i;
      begin
        for (i=0; i<LEN; i=i+1)
          begin
             if (value_xz[i] === 1'bx || value_xz[i] === 1'bz )
                 value_xz[i] = 1'b0;  // forced to zero
          end
          xz_expected = value_xz;
      end
  endfunction

  // unsigned 4-value sum
  function unsigned [LEN-1:0] u_sum (input unsigned [LEN-1:0] a, b);
      u_sum = a + b;
  endfunction

  // unsigned byte sum as function
  function byte unsigned fu_sum (input byte unsigned a, b);
      fu_sum = a + b;
  endfunction

  // unsigned byte sum as task
  task tu_sum (input byte unsigned a, b, output byte unsigned c);
      c = a + b;
  endtask

   // unsigned 4-value mults
  function unsigned [LEN-1:0] u_mul (input unsigned [LEN-1:0] a, b);
      u_mul = a * b;
  endfunction

  // unsigned byte mults
  function byte unsigned fu_mul (input byte unsigned a, b);
      fu_mul = a * b;
  endfunction

  // unsigned byte mult as task
  task tu_mul (input byte unsigned a, b, output byte unsigned c);
      c = a * b;
  endtask


endmodule
