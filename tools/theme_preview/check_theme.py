#!/usr/bin/env python3
"""check_theme.py — structural limits of a ReChord theme package.

Mirrors the bounds the firmware enforces when it loads a package
(firmware/theme/theme_skin.c, theme_pkg.c, theme_assets.c):

  skin text        4096 B
  sections         52 (including [skin] and [vars*]; hidden sections count)
  properties       12 per section (this is why [vars] may need [vars2])
  value length     144 B
  vars             20 total, expression <= 112 B, strings <= 24 B
  assets           8, <= 64x64, stride <= 8 B, cache <= 1024 B
  list rows        browser <= 12

Usage: check_theme.py <themes-root> <id> [<id> ...]
Exit status is non-zero when any package violates a hard limit.
"""
import os
import re
import sys

SKIN_TEXT_MAX = 4096
SKIN_SECTIONS = 52
SKIN_PROPS = 12
SKIN_PROP_MAX = 144
SKIN_VARS = 20
SKIN_VAR_EXPR_MAX = 112
SKIN_VAR_STR_MAX = 24
ASSET_MAX = 8
ASSET_DIM_MAX = 64
ASSET_STRIDE_MAX = 8
ASSET_CACHE = 1024
BROWSER_ROWS_MAX = 12
SCREENS = ["home.skin", "now_playing.skin", "library.skin",
           "library_cat.skin", "browser.skin", "settings.skin"]


