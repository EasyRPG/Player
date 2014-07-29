#!/bin/sh

cd $(dirname $0)

STYLE="{
BasedOnStyle: Google,
AccessModifierOffset: -4,
UseTab: ForIndentation,
TabWidth: 4,
IndentWidth: 4,
Standard: Cpp03,
PointerBindsToType: true,
ColumnLimit: 100,
IndentCaseLabels: false,
NamespaceIndentation: All,
SpacesBeforeTrailingComments: 1,
BreakConstructorInitializersBeforeComma: true,
}"

clang-format -i "-style=$STYLE" ../src/*.cpp ../src/*.h ../src/platform/*.cpp ../src/platform/*.h ../tests/*.cpp
