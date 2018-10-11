#!/bin/sh
#
# generates a short changelog from git merges since last release
# by carstene1ns 2017, released in the public domain

release=`git describe --abbrev=0 --tags`
hashes=`git rev-list $release..master --merges --simplify-merges --reverse --max-count=5`

echo "Latest 5 changes since $release release:"
git show -s --format="- %b" $hashes

echo "Changes in the last release: https://blog.easyrpg.org/category/release/"
