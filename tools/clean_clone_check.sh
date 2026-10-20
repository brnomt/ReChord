#!/usr/bin/env bash
# Verify the standalone AP5M build from a clean worktree.
#
# The worktree contains only tracked files: stock/ and community/ are
# gitignored, so their absence here is exactly the clean-clone condition.
# The build must produce the boot-confirmed AP5M image byte-for-byte.
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
REF="${1:-HEAD}"
DIR="$(mktemp -d "${TMPDIR:-/tmp}/rechord-clean.XXXXXX")"

cleanup() {
    git -C "$ROOT" worktree remove --force "$DIR" >/dev/null 2>&1 || rm -rf "$DIR"
}
trap cleanup EXIT

git -C "$ROOT" worktree add --detach "$DIR" "$REF" >/dev/null
echo "clean worktree: $DIR ($REF)"

test ! -e "$DIR/stock"    || { echo "ERROR: stock/ present in clean worktree" >&2; exit 1; }
test ! -e "$DIR/community" || { echo "ERROR: community/ present in clean worktree" >&2; exit 1; }
echo "stock/: absent; community/: absent"

make -C "$DIR" standalone-ap5m

SHA="$(sha256sum "$DIR/build/ReChord_AP5M.IMG" | awk '{print $1}')"
EXPECT="${EXPECT_SHA256:-c2238cb388b14d6bfd984da9328ca0005831b90d42cf4b5be77617b48babb93d}"
if [[ "$SHA" != "$EXPECT" ]]; then
    echo "ERROR: clean-clone AP5M SHA mismatch" >&2
    echo "  built:  $SHA" >&2
    echo "  expect: $EXPECT" >&2
    exit 1
fi

echo "PASS: clean-clone standalone AP5M $SHA"
