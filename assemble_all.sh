#!/usr/bin/env bash
set -e

make assembler

# Expand .am files safely (portable, no bashisms)
set -- *.am
if [ "$1" = "*.am" ]; then
  echo "No .am files found"
  exit 0
fi

for f in "$@"; do
  echo "== $f =="
  ./assembler "$f" || true
done

echo
echo "Generated files:"
ls -l *.ob *.ent *.ext 2>/dev/null || true

