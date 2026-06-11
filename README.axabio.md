# AxaBio OpenBLT Fork

This is AxaBio's fork of [feaser/openblt](https://github.com/feaser/openblt), the
OpenBLT bootloader. It carries a small series of AxaBio-specific bootloader patches
on top of the upstream code.

> This document explains the **branch model and how to keep the fork in sync with
> upstream**. It is intended for developers working on the bootloader (Henry, Koen).
> For the upstream OpenBLT documentation, see the original `README.md`.

---

## Branch model

There are two branches that matter:

| Branch | What it is | Rule |
|---|---|---|
| `master` | A clean **mirror of upstream** `feaser/openblt`. Only feaser's commits live here. | **Never commit your own work here.** Keep it pristine so upstream stays easy to pull. |
| `axabio-patches` | Our **production line**: upstream + AxaBio patches. | **Build and flash from here.** All AxaBio work goes on this branch. |

The point of this split is to keep `master` as a pure upstream copy, so feaser updates
can always be pulled cleanly, while our changes stay isolated and auditable on
`axabio-patches`.

> **Why aren't our patches on `master`?** Because a branch only contains the commits
> that were made on it. We deliberately put our work on `axabio-patches` instead of
> `master` to keep the upstream mirror clean. "It's our fork" does not mean "our
> `master` has our work" — a fork is just a copy of feaser's repo; forking moves
> nothing onto `master` by itself.

### What our patches add

The AxaBio patches on top of upstream (`master..axabio-patches`):

- Read slave ID from the `device_config` flash page (AxaBio XCP bridge)
- Make `DEVICE_CONFIG_FLASH_ADDR` overridable via a `-D` compiler flag
- UART4 / UART5 channel support for the axaFront / axaBack bootloader
- Runtime USART channel override (Modbus RTU)
- Host tool: smaller write chunk + longer T6 timeout for bridge compatibility

To see the exact current delta:

```bash
git log --oneline master..axabio-patches
```

---

## Cloning the fork

The fork is **public**, so no credentials are needed — clone over HTTPS and check out
our branch in one step:

```bash
git clone -b axabio-patches https://github.com/MarkMichiels/openblt.git
cd openblt
```

The `-b axabio-patches` flag clones **and** checks out our patch line, so the files on
disk are the AxaBio version — ready to build. (A plain `git clone` would check out the
default branch `master` = stock feaser, **without** our patches.)

If you cloned without `-b` and need to switch:

```bash
git checkout axabio-patches
```

---

## Keeping up to date with upstream (feaser)

We track upstream by **merging** feaser into `axabio-patches` — not rebasing. Merging
keeps history stable (no force-push), which matters because this repo is consumed as a
git submodule elsewhere and is cloned onto build machines: a plain `git pull` keeps
working for everyone.

### One-time setup (fresh clone only)

A fresh clone only knows `origin` (our fork). Add the upstream remote once:

```bash
git remote add upstream https://github.com/feaser/openblt.git
```

### The update workflow

```bash
git checkout axabio-patches
git fetch upstream                 # get the latest feaser commits
git merge upstream/master          # fold upstream into our line
# -> resolve any conflicts, build + test the firmware, then:
git push origin axabio-patches
```

**You keep full control:** you decide when to pull upstream, you resolve any conflicts
deliberately, and you test the firmware before pushing.

### Do I have to re-apply our patches each time? — No.

Our patches are permanent commits on `axabio-patches`; they stay where they are. A merge
folds feaser's new work *around* them. Git only asks you to intervene where feaser
touched the **same lines** as one of our patches — and you resolve that **once**; the
resolution is recorded in the merge commit, so the same conflict never comes back.

That conflict resolution is the only unavoidable work, and it only happens where our
code and feaser's code overlap. If there is no overlap, the merge is fully automatic.

> Optionally, also keep `master` mirroring upstream so "what is pure upstream" stays
> visible:
> ```bash
> git checkout master
> git fetch upstream
> git merge --ff-only upstream/master
> git push origin master
> git checkout axabio-patches
> ```
> This is optional — the merge workflow above works directly from `upstream/master`.

---

## Used as a submodule (bump the pointer after you push)

This fork is consumed as a **git submodule** inside a parent repository. The parent does
**not** track our branch — it pins one specific commit of `axabio-patches` (a "gitlink").

That means pushing new commits here is **not enough**: the parent keeps pointing at the
old commit until someone **bumps the submodule pointer** and commits that in the parent.
Until then, the rest of the team still builds the old bootloader.

So the full flow after changing the bootloader is **two commits in two repos**:

```bash
# 1. land your change in THIS fork
git checkout axabio-patches
# ... your commits ...
git push origin axabio-patches

# 2. bump the pointer in the PARENT repo so everyone picks it up
cd <parent-repo>
git -C path/to/openblt checkout axabio-patches
git -C path/to/openblt pull
git add path/to/openblt                 # stages the new submodule commit
git commit -m "bump openblt submodule to axabio-patches tip"
git push
```

If the parent shows `modified: path/to/openblt` after you switch this submodule to a
branch tip, that is exactly this: the working submodule is ahead of the pinned commit.
Either bump the pointer (step 2) or, if you are only building locally and don't want to
move the pin yet, leave it — it is the submodule pointer, not a change to the parent's
own files.

## Building and flashing

Build and flash from the `axabio-patches` branch. See the upstream OpenBLT documentation
(`README.md` and `doc/`) for the build and flashing procedure for each target.