def kv_lines(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read().splitlines()


def parse_manifest(path):
    cfg, assets = {}, []
    for line in kv_lines(path):
        s = line.strip()
        if not s or s[0] in "#;" or s.startswith("["):
            continue
        if "=" not in s:
            continue
        k, v = s.split("=", 1)
        k, v = k.strip(), v.strip()
        if k.startswith("asset_"):
            parts = [p.strip() for p in v.split(",")]
            assets.append((k[6:], parts[0], int(parts[1]), int(parts[2])))
        else:
            cfg[k] = v
    return cfg, assets


def split_sections(text):
    """Return [[name, raw prop count]] (the C loader keeps only SKIN_PROPS)."""
    secs = []
    cur = None
    for line in text.splitlines():
        s = line.strip()
        if not s or s[0] in "#;":
            continue
        if s.startswith("["):
            close = s.find("]")
            if close > 0:
                cur = [s[1:close], 0]
                secs.append(cur)
                continue
        if cur is None:
            continue
        if "=" in s:
            cur[1] += 1   # counted raw: the C loader silently drops the 13th+
    return secs


def check_skin(root, ident, name, errors, warn):
    path = os.path.join(root, ident, name)
    if not os.path.exists(path):
        errors.append(f"{name}: missing")
        return
    raw = open(path, "rb").read()
    if len(raw) > SKIN_TEXT_MAX:
        errors.append(f"{name}: {len(raw)} B > {SKIN_TEXT_MAX}")
    text = raw.decode("utf-8", "replace")
    secs = split_sections(text)
    if len(secs) > SKIN_SECTIONS:
        errors.append(f"{name}: {len(secs)} sections > {SKIN_SECTIONS}")

    vars_total = 0
    for sec_name, nprops in secs:
        if re.fullmatch(r"vars\d*", sec_name):
            vars_total += nprops
        if nprops > SKIN_PROPS:
            errors.append(f"{name}: [{sec_name}] {nprops} props > {SKIN_PROPS}")
    if vars_total > SKIN_VARS:
        errors.append(f"{name}: {vars_total} vars > {SKIN_VARS}")

    # Build each section's key/value map as the C loader would see it.
    sections, cur = [], None
    for line in text.splitlines():
        s = line.strip()
        if not s or s[0] in "#;":
            continue
        if s.startswith("["):
            close = s.find("]")
            if close > 0:
                cur = {"__name": s[1:close], "__n": 0}
                sections.append(cur)
                continue
        if cur is None or "=" not in s or cur["__n"] >= SKIN_PROPS:
            continue
        k, v = s.split("=", 1)
        cur[k.strip()] = v.strip()
        cur["__n"] += 1

    for sec in sections:
        is_vars = re.fullmatch(r"vars\d*", sec["__name"]) is not None
        for k, v in sec.items():
            if k.startswith("__"):
                continue
            if len(v) > SKIN_PROP_MAX:
                errors.append(f"{name}: {sec['__name']}.{k} value {len(v)} B > "
                              f"{SKIN_PROP_MAX}")
            if v.count("(") != v.count(")"):
                errors.append(f"{name}: {sec['__name']}.{k}: unbalanced ()")
            if v.count('"') % 2:
                errors.append(f"{name}: {sec['__name']}.{k}: unbalanced quotes")
            if is_vars:
                if len(v) > SKIN_VAR_EXPR_MAX:
                    errors.append(f"{name}: var {k} expr {len(v)} B > "
                                  f"{SKIN_VAR_EXPR_MAX}")
                for lit in re.findall(r'"([^"]*)"', v):
                    if len(lit) > SKIN_VAR_STR_MAX:
                        errors.append(f"{name}: var {k} string {len(lit)} B > "
                                      f"{SKIN_VAR_STR_MAX} (truncated)")

        if sec.get("type") == "list":
            for f in ("x", "y", "height", "rows", "selected"):
                val = sec.get(f)
                if val is not None and not re.fullmatch(r"-?\d+", val):
                    warn.append(f"{name}: list {f}={val} is not an integer "
                                "(the engine reads it literally)")
            if name == "browser.skin":
                rows = sec.get("rows")
                if rows and rows.isdigit() and int(rows) > BROWSER_ROWS_MAX:
                    errors.append(f"{name}: list rows={rows} > "
                                  f"{BROWSER_ROWS_MAX} (browser rejects)")


def check_package(root, ident):
    errors, warn = [], []
    base = os.path.join(root, ident)
    cfg_path = os.path.join(base, "theme.cfg")
    if not os.path.exists(cfg_path):
        print(f"FAIL {ident}: theme.cfg missing")
        return False
    cfg, assets = parse_manifest(cfg_path)
    if cfg.get("format") != "1":
        errors.append("theme.cfg: format must be 1")
    if cfg.get("home") not in ("skin", "menu", "companion"):
        errors.append("theme.cfg: home must be menu|skin")
    if cfg.get("home") in ("skin", "companion") and not cfg.get("skin"):
        errors.append("theme.cfg: home=skin requires skin=")

    # assets: count, geometry, file size, cache
    used = 0
    if len(assets) > ASSET_MAX:
        errors.append(f"assets: {len(assets)} > {ASSET_MAX}")
    for aname, apath, aw, ah in assets:
        stride = (aw + 7) // 8
        size = stride * ah
        used += size
        if len(aname) > 15:
            errors.append(f"asset_{aname}: name > 15 chars")
        if aw > ASSET_DIM_MAX or ah > ASSET_DIM_MAX:
            errors.append(f"asset_{aname}: {aw}x{ah} > {ASSET_DIM_MAX}")
        if stride > ASSET_STRIDE_MAX:
            errors.append(f"asset_{aname}: stride {stride} > {ASSET_STRIDE_MAX}")
        if ".." in apath:
            errors.append(f"asset_{aname}: path may not contain ..")
        ap = os.path.join(base, apath)
        if not os.path.exists(ap):
            errors.append(f"asset_{aname}: {apath} missing")
        elif os.path.getsize(ap) != size:
            errors.append(f"asset_{aname}: {os.path.getsize(ap)} B, expected "
                          f"{size} B for {aw}x{ah}")
    if used > ASSET_CACHE:
        errors.append(f"asset cache: {used} B > {ASSET_CACHE}")

    # the skins the manifest declares (plus the six canonical names)
    declared = set()
    for k, v in cfg.items():
        if k in ("skin", "skin_main") or k.startswith("skin_"):
            declared.add(v)
    declared |= set(SCREENS)
    for name in sorted(declared):
        if not name.endswith(".skin"):
            continue
        check_skin(root, ident, name, errors, warn)

    print(f"{'FAIL' if errors else 'PASS'} {ident}: "
          f"{len(assets)} assets / {used} B cache")
    for e in errors:
        print(f"  ERROR {e}")
    for w in warn:
        print(f"  warn  {w}")
    return not errors


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    root = sys.argv[1]
    ok = True
    for ident in sys.argv[2:]:
        ok = check_package(root, ident) and ok
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
