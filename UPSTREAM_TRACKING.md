# Upstream Tracking

This is the reconciliation ledger between
[`steveicarus/iverilog`](https://github.com/steveicarus/iverilog) and this
[`apullin/aiverilog`](https://github.com/apullin/aiverilog) fork. It records
work derived from upstream issues, work that upstream later made unnecessary,
and reviewed reports for which the fork deliberately carries no patch.

Last reconciled: 2026-08-29

- Latest upstream commit integrated at this checkpoint:
  [`64f13540a`](https://github.com/steveicarus/iverilog/commit/64f13540a6ec8122c16b10efa220ed0eff4686f9).
- Latest upstream-derived change before this ledger update:
  [`ca67eb43a`](https://github.com/apullin/aiverilog/commit/ca67eb43aff07dd0faa97a0035cf259cd31dc8a8).
- `Landed` means that Aiverilog intentionally retains a fork delta.
- `Elided` means that no fork delta remains or was needed, usually because
  upstream already supplied the behavior.
- This records fork disposition, not the mutable open/closed state displayed
  by the upstream issue tracker.

## Why The History Contains Upstream Merges

The 2026 fork campaign began from common base
[`0723d9a47`](https://github.com/steveicarus/iverilog/commit/0723d9a477b602dead4d99590de0bc5e3647737f)
without first refreshing the local upstream remote. By the time the omission
was discovered, 84 fork-only non-merge commits had already been published and
linked through the fork's issues and pull requests. Upstream had meanwhile
advanced by 15 commits to
[`e5482b89b`](https://github.com/steveicarus/iverilog/commit/e5482b89b1ec36237e0c70bdb8e0c8f8acd87153).

Rewriting the published fork would have broken those review links without
improving the resulting source tree. Instead,
[Aiverilog PR #27](https://github.com/apullin/aiverilog/pull/27) merged the
upstream history after a patch-by-patch cross-check. Pure upstream still failed
22 of 25 selected fork legacy regressions, failed 12 of 16 modern regressions,
and timed out on one more; none of the 84 fork commits was patch-equivalent to
the incoming work. The combined tree then passed the complete legacy, modern,
and VPI suites. A later ordinary merge integrated upstream
[`15689210c`](https://github.com/steveicarus/iverilog/commit/15689210c137c23107c96f63ddcdfb0d72076aae).

The fork therefore uses normal upstream merges from that point forward. The
published PR graph is historical evidence, not a stack that should be rebased
or force-pushed whenever upstream advances.

The second sync on 2026-08-29 merged 105 upstream commits (`15689210c..64f13540a`)
via [`2c098f871`](https://github.com/apullin/aiverilog/commit/2c098f87177cf3f4cd18f0d9785fccca70946751)
and two follow-ups
[`616095528`](https://github.com/apullin/aiverilog/commit/616095528436836be877743ca6e03ed5819ca7f7)
and [`39032f39d`](https://github.com/apullin/aiverilog/commit/39032f39d6a153b7399e43c1aad00c48d2d8cf83).
Fork bugfix branches were retained as artifacts per policy; the merge is the
single source of truth for comparison.

The first attempt to publish the subsequent issue work accidentally made the
entire sync part of PR #68, then merged PRs #69-#71 and #73 before their checks
and review were complete. Fork `master` was repaired to retain the reviewed sync
commits above while replacing those post-sync changes with the focused commits
listed below. The old PR pages remain audit records, not authoritative patch
boundaries.

## Landed Before Fork Issue Bookkeeping

These fixes were committed directly during the original bugfix campaign. They
all link the upstream report in their commit messages, but predate the later
one-fork-issue/one-PR workflow.

| Upstream report | Fork record | Retained result |
| --- | --- | --- |
| [#1112](https://github.com/steveicarus/iverilog/issues/1112) | [`cd34255c4`](https://github.com/apullin/aiverilog/commit/cd34255c42a570db8075583aace6324ef9616729), later [fork #11](https://github.com/apullin/aiverilog/issues/11) / [PR #12](https://github.com/apullin/aiverilog/pull/12) | Diagnose an invalid `$bits` argument instead of silently returning zero; keep both test runners' diagnostics aligned. |
| [#1170](https://github.com/steveicarus/iverilog/issues/1170) | [`7c58db61d`](https://github.com/apullin/aiverilog/commit/7c58db61dffd1a743810820e3cbaa93367a3c8b6) | Let `tgt-sizer` skip the SystemVerilog `$unit` package scope. |
| [#1267](https://github.com/steveicarus/iverilog/issues/1267) | [`25a19d36b`](https://github.com/apullin/aiverilog/commit/25a19d36ba7244c227f3337ef645fcfa3e6f899a) | Allow `wire logic` connected to a `uwire` port to have multiple drivers. Interaction with resolved net types remains an explicit TODO in the commit. |
| [#1268](https://github.com/steveicarus/iverilog/issues/1268) | [`fa9205e05`](https://github.com/apullin/aiverilog/commit/fa9205e05020d6c6ef6b24c1c65d60e997c5976e) | Permit a variable to be driven by one primitive-gate output. |
| [#1134](https://github.com/steveicarus/iverilog/issues/1134) | [`11b20413e`](https://github.com/apullin/aiverilog/commit/11b20413e547e712e13f6bd1718e62288a2a6315) | Support member access through unpacked arrays of packed structs. |
| [#716](https://github.com/steveicarus/iverilog/issues/716) | [`d00e07ccc`](https://github.com/apullin/aiverilog/commit/d00e07ccca6794d597df8c7bc23cf230c0e1f958) | Report incompatible task argument types instead of asserting. |
| [#1298](https://github.com/steveicarus/iverilog/issues/1298) | [`628173e8c`](https://github.com/apullin/aiverilog/commit/628173e8c8e4524e12f1d2f85fe73957ac8f9e51) | Elaborate and execute string-valued conditional expressions. |
| [#1321](https://github.com/steveicarus/iverilog/issues/1321) | [`9217e3c0f`](https://github.com/apullin/aiverilog/commit/9217e3c0fc784761755cce596a02621487f77f60) | Accept prefix labels on `begin` and `fork` blocks. |
| [#1276](https://github.com/steveicarus/iverilog/issues/1276) | [`dab398b83`](https://github.com/apullin/aiverilog/commit/dab398b83fc5b14f1c971fc2dbdff7e4daf9b642) | Guard only: reject an unsupported procedural continuous assignment with a concatenation l-value instead of asserting. |
| [#1119](https://github.com/steveicarus/iverilog/issues/1119) / [#1120](https://github.com/steveicarus/iverilog/issues/1120) | [`a37e1dc68`](https://github.com/apullin/aiverilog/commit/a37e1dc68ce58522cb662b92d3562a6149aa1a61) | Guard only: make the `vlog95` backend reject unsafe shared-port translation instead of asserting or emitting incorrect Verilog. |
| [#1103](https://github.com/steveicarus/iverilog/issues/1103) | [`66fd57d0d`](https://github.com/apullin/aiverilog/commit/66fd57d0d970b71aba86d6070b79f162d4e614ed) | Guard only: diagnose unsupported `join_any`/`join_none` in automatic scopes before VVP aborts. |
| [#455](https://github.com/steveicarus/iverilog/issues/455) | [`e9720bb6c`](https://github.com/apullin/aiverilog/commit/e9720bb6c4fd7ff096f87d5c4e25c9bb4af9c5c6) | Enforce ``default_nettype none`` on ANSI and split port declarations. |
| [#1038](https://github.com/steveicarus/iverilog/issues/1038) | [`be37b84fb`](https://github.com/apullin/aiverilog/commit/be37b84fbea24a0365693fb57450efadf4fd2041) | Resolve package-qualified names through `vpi_handle_by_name`. |
| [#1291](https://github.com/steveicarus/iverilog/issues/1291) | [`8633c2cec`](https://github.com/apullin/aiverilog/commit/8633c2cecadf567fcec239b66da63e1de0974d5d) | Guard only: turn an uninitialized event-pin abort into a source-located unsupported diagnostic. Root-cause elaborator reuse remains deferred. |
| [#1236](https://github.com/steveicarus/iverilog/issues/1236) | [`2a4ec9ec2`](https://github.com/apullin/aiverilog/commit/2a4ec9ec2f675504dcd723bb60d0e742139dc4d1) | Stop shrinking live `StringHeap` cells with `realloc`, which could move storage and invalidate every returned string. |
| [#1202](https://github.com/steveicarus/iverilog/issues/1202) | [`c90d96ec2`](https://github.com/apullin/aiverilog/commit/c90d96ec2f7e31f2efcfdc2f0cd0c60763b5d845), adapted in [`616095528`](https://github.com/apullin/aiverilog/commit/616095528436836be877743ca6e03ed5819ca7f7) | Drive default expressions on top-level input ports. Post-sync `LineInfo::set_line` only copies `lexical_pos` when `UINT_MAX`; the synthesized `PEIdent`'s `~0U` was clobbered, re-asserted after `set_line` (check that `early_sig_elab3` gold also tracked the new `591d2c247` phrasing). |

## Landed Through Fork Issues And Pull Requests

This is the durable public workflow used after the initial stack. A merge hash
identifies the point at which the result entered fork `master`; implementation
commits are included for batched PRs.

| Upstream report | Fork issue / PR / merge | Retained result |
| --- | --- | --- |
| [#1407](https://github.com/steveicarus/iverilog/issues/1407) | [#1](https://github.com/apullin/aiverilog/issues/1), [PR #2](https://github.com/apullin/aiverilog/pull/2), [`19bec699a`](https://github.com/apullin/aiverilog/commit/19bec699a7d55a560a034fbb0868cd2ddfff4d8c) | Dynamically size wide literal VPI argument rendering instead of overflowing a stack buffer. |
| [#1411](https://github.com/steveicarus/iverilog/issues/1411) | [#3](https://github.com/apullin/aiverilog/issues/3), [PR #4](https://github.com/apullin/aiverilog/pull/4), [`33e3dde03`](https://github.com/apullin/aiverilog/commit/33e3dde03a5b8fcd66a8066d3866b7e68652559a) | Clear reused VPI vector storage so stale high bits cannot corrupt bit-vector system functions. |
| [#1288](https://github.com/steveicarus/iverilog/issues/1288) | [#5](https://github.com/apullin/aiverilog/issues/5), [PR #6](https://github.com/apullin/aiverilog/pull/6), [`fa1dc4e82`](https://github.com/apullin/aiverilog/commit/fa1dc4e8260587f278954df31aa762110dcc156a) | Accept a final command-file name without a trailing newline. |
| [#1019](https://github.com/steveicarus/iverilog/issues/1019) | [#7](https://github.com/apullin/aiverilog/issues/7), [PR #8](https://github.com/apullin/aiverilog/pull/8), [`5497518c6`](https://github.com/apullin/aiverilog/commit/5497518c66a13d4d6617efa9dd526a0023e7a36e) | Add `iverilog --help` and `--version`. |
| [#1187](https://github.com/steveicarus/iverilog/issues/1187) | [#9](https://github.com/apullin/aiverilog/issues/9), [PR #10](https://github.com/apullin/aiverilog/pull/10), [`73d5c68cc`](https://github.com/apullin/aiverilog/commit/73d5c68cc5c48adf1bc03edc910b4340361be35c) | Support spaces in `-B` tool paths while preserving native MinGW invocation syntax. |
| [#1396](https://github.com/steveicarus/iverilog/issues/1396) | [#13](https://github.com/apullin/aiverilog/issues/13), [PR #14](https://github.com/apullin/aiverilog/pull/14), [`d3487c94f`](https://github.com/apullin/aiverilog/commit/d3487c94fc7425f49b80f957bfe3f748b55ebb1e) | Include source and destination types in required enum-cast diagnostics. |
| [#1000](https://github.com/steveicarus/iverilog/issues/1000) | [#15](https://github.com/apullin/aiverilog/issues/15), [PR #16](https://github.com/apullin/aiverilog/pull/16), [`1259da6f0`](https://github.com/apullin/aiverilog/commit/1259da6f0b46876318b930b78bfd3a2b4fc3fcb1) | Emit escaped Make-compatible dependency files with `-Mmake=<file>`. |
| [#1322](https://github.com/steveicarus/iverilog/issues/1322) / [#1381](https://github.com/steveicarus/iverilog/issues/1381) | [#17](https://github.com/apullin/aiverilog/issues/17), [PR #18](https://github.com/apullin/aiverilog/pull/18), [`931cb5564`](https://github.com/apullin/aiverilog/commit/931cb55646ba8819ce7b570e2b6621a4d1a8171e) | Import and harden [upstream PR #1377](https://github.com/steveicarus/iverilog/pull/1377): optional warnings for invalid dynamic unpacked-array writes. |
| [#1218](https://github.com/steveicarus/iverilog/issues/1218) | [#19](https://github.com/apullin/aiverilog/issues/19), [PR #20](https://github.com/apullin/aiverilog/pull/20), [`e8cb74996`](https://github.com/apullin/aiverilog/commit/e8cb74996706bcf5af7c9aa122c42e45daf2799b) | Preserve an undriven local net while an unpacked-array alias still refers to its nexus. |
| [PR #1405](https://github.com/steveicarus/iverilog/pull/1405) | [#21](https://github.com/apullin/aiverilog/issues/21), [PR #22](https://github.com/apullin/aiverilog/pull/22), [`593fce299`](https://github.com/apullin/aiverilog/commit/593fce299eb3ed17aa6234d680a8133a4b878ec3) | Import missing `AH_TEMPLATE` declarations so modern `autoheader` can regenerate the build files. |
| [#1220](https://github.com/steveicarus/iverilog/issues/1220) | [PR #24](https://github.com/apullin/aiverilog/pull/24), [`580b4bf81`](https://github.com/apullin/aiverilog/commit/580b4bf81ac06d67206be6a1e0926bfa3519a656), merge [`1ea7a6621`](https://github.com/apullin/aiverilog/commit/1ea7a66213b6fa0da2224c221e44164c79a5d00c) | Materialize nexuses for unconnected unpacked `uwire` ports. |
| [#276](https://github.com/steveicarus/iverilog/issues/276) / [#521](https://github.com/steveicarus/iverilog/issues/521) / [#1277](https://github.com/steveicarus/iverilog/issues/1277) | [PR #24](https://github.com/apullin/aiverilog/pull/24), then [#36](https://github.com/apullin/aiverilog/issues/36) / [PR #37](https://github.com/apullin/aiverilog/pull/37), merge [`9b3f47e1a`](https://github.com/apullin/aiverilog/commit/9b3f47e1a8004e3107f761deb436c2a722cb279d) | Canonically collapse dynamic packed-prefix indices for full selections and slices, on reads and writes. |
| [#1317](https://github.com/steveicarus/iverilog/issues/1317) | [PR #24](https://github.com/apullin/aiverilog/pull/24), [`14033b9f7`](https://github.com/apullin/aiverilog/commit/14033b9f73c1f0cd285ddbd92d5baac5dc50c6b3), merge [`1ea7a6621`](https://github.com/apullin/aiverilog/commit/1ea7a66213b6fa0da2224c221e44164c79a5d00c) | Reject conflicting procedural drivers when one process is `always_ff`. |
| [#1016](https://github.com/steveicarus/iverilog/issues/1016) | [PR #25](https://github.com/apullin/aiverilog/pull/25), [`50b564f05`](https://github.com/apullin/aiverilog/commit/50b564f057803bc8a4bb90cbb3ee7768c0710741), merge [`4a64e57c8`](https://github.com/apullin/aiverilog/commit/4a64e57c8691d17efd76405ce01636d8ad8d5c16) | Keep named sequential blocks in their enclosing loop thread unless a normal `disable` statement actually targets the scope. |
| [#638](https://github.com/steveicarus/iverilog/issues/638) | [PR #25](https://github.com/apullin/aiverilog/pull/25), [`b17b185c1`](https://github.com/apullin/aiverilog/commit/b17b185c1e7221d622202b47b32edf006cdc44c9), merge [`4a64e57c8`](https://github.com/apullin/aiverilog/commit/4a64e57c8691d17efd76405ce01636d8ad8d5c16) | Remove scanner `REJECT` paths so Flex buffers can grow for long generated tokens. |
| [#1093](https://github.com/steveicarus/iverilog/issues/1093) / [#1325](https://github.com/steveicarus/iverilog/issues/1325) | [#28](https://github.com/apullin/aiverilog/issues/28), [PR #29](https://github.com/apullin/aiverilog/pull/29), [`1eff0b171`](https://github.com/apullin/aiverilog/commit/1eff0b171b779ebfa3c17bc8dce7cd730c49a619) | Preserve per-bit variable state when only part of a coerced variable has structural drivers. |
| [#1436](https://github.com/steveicarus/iverilog/issues/1436) | [#30](https://github.com/apullin/aiverilog/issues/30), [PR #31](https://github.com/apullin/aiverilog/pull/31), [`e8271656a`](https://github.com/apullin/aiverilog/commit/e8271656ab2f0ebc4c9d2bb8380207ec375baa39) | Implement VVP multiple-match checks for `unique` and `unique0` cases. |
| [#1231](https://github.com/steveicarus/iverilog/issues/1231) | [#32](https://github.com/apullin/aiverilog/issues/32), [PR #33](https://github.com/apullin/aiverilog/pull/33), [`9d8696855`](https://github.com/apullin/aiverilog/commit/9d8696855f79d1771060d0d772837c93afb4f84c) | Diagnose an unsupported coerced unpacked inout port instead of asserting. |
| [#1183](https://github.com/steveicarus/iverilog/issues/1183) | [#34](https://github.com/apullin/aiverilog/issues/34), [PR #35](https://github.com/apullin/aiverilog/pull/35), [`003c7a8de`](https://github.com/apullin/aiverilog/commit/003c7a8dee678fdfecf3c9f0a473134689b5f02d) | Reject an unpacked array passed to a packed task output before target generation. |
| [#835](https://github.com/steveicarus/iverilog/issues/835) | [#38](https://github.com/apullin/aiverilog/issues/38), [PR #39](https://github.com/apullin/aiverilog/pull/39), [`32d53f1cd`](https://github.com/apullin/aiverilog/commit/32d53f1cd30eb51958ec0ce33a05f15ccc1879ac) | Support dynamic packed-array prefixes on packed-struct member l-values and cleanly reject unsupported member indices. |
| [#470](https://github.com/steveicarus/iverilog/issues/470) | [#40](https://github.com/apullin/aiverilog/issues/40), [PR #41](https://github.com/apullin/aiverilog/pull/41), [`73cc40a6b`](https://github.com/apullin/aiverilog/commit/73cc40a6b5431972a04f175bf6f6c2b0ee624ff5) | Support event controls on dynamic-array elements without delivering objects to vector part-select functors. |
| [#405](https://github.com/steveicarus/iverilog/issues/405) | [#42](https://github.com/apullin/aiverilog/issues/42), [PR #43](https://github.com/apullin/aiverilog/pull/43), [`28f650f3f`](https://github.com/apullin/aiverilog/commit/28f650f3fb6cd0aa44c19c93aa501b6f410f2db6) | Validate arguments even when a task or void function has an empty body. |
| [#312](https://github.com/steveicarus/iverilog/issues/312) | [#44](https://github.com/apullin/aiverilog/issues/44), [PR #45](https://github.com/apullin/aiverilog/pull/45), [`aa233e543`](https://github.com/apullin/aiverilog/commit/aa233e543bc51a652f05f5d2e8e2809d205fddd1) | Preserve the VPI variable type of continuously driven `logic` while retaining net-resolution semantics. |
| [#962](https://github.com/steveicarus/iverilog/issues/962) | [#46](https://github.com/apullin/aiverilog/issues/46), [PR #47](https://github.com/apullin/aiverilog/pull/47), [`b354b91f9`](https://github.com/apullin/aiverilog/commit/b354b91f99cfb30831358a323154ad0c2591cb8f) | Stop active and nonblocking scheduler work immediately after `$finish`, while still running final blocks. |
| [#1140](https://github.com/steveicarus/iverilog/issues/1140) | [#48](https://github.com/apullin/aiverilog/issues/48), [PR #49](https://github.com/apullin/aiverilog/pull/49), [`6bdfd2254`](https://github.com/apullin/aiverilog/commit/6bdfd225467f008a100f7f618cff1358148c83a1), superseded by upstream [`591d2c247`](https://github.com/steveicarus/iverilog/commit/591d2c2473148777bc516694e3e6f15f4c75ca20) | Fork initially rejected X/Z dimensions with SV-error/Verilog-warning split; upstream now always errors with `Dimensions must be a constant with no unknown or high-Z bits`. Master at [`2c098f871`](https://github.com/apullin/aiverilog/commit/2c098f87177cf3f4cd18f0d9785fccca70946751) unwinds the fork phrasing and adopts upstream's diagnostic; `fix/upstream-1140` retained as artifact and `br_gh1140` golds updated. |
| [#1326](https://github.com/steveicarus/iverilog/issues/1326) | [#50](https://github.com/apullin/aiverilog/issues/50), [PR #51](https://github.com/apullin/aiverilog/pull/51), [`adcf11430`](https://github.com/apullin/aiverilog/commit/adcf114309b78feb90622ad2c7e88a1752727c63) | Adopt the requested convention of starting edge-sensitive `always` listeners before ordinary time-zero `initial` processes. |
| [#1184](https://github.com/steveicarus/iverilog/issues/1184) | [#52](https://github.com/apullin/aiverilog/issues/52), [PR #53](https://github.com/apullin/aiverilog/pull/53), [`246e33cea`](https://github.com/apullin/aiverilog/commit/246e33cea2a411322bb2ec5c7af04c570a5269de) | Schedule multi-bit module-path transitions per bit and coalesce equal-time arrivals. |
| [#1048](https://github.com/steveicarus/iverilog/issues/1048) | [#54](https://github.com/apullin/aiverilog/issues/54), [PR #55](https://github.com/apullin/aiverilog/pull/55), [`01508e56c`](https://github.com/apullin/aiverilog/commit/01508e56cab8ff313a1054ea67b3f26e4fd49c6a) | Preserve a common enum type through conditional expressions, including X/Z selection. |
| [#678](https://github.com/steveicarus/iverilog/issues/678) | [#56](https://github.com/apullin/aiverilog/issues/56), [PR #57](https://github.com/apullin/aiverilog/pull/57), [`9d5320cff`](https://github.com/apullin/aiverilog/commit/9d5320cff75cc9fde1b2062d70c54f5b3bd3b317) | Coalesce transient event propagation from atomic blocking vector assignments. A broad prototype with roughly 40% ordinary-fanout slowdown was rejected; the landed path is narrowly selected, leaves existing J-Core timing neutral, and passed all 12 CI jobs. NBA concatenations and unpacked-array word stores remain separate follow-ups. |
| [#1466](https://github.com/steveicarus/iverilog/issues/1466) | [PR #68](https://github.com/apullin/aiverilog/pull/68), repaired as [`728323077`](https://github.com/apullin/aiverilog/commit/728323077e2c184e387db51451d3bbbe7908b58d) | Guard only: propagate packed-index calculation failure so the non-ANSI unpacked-array port case receives a source diagnostic instead of aborting in `eval_part_select_`. Full support for the legal construct remains separate work. |
| [#1441](https://github.com/steveicarus/iverilog/issues/1441) | [PR #69](https://github.com/apullin/aiverilog/pull/69), replaced by [`b459ba79e`](https://github.com/apullin/aiverilog/commit/b459ba79ebafd46120dc14415fc42dac9cb1e781) | Carry explicit one-bit packed-vector identity through the VVP format and `__vpiSignal`, including nets, variables, aliases, and net-array words. VPI no longer guesses from a signal name or a collapsed range. |

## Elided Or Already Present

These reports were reviewed, but retaining an Aiverilog patch would duplicate
upstream or existing behavior.

| Upstream report | Disposition |
| --- | --- |
| [#670](https://github.com/steveicarus/iverilog/issues/670) | Elided. Upstream's broader type-identifier grammar work in [PR #1439](https://github.com/steveicarus/iverilog/pull/1439) includes explicit `br_gh670` coverage. The old fork patch also broadened an unrelated grammar rule. |
| [#1217](https://github.com/steveicarus/iverilog/issues/1217) / [#1224](https://github.com/steveicarus/iverilog/issues/1224) | Elided. Upstream [PR #1367](https://github.com/steveicarus/iverilog/pull/1367) fixes both with equivalent regressions; the fork's old test-only commits were dropped. |
| [#1140](https://github.com/steveicarus/iverilog/issues/1140) | Superseded. Upstream [`591d2c247`](https://github.com/steveicarus/iverilog/commit/591d2c2473148777bc516694e3e6f15f4c75ca20) now provides `br_gh1140a/b/c` with `Dimensions must be a constant ...`; fork's `br_gh1140` phrasing and `br_gh1140_v` warning were unwound in [`2c098f871`](https://github.com/apullin/aiverilog/commit/2c098f87177cf3f4cd18f0d9785fccca70946751)/[`616095528`](https://github.com/apullin/aiverilog/commit/616095528436836be877743ca6e03ed5819ca7f7); `early_sig_elab3` gold also updated. |
| [#1449](https://github.com/steveicarus/iverilog/issues/1449) | Elided. The integrated upstream tree already turns the reported assertion into a clean unsupported-type diagnostic. Full unpacked-array value support is the subject of upstream [PR #1450](https://github.com/steveicarus/iverilog/pull/1450); the fork does not carry a brittle golden test for the temporary diagnostic. |
| [#1459](https://github.com/steveicarus/iverilog/issues/1459) | Already present through the [#1298](https://github.com/steveicarus/iverilog/issues/1298) implementation in [`628173e8c`](https://github.com/apullin/aiverilog/commit/628173e8c8e4524e12f1d2f85fe73957ac8f9e51). [`ca67eb43a`](https://github.com/apullin/aiverilog/commit/ca67eb43aff07dd0faa97a0035cf259cd31dc8a8) adds a runtime regression that invokes the task and checks both conditional arms. |
| [#1128](https://github.com/steveicarus/iverilog/issues/1128) | Already present. VVP accepts `exit` as a prompt synonym for `finish`. |
| [#1194](https://github.com/steveicarus/iverilog/issues/1194) | No clear missing behavior after reviewing existing extended-argument support and documentation. No fork patch. |

## Reviewed And Deferred

This is not an exhaustive copy of the upstream tracker. It records reports that
were investigated enough to make a deliberate no-patch decision or to identify
the next prerequisite.

| Upstream report | Current fork disposition |
| --- | --- |
| [PR #1406](https://github.com/steveicarus/iverilog/pull/1406) | Do not import as written: its second `fopen64` probe reuses the cached negative result, and the vendored `config.guess` change should be handled upstream. |
| [#1108](https://github.com/steveicarus/iverilog/issues/1108) | Reproduced in `vlog95`; a correct fix needs a narrow inherited-context dependency fingerprint to avoid specializing every descendant module. |
| [#1206](https://github.com/steveicarus/iverilog/issues/1206) | Deferred pending a precise policy for later source definitions overriding library definitions. |
| [#907](https://github.com/steveicarus/iverilog/issues/907) | Small implementation, unresolved public API choice for suppressing `$writemem*` address comments. |
| [#923](https://github.com/steveicarus/iverilog/issues/923) | Prototype dropped: a default warning changed many existing regression outputs for little value. Revisit only with an opt-in warning class or policy decision. |
| [#1233](https://github.com/steveicarus/iverilog/issues/1233) | `$readmem*` files are runtime inputs and cannot be made accurate compiler `-M` dependencies without a separate facility. |
| [#1284](https://github.com/steveicarus/iverilog/issues/1284) | Better `input reg` diagnostics depend on several language-generation cases; not a safe message-only edit. |
| [#1139](https://github.com/steveicarus/iverilog/issues/1139) | Undefined-package diagnostics are entangled with lexer/parser handling of type identifiers and packages. |
| [#1442](https://github.com/steveicarus/iverilog/issues/1442) | Still unsupported. Passing string parameters through an override cannot bind the source parameter, and concatenated string overrides still reach an unimplemented typed-expression path. PRs #70 and #73 only captured those failures and were removed from `master`; a real fix needs string-valued parameter expressions to elaborate in the override scope. |
| [#560](https://github.com/steveicarus/iverilog/issues/560) | Deep `tranif1` strength/storage semantics; not accepted as a quick patch. |
| [#449](https://github.com/steveicarus/iverilog/issues/449) | `$finish(2)` accounting needs a cross-platform definition of process and dump-thread resource reporting. |
| [#284](https://github.com/steveicarus/iverilog/issues/284) | Large numeric module attributes require a target-API representation decision beyond native `long`. |

## Fork Work Not Derived From Upstream Issues

Upstream reconciliation must also preserve fork-native work that has no issue
number to match:

- The sanitizer-driven correctness audit between
  [`593fce299...ba3413374`](https://github.com/apullin/aiverilog/compare/593fce299eb3ed17aa6234d680a8133a4b878ec3...ba34133749a128f8cdd790dfc3bba191a4f92235)
  hardens command-file ingestion, VPI formatting and scanning, vector
  conversions, arithmetic, and unsupported parser paths.
- [PR #23](https://github.com/apullin/aiverilog/pull/23) is the profile-first
  performance campaign, including benchmark oracles and the optimized build
  harness.
- [PR #26](https://github.com/apullin/aiverilog/pull/26) adds the separately
  measured constant nonblocking-assignment fusion left after PR #23.

These changes should be compared semantically against incoming upstream work;
the absence of a matching upstream issue number is not evidence that they can
be discarded.

## Pull Request Boundaries

Keep upstream-compatible work easy to inspect and easy to exchange:

- If a fix maps one-to-one, or nearly one-to-one, to an upstream issue or
  commit, land it as its own minimal fork PR. Preserve original authorship and
  identify the upstream source when importing an existing patch.
- Combine reports only when they demonstrably share one cause and one code
  path, and splitting the implementation would be artificial.
- Keep fork organization, harness work, broad cleanups, and structural
  refactors out of those fix PRs. Such work belongs in a separate, explicitly
  scoped major PR with its own validation and review story.

This separation keeps small fixes candidates for upstream exchange even when
the larger Aiverilog organization or architecture intentionally diverges.

## Reconciliation Procedure

For new upstream-derived work:

1. Fetch upstream before reproducing the report, and test against the current
   combined `master`.
2. Create a fork issue only after the report is reproduced and a bounded fix is
   accepted.
3. Put both upstream and fork issue URLs in the implementation commit; keep one
   logical issue per commit unless two warnings share one cause and code path.
4. Require a focused regression, complete local suites, and the fork's full CI
   matrix before merging.
5. Add or update the corresponding row here only after the fork PR merges.

For a new upstream sync:

1. Inventory every incoming upstream commit and compare overlapping paths with
   retained fork deltas in this ledger.
2. Run focused fork regressions against pure upstream when an incoming change
   might supersede a row.
3. Keep useful regressions even if upstream independently supplies an
   equivalent implementation.
4. Merge upstream normally. Do not rewrite published fork history except for an
   explicitly authorized repair; preserve the old tip and document the rewrite.
5. Run complete functional suites and performance output oracles, then update
   the integrated upstream hash and any newly elided rows above.
