// pr1776485.v

module vvp_fun_and;

   wire [7:0] ADC_gain;
   reg [7:0]  force_low;
   wire [7:0] adc_gain_out = ADC_gain & ~force_low;

   UXO source(ADC_gain[5:0]);

   initial begin
      force_low = 0;

      #1 $display("ADC_gain = %b, acd_gain_out = %b", ADC_gain, adc_gain_out);

      if (ADC_gain !== 8'bzz_101010) begin
	 $display("FAILED -- ADC_gain=%b", ADC_gain);
	 $finish;
      end

      if (adc_gain_out !== 8'bxx_101010) begin
	 $display("FAILED -- adc_gain_out=%b", adc_gain_out);
	 $finish;
      end

      $display("PASSED");
   end

endmodule

module UXO(output [5:0] gain);
   assign gain = 6'b101010;
endmodule
