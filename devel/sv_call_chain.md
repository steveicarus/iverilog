# SystemVerilog chained calls: `a().b()`

This note describes the parser and elaboration support for **chained calls**: a
function call whose value is a class handle, followed by one or more
`.method(args)` segments (e.g. `get_c().f()`, `a().b().c()`).

## Language shape

- **Parse:** A dedicated nonterminal `call_chain_expr` in `parse.y` builds a
  left-associated chain:
  - `hierarchy_identifier attribute_list_opt argument_list_parens` — first call;
  - `call_chain_expr '.' hierarchy_identifier attribute_list_opt argument_list_parens` — each further segment.
- **`expr_primary`** includes `call_chain_expr` **before** the bare
  `hierarchy_identifier` alternative so `id (` is parsed as a call, not as an
  identifier plus a stray `(`.

## Parse tree (`PExpr`)

- **`PECallFunction`** (`PExpr.h` / `PExpr.cc`):
  - Optional **`chain_prefix_`**: inner `PExpr` for the prefix (another
    `PECallFunction` for longer chains).
  - Constructors and accessors: `peek_path()`, `peek_chain_prefix()`.
- **`pform_make_chained_call_function`** (`pform.cc`) — requires SystemVerilog;
  builds `PECallFunction(prefix, method_name, args)`.

## Elaboration (`elab_expr.cc`)

- **`PECallFunction::elaborate_expr_`** delegates to **`elaborate_expr_chain_`**
  when `chain_prefix_` is set.
- **`elaborate_class_method_net_this_`** passes the elaborated prefix as the
  implicit `this` argument (first parameter slot), including nested `NetEUFunc`
  for inner calls — not only `NetESignal(net)`.
- **`resolve_call_chain_prefix_class`** (static helper) resolves the **class
  type** of the prefix for multi-hop chains (e.g. width checks), walking the
  chain prefix recursively instead of searching only the tail name.

## Dump / debug

- **`pform_dump.cc`** prints chained calls with a `prefix.` prefix before the
  method path.

## Regression

- **`ivtest/ivltests/sv_call_chain_method1.v`**
- **`ivtest/vvp_tests/sv_call_chain_method1.json`** (`-g2012`)
- Listed in **`ivtest/regress-vvp.list`** as `sv_call_chain_method1`.

## Using a locally built `iverilog`

`iverilog` invokes the installed compiler under your prefix, typically
`$PREFIX/lib/ivl/ivl`, not the `ivl` binary in the build tree. After changing
the parser, **reinstall** or copy the new `ivl` into that lib directory so
`iverilog` picks up the change; otherwise chained-call syntax may still fail
with a **syntax error** while a direct `./ivl -C...` test from the build tree
succeeds.

```bash
# Example after building in-tree
make install
# or copy only the compiler binary to your existing install
cp ivl "$PREFIX/lib/ivl/ivl"
```

## Related files (non-exhaustive)

| Area        | Files |
|------------|--------|
| Grammar    | `parse.y` (`call_chain_expr`, `expr_primary`) |
| Parse form | `pform.cc`, `pform.h` |
| AST        | `PExpr.h`, `PExpr.cc` |
| Elab       | `elab_expr.cc` |
| Dump       | `pform_dump.cc` |
