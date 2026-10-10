#!/usr/bin/env python3
"""
fix_include_case.py — Make #include directives match on-disk header names.

The RKNanoD SDK tree was authored on Windows (case-insensitive filesystems),
so directives like `#include "Macro.h"` target a file actually named
`macro.h`, and some use backslash paths (`..\\ImageInclude\\image_main.h`).
Both break on Linux. This script rewrites ONLY #include lines:

  * basename matched case-insensitively to exactly one header  -> exact name
  * backslash paths normalized to forward slashes
  * relative paths resolved against the including file's directory
    (case-insensitive); if the relative target does not exist but the
    basename is unique in the tree, the directive is reduced to the basename
    (the build adds every header directory to the include path).

Nothing else in the source is touched. Re-runnable (idempotent).

Usage:
    python3 tools/fix_include_case.py [--root DIR] [--dry-run]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

INCLUDE_RE = re.compile(r'^(\s*#\s*include\s+)([<"])([^>"]+)([>"])(.*)$')


def index_headers(root: Path) -> dict[str, list[Path]]:
    idx: dict[str, list[Path]] = {}
    for f in root.rglob("*.h"):
        idx.setdefault(f.name.lower(), []).append(f)
    return idx


def resolve_target(f: Path, inc: str, idx: dict[str, list[Path]], root: Path) -> str | None:
    """Return the corrected include string, or None if it already resolves."""
    norm = inc.replace("\\", "/")
    # 1) exact relative resolution from the including file's directory
    if (f.parent / norm).exists():
        return norm if norm != inc else None
    # 2) case-insensitive resolution of the relative path
    cur = f.parent
    parts = norm.split("/")
    walked = []
    ok = True
    for part in parts[:-1]:
        match = next((d for d in cur.iterdir() if d.name.lower() == part.lower()), None)
        if match is None:
            ok = False
            break
        walked.append(match.name)
        cur = match
    if ok:
        leaf = next((p for p in cur.iterdir() if p.name.lower() == parts[-1].lower()), None)
        if leaf is not None:
            fixed = "/".join(walked + [leaf.name])
            return fixed if fixed != inc else None
    # 3) basename anywhere in the tree -> bare name (build adds -I dirs).
    # Ambiguous basenames (two Fade.h etc.) resolve to the nearest candidate
    # relative to the including file: SDK headers shadow by subsystem.
    cands = idx.get(Path(norm).name.lower(), [])
    if len(cands) == 1:
        name = cands[0].name
        return name if name != inc else None
    if len(cands) > 1:
        def dist(p: Path) -> tuple[int, int]:
            try:
                rel = p.relative_to(f.parent)
                return (0, len(rel.parts))
            except ValueError:
                return (1, len(f.parent.relative_to(root).parts) + len(p.relative_to(root).parts))
        best = min(cands, key=dist)
        name = best.name
        return name if name != inc else None
    return None


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="firmware", help="tree to fix (default: firmware)")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    root = Path(args.root).resolve()
    if not root.is_dir():
        sys.exit(f"not a directory: {root}")
    idx = index_headers(root)

    changed_files = 0
    changed_lines = 0
    unresolved: list[tuple[Path, int, str]] = []
    for f in sorted(root.rglob("*")):
        if f.suffix.lower() not in (".c", ".h"):
            continue
        lines = f.read_text(errors="replace").splitlines(keepends=True)
        out = []
        dirty = False
        for i, line in enumerate(lines, 1):
            m = INCLUDE_RE.match(line.rstrip("\r\n"))
            if not m:
                out.append(line)
                continue
            head, openq, inc, closeq, tail = m.groups()
            fixed = resolve_target(f, inc, idx, root)
            if fixed is None:
                base = Path(inc.replace("\\", "/")).name
                exact = any(p.name == base for p in idx.get(base.lower(), []))
                system = base in {"stdio.h", "stddef.h", "stdlib.h", "string.h",
                                  "math.h", "stdbool.h", "stdint.h", "limits.h"}
                if not (f.parent / inc.replace("\\", "/")).exists() and not exact and not system:
                    unresolved.append((f, i, inc))
                out.append(line)
                continue
            eol = "\r\n" if line.endswith("\r\n") else "\n" if line.endswith("\n") else ""
            out.append(f"{head}{openq}{fixed}{closeq}{tail}{eol}")
            dirty = True
            changed_lines += 1
        if dirty:
            changed_files += 1
            if not args.dry_run:
                f.write_text("".join(out))

    mode = "would fix" if args.dry_run else "fixed"
    print(f"{mode} {changed_lines} include line(s) in {changed_files} file(s)")
    if unresolved:
        print(f"unresolved includes (missing headers, not case issues): {len(unresolved)}")
        for f, i, inc in unresolved[:15]:
            print(f"  {f.relative_to(root.parent)}:{i}: {inc}")


if __name__ == "__main__":
    main()
