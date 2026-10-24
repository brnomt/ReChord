#!/usr/bin/env bash
# Verify the standalone AP5M build from a clean worktree, twice.
#
# Each worktree is created at REF (default HEAD) and contains only tracked
# files: stock/ and community/ are gitignored, so their absence here is exactly
# the clean-clone condition. Both worktrees must build the 5 MB image and the
# two images must be byte-identical (the packer is deterministic).
#
# The byte-exact boot-confirmed golden is still available, but explicit:
#   EXPECT_SHA256=<sha256> make clean-clone-check
# It is not hardcoded because every firmware commit changes the image; updating
# the golden is a decision made after flashing, not a repository invariant.
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
REF="${1:-HEAD}"
EXPECT="${EXPECT_SHA256:-}"
DIR_A="$(mktemp -d "${TMPDIR:-/tmp}/rechord-clean-a.XXXXXX")"
DIR_B="$(mktemp -d "${TMPDIR:-/tmp}/rechord-clean-b.XXXXXX")"

cleanup() {
    for d in "$DIR_A" "$DIR_B"; do
        git -C "$ROOT" worktree remove --force "$d" >/dev/null 2>&1 || rm -rf "$d"
    done
}
trap cleanup EXIT

# Prints the image SHA-256 on stdout; everything else goes to stderr so the
# caller can capture only the hash.
build_clean() {
    local dir="$1" label="$2" log="$1/build.log"

    git -C "$ROOT" worktree add --detach "$dir" "$REF" >/dev/null
    echo "clean worktree $label: $dir ($REF)" >&2

    if [[ -e "$dir/stock" ]]; then
        echo "ERROR: stock/ present in clean worktree $label" >&2
        return 1
    fi
    if [[ -e "$dir/community" ]]; then
        echo "ERROR: community/ present in clean worktree $label" >&2
        return 1
    fi
    echo "  stock/: absent; community/: absent" >&2

    if ! make -C "$dir" standalone-ap5m >"$log" 2>&1; then
        echo "ERROR: standalone-ap5m failed in worktree $label" >&2
        tail -n 80 "$log" >&2
        return 1
    fi
    grep -E "fw1 payload|loader code base|RESULT: PASS" "$log" >&2 || true

    sha256sum "$dir/build/ReChord_AP5M.IMG" | awk '{print $1}'
}

SHA_A="$(build_clean "$DIR_A" A)"
SHA_B="$(build_clean "$DIR_B" B)"

echo "clean-clone A: $SHA_A"
echo "clean-clone B: $SHA_B"

if [[ "$SHA_A" != "$SHA_B" ]]; then
    echo "ERROR: the standalone AP5M build is not reproducible" >&2
    echo "  A: $SHA_A" >&2
    echo "  B: $SHA_B" >&2
    exit 1
fi

if [[ -n "$EXPECT" && "$SHA_A" != "$EXPECT" ]]; then
    echo "ERROR: clean-clone AP5M SHA does not match EXPECT_SHA256" >&2
    echo "  built:  $SHA_A" >&2
    echo "  expect: $EXPECT" >&2
    exit 1
fi

if [[ -n "$EXPECT" ]]; then
    echo "PASS: clean-clone standalone AP5M $SHA_A (matches EXPECT_SHA256)"
else
    echo "PASS: clean-clone standalone AP5M is reproducible ($SHA_A)"
fi
