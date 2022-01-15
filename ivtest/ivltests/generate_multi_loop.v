`begin_keywords "1364-2005"
module generate_multi_loop();

reg [31:0] input_value;

wire [31:0] output_value;

generate
  genvar i;
  genvar j;

  for (i = 0; i < 4; i = i + 1) begin:byte
    wire [7:0] byte_value;

    for (j = 0; j < 8; j = j + 1) begin:bit
      wire bit_value;

      buf buffer(bit_value, input_value[i*8+j]);

      assign byte_value[j] = bit_value;
    end

    assign output_value[i*8+7:i*8] = byte_value;
  end
endgenerate

initial begin
  input_value = 32'h12345678;
  #1;
  $write("byte_value =");
  $write(" %b", byte[3].byte_value);
  $write(" %b", byte[2].byte_value);
  $write(" %b", byte[1].byte_value);
  $write(" %b", byte[0].byte_value);
  $write("\n");
  $write("bit_value  = ");
  $write("%b", byte[3].bit[7].bit_value);
  $write("%b", byte[3].bit[6].bit_value);
  $write("%b", byte[3].bit[5].bit_value);
  $write("%b", byte[3].bit[4].bit_value);
  $write("%b", byte[3].bit[3].bit_value);
  $write("%b", byte[3].bit[2].bit_value);
  $write("%b", byte[3].bit[1].bit_value);
  $write("%b", byte[3].bit[0].bit_value);
  $write(" ");
  $write("%b", byte[2].bit[7].bit_value);
  $write("%b", byte[2].bit[6].bit_value);
  $write("%b", byte[2].bit[5].bit_value);
  $write("%b", byte[2].bit[4].bit_value);
  $write("%b", byte[2].bit[3].bit_value);
  $write("%b", byte[2].bit[2].bit_value);
  $write("%b", byte[2].bit[1].bit_value);
  $write("%b", byte[2].bit[0].bit_value);
  $write(" ");
  $write("%b", byte[1].bit[7].bit_value);
  $write("%b", byte[1].bit[6].bit_value);
  $write("%b", byte[1].bit[5].bit_value);
  $write("%b", byte[1].bit[4].bit_value);
  $write("%b", byte[1].bit[3].bit_value);
  $write("%b", byte[1].bit[2].bit_value);
  $write("%b", byte[1].bit[1].bit_value);
  $write("%b", byte[1].bit[0].bit_value);
  $write(" ");
  $write("%b", byte[0].bit[7].bit_value);
  $write("%b", byte[0].bit[6].bit_value);
  $write("%b", byte[0].bit[5].bit_value);
  $write("%b", byte[0].bit[4].bit_value);
  $write("%b", byte[0].bit[3].bit_value);
  $write("%b", byte[0].bit[2].bit_value);
  $write("%b", byte[0].bit[1].bit_value);
  $write("%b", byte[0].bit[0].bit_value);
  $write("\n");
  if (output_value == input_value)
    $display("Test passed");
  else
    $display("Test FAILED");
end

endmodule
`end_keywords
