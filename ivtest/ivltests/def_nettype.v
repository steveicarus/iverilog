module all;
  reg pass;

  task automatic check;
    input sig;
    input val;
    input [32*8:1] name;
    begin
      if (sig !== val) begin
        $display("FAILED \"%0s\", expected %b, got %b", name, val, sig);
        pass = 1'b0;
      end
    end
  endtask

  initial begin
    pass = 1'b1;
    #100;
    if (pass) $display("PASSED");
  end
endmodule

/* Check the wire net type. */
`default_nettype wire
module top_wire;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wire(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "wire(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wire(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "wire(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "wire(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wire(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wire(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "wire(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "wire(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "wire(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wire(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "wire(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wire(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wire(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wire(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "wire(z,z)");
  end
endmodule

/* Check the tri net type (should be identical to wire). */
`default_nettype tri
module top_tri;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "tri(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "tri(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "tri(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "tri(z,z)");
  end
endmodule

/* Check the tri0 net type (should be the same as tri except z,z is 0). */
`default_nettype tri0
module top_tri0;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri0(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri0(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri0(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "tri0(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri0(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri0(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri0(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "tri0(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri0(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri0(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri0(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "tri0(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri0(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri0(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri0(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "tri0(z,z)");
  end
endmodule

/* Check the tri1 net type (should be the same as tri except z,z is 1). */
`default_nettype tri1
module top_tri1;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri1(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri1(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri1(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "tri1(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri1(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri1(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri1(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "tri1(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "tri1(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "tri1(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri1(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "tri1(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "tri1(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "tri1(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "tri1(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "tri1(z,z)");
  end
endmodule

/* Check the wand net type. */
`default_nettype wand
module top_wand;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wand(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'b0, "wand(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'b0, "wand(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "wand(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wand(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wand(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wand(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "wand(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wand(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "wand(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wand(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "wand(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wand(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wand(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wand(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "wand(z,z)");
  end
endmodule

/* Check the triand net type (should be the same as wand). */
`default_nettype triand
module top_triand;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "triand(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'b0, "triand(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'b0, "triand(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "triand(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "triand(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "triand(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "triand(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "triand(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "triand(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'bx, "triand(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "triand(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "triand(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "triand(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "triand(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "triand(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "triand(z,z)");
  end
endmodule

/* Check the wor net type. */
`default_nettype wor
module top_wor;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wor(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wor(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wor(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "wor(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'b1, "wor(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wor(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'b1, "wor(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "wor(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "wor(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wor(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wor(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "wor(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "wor(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "wor(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "wor(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "wor(z,z)");
  end
endmodule

/* Check the trior net type (should be the same as wor). */
`default_nettype trior
module top_trior;
  reg in0, in1;

  assign tmp = in0;
  assign tmp = in1;

  initial begin
    in0 = 1'b0; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "trior(0,0)");
    in0 = 1'b0; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "trior(0,1)");
    in0 = 1'b0; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "trior(0,x)");
    in0 = 1'b0; in1 = 1'bz;
    #1 all.check(tmp, 1'b0, "trior(0,z)");

    in0 = 1'b1; in1 = 1'b0;
    #1 all.check(tmp, 1'b1, "trior(1,0)");
    in0 = 1'b1; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "trior(1,1)");
    in0 = 1'b1; in1 = 1'bx;
    #1 all.check(tmp, 1'b1, "trior(1,x)");
    in0 = 1'b1; in1 = 1'bz;
    #1 all.check(tmp, 1'b1, "trior(1,z)");

    in0 = 1'bx; in1 = 1'b0;
    #1 all.check(tmp, 1'bx, "trior(x,0)");
    in0 = 1'bx; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "trior(x,1)");
    in0 = 1'bx; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "trior(x,x)");
    in0 = 1'bx; in1 = 1'bz;
    #1 all.check(tmp, 1'bx, "trior(x,z)");

    in0 = 1'bz; in1 = 1'b0;
    #1 all.check(tmp, 1'b0, "trior(z,0)");
    in0 = 1'bz; in1 = 1'b1;
    #1 all.check(tmp, 1'b1, "trior(z,1)");
    in0 = 1'bz; in1 = 1'bx;
    #1 all.check(tmp, 1'bx, "trior(z,x)");
    in0 = 1'bz; in1 = 1'bz;
    #1 all.check(tmp, 1'bz, "trior(z,z)");
  end
endmodule
