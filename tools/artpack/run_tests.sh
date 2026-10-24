#!/bin/sh
# Host tests for tools/artpack (generated images only).
set -e
cd "$(dirname "$0")"
exec python3 test_artpack.py
