# Workflow (this fork only)

## Remotes

```bash
git remote -v
# origin    https://github.com/muhammadjawadkhan/iverilog-uvm.git
# upstream  https://github.com/steveicarus/iverilog.git
```

## Do

- Push branches to **`origin`** (`muhammadjawadkhan/iverilog-uvm`)
- Open PRs against **this fork’s** `master` (or `development` if created later)
- Use one feature per branch: `feat/param-classes`, `feat/assoc-array`, …
- Optionally `git fetch upstream` and merge upstream bugfixes into this fork
- Update [STATUS.md](STATUS.md) when a feature lands

## Do not

- Open PRs to **`steveicarus/iverilog`** for UVM / this track’s compiler work
- Mix UVM commits into unrelated upstream contribution branches on other remotes
- Claim full UVM 1.2 support prematurely

## Example

```bash
git checkout master
git pull origin master
git checkout -b feat/param-classes
# ... work ...
git push -u origin feat/param-classes
gh pr create --repo muhammadjawadkhan/iverilog-uvm --base master --head feat/param-classes
```
