# Associative arrays (Tier A #2)

Status: **partial** — string-keyed vertical slice for UVM-like tables.

## Supported in this slice

```systemverilog
int aa[string];
int aa[*];          // accepted as string-keyed (not IEEE int-key wildcard yet)
aa["x"] = 1;
v = aa["x"];
n = aa.size();      // also aa.num()
aa.exists("x");
aa.delete("x");
aa.delete();        // clear all
foreach (aa[k]) ... // k is string
bb = aa;            // whole-array copy
```

Element types: packed integral (`int` / `logic` vectors) only for now.

## Encoding

| Syntax | `pform_range_t` sentinel |
|--------|--------------------------|
| `[*]` | `(PENull, PENull)` — distinct from queue `(PENull, 0)` and darray `(0, 0)` |
| `[string]` | `(PETypename(string), 0)` via `'[' expression ']'` (avoids bison conflict with typename-as-expr) |

Elaborates to `netaarray_t` with `IVL_VT_AARRAY`. Runtime: `vvp_aarray_vec4` (`std::map<std::string, vvp_vector4_t>`).

## Deferred (do not claim)

- Integer / class / enum keys (true IEEE `[*]` integer keys)
- Class / string / real / nested AA elements
- `first` / `last` / `next` / `prev` methods (foreach uses internal `key_at`)
- Associative arrays as class properties

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).
