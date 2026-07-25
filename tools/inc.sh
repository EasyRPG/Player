find leasy -type f \( -name '*.h' -o -name '*.hpp' \) | sort |
while IFS= read -r file; do
  printf '#include "%s"\n' "$file"
done