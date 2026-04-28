SystemVerilog array locator methods (queues and dynamic arrays)
================================================================

This directory includes regression tests for IEEE 1800 locator methods
on unpacked queues (int q[$]) and dynamic arrays (int da[]):

  find, find_index, find_first, find_first_index, find_last,
  find_last_index, min, max, unique, unique_index, sum (integral)

Behavior notes (LRM-oriented):

  * Methods may use a value argument, e.g. q.find(2), or a predicate with
    `with`, e.g. q.find() with (item == 2). A `with` clause must not be
    combined with a method value argument.

  * find_first, find_last, find_first_index, and find_last_index return a
    queue with zero or one element; no match yields an empty queue (not a
    scalar sentinel).

  * min() and max() return queues containing all elements equal to the
    selected extrema.

  * sum() returns the scalar reduction sum of elements (integral vector types);
    empty arrays/queues yield 0. `sum() with (expr)` reduces the expression
    result for each item.

  * For dynamic arrays, runtime support treats storage as vvp_darray (including
    atom-backed integral arrays), not only vvp_queue_vec4. See vvp/vthread.cc:
    get_queue_or_darray_vec4_from_net() and the %queue/* opcodes used for
    size, word read, find, and unique.

Compiler / codegen touchpoints (typical):

  * Elaboration: elab_expr.cc — NetESFunc $ivl_queue_method$* and with-predicate
    lowering.
  * Code generation: tgt-vvp/eval_object.c — eval_queue_method_find,
    eval_queue_method_find_with; eval_vec4.c for non-with find*.
  * VVP: vvp/vthread.cc — opcode implementations; vvp/compile.cc — opcode tables.

Regression tests (see ivtest/vvp_tests/*.json and regress-vvp.list):

  sv_queue_find.v              Value-argument find* (equality), empty cases.
  sv_queue_find_with.v         find* with `with` (item, index).
  sv_queue_find_locators_ext.v Longer queue, compound predicates.
  sv_queue_unique.v            unique / unique_index.
  sv_darray_find_locators.v    Same locator patterns on int[] dynamic array.
  sv_darray_unique.v           unique() and unique_index() on int[] dynamic array.
  sv_queue_min_max.v           min() and max() on queue values.
  sv_darray_min_max.v          min() and max() on dynamic array values.
  sv_queue_min_max_with.v      min()/max() with predicate on queue values.
  sv_darray_min_max_with.v     min()/max() with predicate on dynamic arrays.
  sv_class_darray_prop_locators.v locator methods on class dynamic-array properties.
  sv_queue_unique_with.v       unique()/unique_index() with predicate on queues.
  sv_darray_unique_with.v      unique()/unique_index() with predicate on dynamic arrays.
  sv_class_queue_prop_locators.v  locator methods on class queue properties.
  sv_queue_sum.v               integral sum() reduction on queues.
  sv_darray_sum.v              integral sum() reduction on dynamic arrays.
  sv_queue_sum_with.v          sum() with expression on queues.
  sv_darray_sum_with.v         sum() with expression on dynamic arrays.
