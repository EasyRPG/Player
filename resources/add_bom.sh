#!/bin/bash
echo "\xEF\xBB\xBF" > $2
cat $1 >> $2
