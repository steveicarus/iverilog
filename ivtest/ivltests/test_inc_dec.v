/*
 * Author: Oswaldo Cadenas <oswaldo.cadenas@gmail.com>
 */
module test;

parameter S = 9;
parameter K = 3;
parameter L = 2**(S-K);
parameter N = 2**(S-1);

reg signed [S-1:0] a_reg;
bit signed [S-1:0] a_bit;
byte signed a_byte;
shortint signed a_short;
int signed a_int;
longint signed a_long;
byte signed amount;
byte unsigned pos;
int temp;
int i;

initial begin
   // test for style "a += some" statement on type reg
   for (i = 0; i < N; i = i+1) begin
     a_reg = $random % L;
     amount = $random % K;
     #1;
     temp = a_reg + amount;
     a_reg += amount;
     #1;
     //$display ("a = %0d, amount = %0d, temp = %0d", a, amount, temp);
     if (temp !== a_reg) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_reg - amount;
     a_reg -= amount;
     #1;
     if (temp !== a_reg) begin
        $display("FAILED");
        $finish;
     end
   end

   // test for style "a += some" statement on type bit
   for (i = 0; i < N; i = i+1) begin
     a_bit = $random % L;
     amount = $random % K;
     #1;
     temp = a_bit + amount;
     a_bit += amount;
     #1;
     if (temp !== a_bit) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_bit - amount;
     a_bit -= amount;
     #1;
     if (temp !== a_bit) begin
        $display("FAILED");
        $finish;
     end
   end // for

   // test for style "a += some" statement on type byte
   for (i = 0; i < N; i = i+1) begin
     a_byte = $random % L;
     amount = $random % K;
     #1;
     temp = a_byte + amount;
     a_byte += amount;
     #1;
     if (temp !== a_byte) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_byte - amount;
     a_byte -= amount;
     #1;
     if (temp !== a_byte) begin
        $display("FAILED");
        $finish;
     end
   end // for

   // test for style "a += some" statement on type shortint
   for (i = 0; i < N; i = i+1) begin
     a_short = 2*($random % L);
     amount = 2*($random % K);
     #1;
     temp = a_short + amount;
     a_short += amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short - amount;
     a_short -= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short * amount;
     a_short *= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short / amount;
     a_short /= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short % amount;
     a_short %= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short & amount;
     a_short &= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short | amount;
     a_short |= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short ^ amount;
     a_short ^= amount;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     pos = 2*({$random} % K);
     temp = a_short << pos;
     a_short <<= pos;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short >> pos;
     a_short >>= pos;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short <<< pos;
     a_short <<<= pos;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_short >>> pos;
     a_short >>>= pos;
     #1;
     if (temp !== a_short) begin
        $display("FAILED");
        $finish;
     end
   end // for

   // test for style "a += some" statement on type int
   for (i = 0; i < N; i = i+1) begin
     a_int = 4*($random % L);
     amount = 4*($random % K);
     #1;
     temp = a_int + amount;
     a_int += amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int - amount;
     a_int -= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int * amount;
     a_int *= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int / amount;
     a_int /= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int % amount;
     a_int %= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int & amount;
     a_int &= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int | amount;
     a_int |= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int ^ amount;
     a_int ^= amount;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     pos = 4*({$random} % K);
     temp = a_int << pos;
     a_int <<= pos;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int >> pos;
     a_int >>= pos;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int <<< pos;
     a_int <<<= pos;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_int >>> pos;
     a_int >>= pos;
     #1;
     if (temp !== a_int) begin
        $display("FAILED");
        $finish;
     end
   end // for

   // test for style "a += some" statement on type longint
   for (i = 0; i < N; i = i+1) begin
     a_long = 8*($random % L);
     amount = 8*($random % K);
     #1;
     temp = a_long + amount;
     a_long += amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long - amount;
     a_long -= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long * amount;
     a_long *= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long / amount;
     a_long /= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long % amount;
     a_long %= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long & amount;
     a_long &= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long | amount;
     a_long |= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long ^ amount;
     a_long ^= amount;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     pos = 8*({$random} % K);
     temp = a_long << pos;
     a_long <<= pos;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long >> pos;
     a_long >>= pos;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long <<< pos;
     a_long <<<= pos;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
     #1;
     temp = a_long >>> pos;
     a_long >>= pos;
     #1;
     if (temp !== a_long) begin
        $display("FAILED");
        $finish;
     end
   end // for

   $display("PASSED");
   $finish;
end

endmodule
