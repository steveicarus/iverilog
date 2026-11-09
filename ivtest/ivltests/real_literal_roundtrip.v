// Check binary64 values after native compilation and vlog95 round trips.
// Expected bits come from exact decimal rational rounding, independently of
// the translator. Vector expectations cannot undergo real-literal trimming.

module test;
  integer errors;

  task check;
    input real value;
    input [63:0] expected;
    begin
      if ($realtobits(value) !== expected) begin
        $display("FAILED: expected %h, got %h", expected, $realtobits(value));
        errors = errors + 1;
      end
    end
  endtask

  task check_nan;
    input real value;
    reg [63:0] bits;
    begin
      bits = $realtobits(value);
      if (bits[62:52] !== 11'h7ff || bits[51:0] === 52'b0) begin
        $display("FAILED: expected NaN, got %h", bits);
        errors = errors + 1;
      end
    end
  endtask

  initial begin
    errors = 0;
    check(1e10, 64'h4202a05f20000000);
    check(-1e10, 64'hc202a05f20000000);
    check(1e20, 64'h4415af1d78b58c40);
    check(-1e20, 64'hc415af1d78b58c40);
    check(1e30, 64'h46293e5939a08cea);
    check(-1e30, 64'hc6293e5939a08cea);
    check(1e31, 64'h465f8def8808b024);
    check(-1e31, 64'hc65f8def8808b024);
    check(1e40, 64'h483d6329f1c35ca5);
    check(-1e40, 64'hc83d6329f1c35ca5);
    check(1e100, 64'h54b249ad2594c37d);
    check(-1e100, 64'hd4b249ad2594c37d);
    check(1e300, 64'h7e37e43c8800759c);
    check(-1e300, 64'hfe37e43c8800759c);
    check(1e-10, 64'h3ddb7cdfd9d7bdbb);
    check(-1e-10, 64'hbddb7cdfd9d7bdbb);
    check(1e-20, 64'h3bc79ca10c924223);
    check(-1e-20, 64'hbbc79ca10c924223);
    check(1e-30, 64'h39b4484bfeebc2a0);
    check(-1e-30, 64'hb9b4484bfeebc2a0);
    check(1e-40, 64'h37a16c262777579c);
    check(-1e-40, 64'hb7a16c262777579c);
    check(1e-100, 64'h2b2bff2ee48e0530);
    check(-1e-100, 64'hab2bff2ee48e0530);
    check(1e-300, 64'h01a56e1fc2f8f359);
    check(-1e-300, 64'h81a56e1fc2f8f359);
    check(0.0, 64'h0000000000000000);
    check(-0.0, 64'h8000000000000000);
    check(1.0, 64'h3ff0000000000000);
    check(-1.0, 64'hbff0000000000000);
    check(42.0, 64'h4045000000000000);
    check(1.25, 64'h3ff4000000000000);
    check(-0.125, 64'hbfc0000000000000);
    check(1.2345678901234567, 64'h3ff3c0ca428c59fb);
    check(1e9, 64'h41cdcd6500000000);
    check(1e-9, 64'h3e112e0be826d695);
    check(1.0/0.0, 64'h7ff0000000000000);
    check(-1.0/0.0, 64'hfff0000000000000);
    check_nan(0.0/0.0);
    if (errors == 0) $display("PASSED");
  end
endmodule
