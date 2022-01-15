/* pr1650842 */

 module test;

   initial main;

   task main;
      integer _$ID241, _$ID246, _$ID247, _$ID248, _$ID249, a;
      begin
	 a = 0;
	 _$ID241 = a;
	 a = 9;
	 _$ID246 = a;
	 _$ID247 = 3;
	 a = _$ID247;
	 _$ID248 = _$ID246 + _$ID247 ;
	 _$ID249 = _$ID248;
	 a = _$ID249;
	 if( a !== 12 ) begin
	    $write("FAIL: expected 12; got %d\n", a);
	    $display("_$ID241=%d", _$ID241);
	    $display("_$ID246=%d", _$ID246);
	    $display("_$ID247=%d", _$ID247);
	    $display("_$ID248=%d", _$ID248);
	    $display("_$ID249=%d", _$ID249);
	 end else begin
	    $write("PASSED\n");
	 end
      end
   endtask
 endmodule
