// Check that various timing checks can be parsed

module test;

  initial begin
      $display("PASSED");
  end

  wire sig1, sig2, del_sig1, del_sig2, notifier, cond1, cond2;

  specify
    $setup(posedge sig1 , negedge sig2 , 0:0:0);
    $setup(negedge sig1 , posedge sig2 , 0:0:0);
    $setup(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $setup(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $setup(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $setup(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $setup(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);

    $hold(posedge sig1 , negedge sig2 , 0:0:0);
    $hold(negedge sig1 , posedge sig2 , 0:0:0);
    $hold(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $hold(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $hold(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $hold(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $hold(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);

    $setuphold(posedge sig1 , negedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(negedge sig1 , posedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier, cond1 , cond2) ;
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier, cond1 , cond2 , del_sig1 , del_sig2 ) ;
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 , del_sig2 ) ;
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 , del_sig2 ) ;
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,,, del_sig2 ) ;
    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 ,) ;
    $setuphold(edge [10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $setuphold(posedge sig1 , edge [10, x0, 1x] sig2 , 0:0:0 , 0:0:0 , notifier);

    $removal(posedge sig1 , negedge sig2 , 0:0:0);
    $removal(negedge sig1 , posedge sig2 , 0:0:0);
    $removal(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $removal(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $removal(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $removal(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $removal(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);

    $recovery(posedge sig1 , negedge sig2 , 0:0:0);
    $recovery(negedge sig1 , posedge sig2 , 0:0:0);
    $recovery(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $recovery(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $recovery(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $recovery(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $recovery(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);

    $recrem(posedge sig1 , negedge sig2 , 0:0:0 , 0:0:0);
    $recrem(negedge sig1 , posedge sig2 , 0:0:0 , 0:0:0);
    $recrem(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $recrem(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier, cond1 , cond2);
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier, cond1 , cond2 , del_sig1 , del_sig2 );
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 , del_sig2 );
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 , del_sig2 );
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,,, del_sig2 );
    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier,,, del_sig1 ,);
    $recrem(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $recrem(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , 0:0:0 , notifier);

    $skew(posedge sig1 , negedge sig2 , 0:0:0);
    $skew(negedge sig1 , posedge sig2 , 0:0:0);
    $skew(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $skew(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $skew(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $skew(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $skew(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);

    $timeskew(posedge sig1 , negedge sig2 , 0:0:0);
    $timeskew(negedge sig1 , posedge sig2 , 0:0:0);
    $timeskew(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $timeskew(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , notifier);
    $timeskew(posedge sig1, negedge sig2 , 0:0:0 , notifier);
    $timeskew(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , notifier);
    $timeskew(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , notifier);
    $timeskew(posedge sig1 , negedge sig2 , 0:0:0 , notifier , 1'b0);
    $timeskew(negedge sig1 , posedge sig2 , 0:0:0 , notifier , 1'b1 , 1'b0);
    $timeskew(negedge sig1 , posedge sig2 , 0:0:0 , , , 1'b1);

    $fullskew(posedge sig1 , negedge sig2 , 0:0:0 , 0:0:0);
    $fullskew(negedge sig1 , posedge sig2 , 0:0:0 , 0:0:0);
    $fullskew(posedge sig1 &&& cond1 == cond2 , posedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $fullskew(negedge sig1 &&& cond1 == cond2 , negedge sig2 &&& cond1 == cond2 , 0:0:0 , 0:0:0 , notifier);
    $fullskew(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $fullskew(edge[10, x0, 1x] sig1 , posedge sig2 , 0:0:0 , 0:0:0 , notifier);
    $fullskew(posedge sig1 , edge[10, x0, 1x] sig2 , 0:0:0 , 0:0:0 , notifier);
    $fullskew(posedge sig1 , negedge sig2 , 0:0:0 , 0:0:0 , notifier , 1'b0);
    $fullskew(negedge sig1 , posedge sig2 , 0:0:0 , 0:0:0 , notifier , 1'b1 , 1'b0);
    $fullskew(negedge sig1 , posedge sig2 , 0:0:0 , 0:0:0 , , , 1'b1);

    $width(posedge sig1 , 0:0:0 );
    $width(posedge sig1 &&& cond1 , 0:0:0 , 0 );
    $width(posedge sig1 &&& cond1 , 0:0:0 , 0 , notifier );
    $width(edge[10, x0, 1x] sig1 &&& cond1 , 0:0:0 );

    $period(posedge sig1 , 0:0:0 );
    $period(negedge sig1 &&& cond1 , 0:0:0 , notifier );
    $period(edge[10, x0, 1x] sig1 &&& cond1 , 0:0:0 );

    $nochange(posedge sig1 , posedge sig2 , 10 , 20 );
    $nochange(negedge sig1 &&& cond1 , posedge sig2 , 10 , 20 );
    $nochange(negedge sig1 , posedge sig2 &&& cond1 , 10 , 20 , notifier );
    $nochange(edge[10, x0, 1x] sig1 &&& cond1 , posedge sig2 , 10 , 20 );
    $nochange(posedge sig1 &&& cond1 , edge[10, x0, 1x] sig2 , 10 , 20 );

  endspecify
endmodule
