#!/usr/bin/env bash

HEADER="header.txt"
ROOT="../src/leasy"

find "$ROOT" \
    \( -path "$ROOT/lua" -o -path "$ROOT/lua/*" \
       -o -path "$ROOT/thirdparty" -o -path "$ROOT/thirdparty/*" \) \
    -prune -o \
    \( \
        -name "*.c" -o \
        -name "*.cc" -o \
        -name "*.cpp" -o \
        -name "*.cxx" -o \
        -name "*.h" -o \
        -name "*.hh" -o \
        -name "*.hpp" -o \
        -name "*.hxx" \
    \) \
    -print0 |
while IFS= read -r -d '' file; do
    if grep -q " *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗" "$file"; then
        echo "Skipping $file"
        continue
    fi

    tmp=$(mktemp)
    cat "$HEADER" "$file" > "$tmp"
    mv "$tmp" "$file"
    echo "Updated $file"
done