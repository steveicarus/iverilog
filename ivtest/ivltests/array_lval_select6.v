// tests using array elements as indices/selects in an array lval select
`timescale 1ns/100ps
module tb;

reg [7:0] a[7:0];
real r[7:0];
wire [2:0] idx[7:0];

genvar g;
for (g = 0; g < 8; g=g+1)
   assign idx[g] = g;

reg pass;
integer i;
initial begin
   pass = 1'b1;

   // zero everything out
   for (i = 0; i < 8; i = i + 1) begin
      a[i] = 8'h0;
      r[i] = 0.0;
   end

   // test using one in a part select
   a[1][idx[1]*4 +: 4] = 4'ha;
   if (a[1] != 8'ha0) begin
      $display("FAILED part select, expected a0, got %x", a[1]);
      pass = 1'b0;
   end

   // test using one in an index
   a[idx[2]] = 8'hbc;
   if (a[2] != 8'hbc) begin
      $display("FAILED word index, expected bc, got %x", a[2]);
      pass = 1'b0;
   end

   // and now both...
   a[idx[3]][idx[0]*4 +: 4] = 4'hd;
   if (a[3] != 8'h0d) begin
      $display("FAILED word index and part select, expected 0d, got %x", a[3]);
      pass = 1'b0;
   end

   // non-blocking, in part select
   a[4][idx[1]*4 +: 4] <= 4'he;
   if (a[4] != 8'h00) begin
      $display("FAILED NB assign with part select 1, expected 00, got %x", a[4]);
      pass = 1'b0;
   end
   #0.1;
   if (a[4] != 8'he0) begin
      $display("FAILED NB assign with part select 2, expected e0, got %x", a[4]);
      pass = 1'b0;
   end

   // non-blocking, in index
   a[idx[5]] <= 8'h12;
   if (a[5] != 8'h00) begin
      $display("FAILED NB assign with word index 1, expected 00, got %x", a[4]);
      pass = 1'b0;
   end
   #0.1;
   if (a[5] != 8'h12) begin
      $display("FAILED NB assign with word index 2, expected 12, got %x", a[4]);
      pass = 1'b0;
   end

   // non-blocking, index and part select
   a[idx[6]][idx[0]*4 +: 4] <= 4'h3;
   if (a[6] != 8'h00) begin
      $display("FAILED NB assign with both 1, expected 00, got %x", a[4]);
      pass = 1'b0;
   end
   #0.1;
   if (a[6] != 8'h03) begin
      $display("FAILED NB assign with both 2, expected 03, got %x", a[4]);
      pass = 1'b0;
   end

   // NB, both, with a delay
   a[idx[7]][idx[1]*4 +: 4] <= #(idx[1]) 4'h4;
   #0.1;
   if (a[7] != 8'h00) begin
      $display("FAILED NB assign with both and delay 1, expected 00, got %x", a[4]);
      pass = 1'b0;
   end
   #1.1;
   if (a[7] != 8'h40) begin
      $display("FAILED NB assign with both and delay 2, expected 40, got %x", a[4]);
      pass = 1'b0;
   end

   // real array index
   r[idx[0]] = 1.1;
   if (r[0] != 1.1) begin
      $display("FAILED real word, expected 1.0, got %f", r[0]);
      pass = 1'b0;
   end

   // NB to real array
   r[idx[1]] <= 2.2;
   if (r[1] != 0.0) begin
      $display("FAILED NB assign real word 1, expected 0.0 got %f", r[1]);
      pass = 1'b0;
   end
   #0.1;
   if (r[1] != 2.2) begin
      $display("FAILED NB assign real word 2, expected 2.2 got %f", r[1]);
      pass = 1'b0;
   end

   // NB to real array with delay
   r[idx[2]] <= #(idx[2]) 3.3;
   #1.1;
   if (r[2] != 0.0) begin
      $display("FAILED NB assign with delay to real word 1, expected 0.0 got %f", r[1]);
      pass = 1'b0;
   end
   #1.0;
   if (r[2] != 3.3) begin
      $display("FAILED NB assign with delay to real word 2, expected 3.3 got %f", r[1]);
      pass = 1'b0;
   end

   if (pass) $display("PASSED");
end
endmodule
