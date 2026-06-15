#!/usr/bin/env python3
"""
cpp_debug_extractor.py
----------------------
Scans a C++ codebase (.cpp, .hpp, .h) and produces:
  1. A human-readable debug report  (cpp_debug.txt)
  2. A C++ header you can #include   (cpp_debug_index.hpp)
  3. Optionally a JSON dump          (--json)

Usage:
    python cpp_debug_extractor.py <path/to/codebase> [options]

Options:
    --out FILE      Base name for output files (default: cpp_debug)
    --json          Also write a .json dump
    --no-header     Skip generating the C++ header
    --verbose       Extra per-file details (entry list)
"""

import argparse
import json
import os
import re
import sys
from collections import Counter
from dataclasses import dataclass, asdict
from datetime import datetime
from pathlib import Path
from typing import List, Optional


# ═══════════════════════════════════════════════════════════════════════════════
# Terminal helpers
# ═══════════════════════════════════════════════════════════════════════════════

ANSI_RESET  = "\033[0m"
ANSI_BOLD   = "\033[1m"
ANSI_GREEN  = "\033[32m"
ANSI_CYAN   = "\033[36m"
ANSI_YELLOW = "\033[33m"
ANSI_GREY   = "\033[90m"
ANSI_RED    = "\033[31m"

def _supports_color() -> bool:
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

def c(code: str, text: str) -> str:
    return f"{code}{text}{ANSI_RESET}" if _supports_color() else text

def progress_bar(current: int, total: int, width: int = 28) -> str:
    frac   = current / total if total else 1.0
    filled = int(frac * width)
    bar    = "█" * filled + "░" * (width - filled)
    pct    = frac * 100
    return f"[{c(ANSI_GREEN, bar)}] {c(ANSI_BOLD, f'{pct:5.1f}%')}"

def print_progress(idx: int, total: int, filepath: str, found: int, base_dir: str):
    rel   = os.path.relpath(filepath, base_dir)
    # Truncate long paths from the left
    max_path = 45
    if len(rel) > max_path:
        rel = "…" + rel[-(max_path - 1):]
    bar   = progress_bar(idx, total)
    count = c(ANSI_CYAN, f"{found:3d} fn") if found else c(ANSI_GREY, "  0 fn")
    ctr   = c(ANSI_GREY, f"({idx}/{total})")
    print(f"  {bar}  {count}  {ctr}  {c(ANSI_GREY, rel)}", flush=True)


# ═══════════════════════════════════════════════════════════════════════════════
# Data model
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class FunctionInfo:
    name:        str
    return_type: str
    parameters:  str
    namespaces:  List[str]
    class_name:  Optional[str]
    is_method:   bool
    file:        str
    line:        int
    qualifiers:  List[str]
    kind:        str           # function | method | constructor | destructor


# ═══════════════════════════════════════════════════════════════════════════════
# Source preprocessing
# ═══════════════════════════════════════════════════════════════════════════════

_BLOCK_COMMENT = re.compile(r'/\*.*?\*/', re.DOTALL)
_LINE_COMMENT  = re.compile(r'//[^\n]*')
_STRING_LIT    = re.compile(r'"(?:\\.|[^"\\])*"')
_CHAR_LIT      = re.compile(r"'(?:\\.|[^'\\])*'")

def strip_comments(src: str) -> str:
    src = _BLOCK_COMMENT.sub(' ', src)
    src = _LINE_COMMENT.sub('', src)
    src = _STRING_LIT.sub('""', src)
    src = _CHAR_LIT.sub("''", src)
    return src

def build_line_index(raw: str) -> List[int]:
    offsets = [0]
    for i, ch in enumerate(raw):
        if ch == '\n':
            offsets.append(i + 1)
    return offsets

def offset_to_line(offsets: List[int], pos: int) -> int:
    lo, hi = 0, len(offsets) - 1
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if offsets[mid] <= pos:
            lo = mid
        else:
            hi = mid - 1
    return lo + 1


# ═══════════════════════════════════════════════════════════════════════════════
# Scope map  (namespace / class / struct nesting)
# ═══════════════════════════════════════════════════════════════════════════════

_SCOPE_RE = re.compile(
    r'\b(?P<kw>namespace|class|struct|union)\s+'
    r'(?:(?:final|alignas\s*\([^)]*\)|__attribute__\s*\(\([^)]*\)\))\s*)*'
    r'(?P<name>\w+)'
    r'(?:\s*:\s*[^{;]*)?'
    r'\s*\{',
    re.MULTILINE,
)

def extract_scope_map(src: str):
    opens  = {}
    closes = {}
    scope_opens = []

    for m in _SCOPE_RE.finditer(src):
        brace_pos = m.end() - 1
        scope_opens.append((brace_pos, m.group('kw'), m.group('name')))
    for pos, kw, name in scope_opens:
        opens[pos] = (kw, name)

    stack = []
    for i, ch in enumerate(src):
        if ch == '{':
            stack.append(i)
        elif ch == '}' and stack:
            op = stack.pop()
            closes[op] = i

    ranges = []
    for op, (kw, name) in sorted(opens.items()):
        cl = closes.get(op, len(src))
        ranges.append((op, cl, kw, name))
    return ranges

def get_context_at(scope_ranges, pos):
    namespaces, class_stack = [], []
    for (start, end, kw, name) in scope_ranges:
        if start < pos < end:
            if kw == 'namespace':
                namespaces.append(name)
            else:
                class_stack.append(name)
    return namespaces, class_stack


# ═══════════════════════════════════════════════════════════════════════════════
# Function / method extraction
# ═══════════════════════════════════════════════════════════════════════════════

_FUNC_RE = re.compile(
    r'''
    (?:^|(?<=[\n;{}]))
    [ \t]*
    (?P<quals>
        (?:(?:static|virtual|explicit|inline|friend|constexpr|consteval|
               constinit|extern|__forceinline|__declspec\([^)]*\)|
               \[\[[^\]]*\]\])\s+)*
    )
    (?P<ret>
        (?:const\s+|volatile\s+|unsigned\s+|signed\s+|long\s+|short\s+)*
        (?:[\w:<>,*&\s])+?
    )
    \s*
    (?P<scope>(?:\w+::)*)
    (?P<name>~?\w+)
    \s*
    (?P<tpl><[^>]*>)?
    \s*
    \((?P<params>[^)]*)\)
    \s*
    (?P<trailing>
        (?:const|volatile|override|final|noexcept(?:\([^)]*\))?|
           \s*->\s*[\w:<>&*\s]+|\s*=\s*(?:0|default|delete))*
    )
    \s*
    (?=[{;])
    ''',
    re.VERBOSE | re.MULTILINE,
)

_BLACKLIST_NAMES = {
    'if','else','for','while','do','switch','case','return',
    'catch','throw','new','delete','sizeof','alignof','decltype',
    'static_assert','assert','try','namespace','class','struct',
    'union','enum','typedef','using','template','typename',
    'public','private','protected',
}
_BLACKLIST_RET = re.compile(r'^\s*(?:if|else|for|while|do|switch|return|catch|throw|#\w+)\s*$')

_ALL_QUALS = ('static','virtual','explicit','inline','friend',
              'constexpr','consteval','extern','const','override',
              'final','noexcept','= 0','= default','= delete')

def parse_functions(filepath: str) -> List[FunctionInfo]:
    try:
        raw = Path(filepath).read_text(encoding='utf-8', errors='replace')
    except OSError as e:
        print(c(ANSI_RED, f"  [WARN] Cannot read {filepath}: {e}"), file=sys.stderr)
        return []

    line_offsets = build_line_index(raw)
    src_clean    = strip_comments(raw)
    scope_ranges = extract_scope_map(src_clean)

    results, seen = [], set()

    for m in _FUNC_RE.finditer(src_clean):
        name = m.group('name')
        if name in _BLACKLIST_NAMES:
            continue
        ret = (m.group('ret') or '').strip()
        if _BLACKLIST_RET.match(ret) or ret.startswith('#') or not ret:
            continue

        pos    = m.start()
        lineno = offset_to_line(line_offsets, pos)
        key    = (name, lineno)
        if key in seen:
            continue
        seen.add(key)

        namespaces, class_stack = get_context_at(scope_ranges, pos)
        scope_prefix = (m.group('scope') or '').rstrip('::')
        class_name   = class_stack[-1] if class_stack else (scope_prefix or None)

        quals_raw  = (m.group('quals') or '') + ' ' + (m.group('trailing') or '')
        qualifiers = [q for q in _ALL_QUALS if q in quals_raw]

        if name.startswith('~'):
            kind = 'destructor'
        elif name == class_name:
            kind = 'constructor'
        elif class_name:
            kind = 'method'
        else:
            kind = 'function'

        params = re.sub(r'\s+', ' ', (m.group('params') or '').strip())
        ret    = re.sub(r'\s+', ' ', ret).strip()

        results.append(FunctionInfo(
            name=name, return_type=ret, parameters=params,
            namespaces=namespaces, class_name=class_name,
            is_method=kind in ('method','constructor','destructor'),
            file=filepath, line=lineno, qualifiers=qualifiers, kind=kind,
        ))

    return results


# ═══════════════════════════════════════════════════════════════════════════════
# Text report
# ═══════════════════════════════════════════════════════════════════════════════

_KIND_ICON = {'function':'⚙', 'method':'◈', 'constructor':'✦', 'destructor':'✕'}

def format_report(all_funcs: List[FunctionInfo], base_dir: str, scanned: int) -> str:
    W = 72
    lines = []
    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    lines += [
        "=" * W,
        "  C++ CODEBASE DEBUG REPORT",
        f"  Generated : {ts}",
        f"  Root      : {base_dir}",
        f"  Files     : {scanned}",
        f"  Entries   : {len(all_funcs)}",
        "=" * W, "",
    ]

    by_file: dict = {}
    for f in all_funcs:
        by_file.setdefault(f.file, []).append(f)

    for filepath in sorted(by_file.keys()):
        rel     = os.path.relpath(filepath, base_dir)
        entries = sorted(by_file[filepath], key=lambda x: x.line)

        lines += [f"┌─ FILE: {rel}", f"│  ({len(entries)} entries)", "│"]

        for e in entries:
            icon      = _KIND_ICON.get(e.kind, '?')
            ns_str    = '::'.join(e.namespaces) if e.namespaces else '(global)'
            class_str = e.class_name or '—'
            quals_str = ', '.join(e.qualifiers) if e.qualifiers else 'none'

            lines.append(f"│  {icon} [{e.kind.upper():12s}]  {e.name}  (line {e.line})")
            lines.append(f"│      Namespace   : {ns_str}")
            lines.append(f"│      Class/Struct: {class_str}")
            lines.append(f"│      Return type : {e.return_type}")
            lines.append(f"│      Parameters  : ({e.parameters})")
            lines.append(f"│      Qualifiers  : {quals_str}")
            lines.append("│")

        lines += ["└" + "─" * (W - 1), ""]

    kind_counts = Counter(f.kind for f in all_funcs)
    lines += [
        "=" * W,
        "  SUMMARY",
        "=" * W,
        f"  ⚙  Free functions : {kind_counts.get('function', 0)}",
        f"  ◈  Methods        : {kind_counts.get('method', 0)}",
        f"  ✦  Constructors   : {kind_counts.get('constructor', 0)}",
        f"  ✕  Destructors    : {kind_counts.get('destructor', 0)}",
        f"     Files scanned  : {scanned}",
        "",
    ]
    return '\n'.join(lines)

WRAP
# ═══════════════════════════════════════════════════════════════════════════════
# C++ header generator
# ═══════════════════════════════════════════════════════════════════════════════

def _escape_str(s: str) -> str:
    return s.replace('\\', '\\\\').replace('"', '\\"')

def generate_cpp_header(all_funcs: List[FunctionInfo], base_dir: str) -> str:
    ts      = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    guard   = "CPP_DEBUG_INDEX_HPP"
    entries = []

    for f in all_funcs:
        ns_joined    = '::'.join(f.namespaces)
        quals_joined = ','.join(f.qualifiers)
        file_rel     = _escape_str(os.path.relpath(f.file, base_dir))
        entries.append(
            f'    {{ "{_escape_str(f.name)}", "{_escape_str(f.return_type)}", '
            f'"{_escape_str(f.parameters)}", "{_escape_str(ns_joined)}", '
            f'"{_escape_str(f.class_name or "")}", '
            f'"{_escape_str(f.kind)}", '
            f'"{_escape_str(quals_joined)}", '
            f'"{file_rel}", {f.line} }}'
        )

    entries_str = ',\n'.join(entries)
    kind_counts = Counter(f.kind for f in all_funcs)

    return f"""\
#pragma once
/** Auto-generated file -- Thanks to Claude AI for this lol! */

#ifndef {guard}
#define {guard}

// ============================================================
//  AUTO-GENERATED by cpp_debug_extractor.py
//  Date   : {ts}
//  Root   : {base_dir}
//  Total  : {len(all_funcs)} entries across {len(set(f.file for f in all_funcs))} file(s)
// ============================================================
//
//  HOW TO USE
//  ----------
//  #include "cpp_debug_index.hpp"
//
//  // Iterate all entries:
//  for (const auto& e : leasy::dbg::kAllEntries)
//      std::cout << e.file << ":" << e.line << "  " << e.name << "\\n";
//
//  // Filter by kind ("function" | "method" | "constructor" | "destructor"):
//  auto methods = leasy::dbg::filter_by_kind("method");
//
//  // Find by name:
//  auto hits = leasy::dbg::find_by_name("myFunc");
//
// ============================================================

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <vector>

namespace leasy::dbg {{

// ── Entry struct ──────────────────────────────────────────────
struct Entry {{
    std::string_view name;
    std::string_view return_type;
    std::string_view parameters;
    std::string_view namespaces;   ///< "::" joined
    std::string_view class_name;   ///< empty for free functions
    std::string_view kind;         ///< function | method | constructor | destructor
    std::string_view qualifiers;   ///< "," joined list
    std::string_view file;         ///< path relative to project root
    int              line;
}};

// ── Full table ────────────────────────────────────────────────
inline constexpr Entry kAllEntries[] = {{
{entries_str}
}};

inline constexpr std::size_t kEntryCount = {len(all_funcs)};

// ── Counts per kind ───────────────────────────────────────────
inline constexpr std::size_t kFunctionCount    = {kind_counts.get('function',    0)};
inline constexpr std::size_t kMethodCount      = {kind_counts.get('method',      0)};
inline constexpr std::size_t kConstructorCount = {kind_counts.get('constructor', 0)};
inline constexpr std::size_t kDestructorCount  = {kind_counts.get('destructor',  0)};

// ── Helpers ───────────────────────────────────────────────────

/// Return all entries whose kind matches `k`.
inline std::vector<const Entry*> filter_by_kind(std::string_view k) {{
    std::vector<const Entry*> out;
    for (const auto& e : kAllEntries)
        if (e.kind == k) out.push_back(&e);
    return out;
}}

/// Return all entries whose name matches `n` (exact).
inline std::vector<const Entry*> find_by_name(std::string_view n) {{
    std::vector<const Entry*> out;
    for (const auto& e : kAllEntries)
        if (e.name == n) out.push_back(&e);
    return out;
}}

/// Return all entries whose file path contains `substr`.
inline std::vector<const Entry*> filter_by_file(std::string_view substr) {{
    std::vector<const Entry*> out;
    for (const auto& e : kAllEntries)
        if (e.file.find(substr) != std::string_view::npos) out.push_back(&e);
    return out;
}}

/// Return all entries that belong to a given namespace (exact match of any component).
inline std::vector<const Entry*> filter_by_namespace(std::string_view ns) {{
    std::vector<const Entry*> out;
    for (const auto& e : kAllEntries)
        if (e.namespaces.find(ns) != std::string_view::npos) out.push_back(&e);
    return out;
}}

/// Return all entries belonging to a given class/struct name.
inline std::vector<const Entry*> filter_by_class(std::string_view cls) {{
    std::vector<const Entry*> out;
    for (const auto& e : kAllEntries)
        if (e.class_name == cls) out.push_back(&e);
    return out;
}}

}}

#endif // {guard}
"""


# ═══════════════════════════════════════════════════════════════════════════════
# CLI
# ═══════════════════════════════════════════════════════════════════════════════

def collect_cpp_files(root: str) -> List[str]:
    exts = {'.cpp','.cc','.cxx','.c','.hpp','.hh','.hxx','.h'}
    found = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            if Path(fn).suffix.lower() in exts:
                found.append(os.path.join(dirpath, fn))
    return sorted(found)

def main():
    parser = argparse.ArgumentParser(
        description='Extract function/method debug info from a C++ codebase.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('path',       help='Root directory or single file to scan')
    parser.add_argument('--out',      default='cpp_debug',
                        help='Base name for output files (default: cpp_debug)')
    parser.add_argument('--json',     action='store_true',
                        help='Also write a JSON dump')
    parser.add_argument('--no-header',action='store_true',
                        help='Skip generating the C++ header')
    parser.add_argument('--verbose',  action='store_true',
                        help='Print every found entry during scan')
    args = parser.parse_args()

    root = os.path.abspath(args.path)
    if os.path.isfile(root):
        cpp_files = [root]
        base_dir  = os.path.dirname(root)
    elif os.path.isdir(root):
        cpp_files = collect_cpp_files(root)
        base_dir  = root
    else:
        print(c(ANSI_RED, f"Error: '{root}' is not a file or directory."), file=sys.stderr)
        sys.exit(1)

    if not cpp_files:
        print(c(ANSI_YELLOW, "No C++ source files found."), file=sys.stderr)
        sys.exit(1)

    total = len(cpp_files)
    print(f"\n{c(ANSI_BOLD, '  C++ Debug Extractor')}")
    print(f"  Root : {c(ANSI_CYAN, base_dir)}")
    print(f"  Files: {c(ANSI_BOLD, str(total))}\n")

    all_funcs: List[FunctionInfo] = []

    for idx, fp in enumerate(cpp_files, start=1):
        funcs = parse_functions(fp)
        all_funcs.extend(funcs)
        print_progress(idx, total, fp, len(funcs), base_dir)

        if args.verbose and funcs:
            for f in funcs:
                icon = _KIND_ICON.get(f.kind, '?')
                print(f"      {icon} {f.kind:12s}  {f.name}  (line {f.line})")

    print()  # blank line after progress

    # ── Outputs ─────────────────────────────────────────────────────────────
    out_base = args.out

    # 1. Text report
    txt_path = out_base + ".txt"
    Path(txt_path).write_text(
        format_report(all_funcs, base_dir, total), encoding='utf-8'
    )
    print(f"  {c(ANSI_GREEN, '✔')} Text report  →  {c(ANSI_BOLD, txt_path)}")

    # 2. C++ header
    if not args.no_header:
        hpp_path = out_base + "_index.hpp"
        Path(hpp_path).write_text(
            generate_cpp_header(all_funcs, base_dir), encoding='utf-8'
        )
        print(f"  {c(ANSI_GREEN, '✔')} C++ header   →  {c(ANSI_BOLD, hpp_path)}")

    # 3. JSON
    if args.json:
        json_path = out_base + ".json"
        Path(json_path).write_text(
            json.dumps([asdict(f) for f in all_funcs], indent=2), encoding='utf-8'
        )
        print(f"  {c(ANSI_GREEN, '✔')} JSON dump    →  {c(ANSI_BOLD, json_path)}")

    kind_counts = Counter(f.kind for f in all_funcs)
    print(f"\n  {c(ANSI_BOLD, str(len(all_funcs)))} entries total  "
          f"({kind_counts.get('function',0)} fn · "
          f"{kind_counts.get('method',0)} methods · "
          f"{kind_counts.get('constructor',0)} ctors · "
          f"{kind_counts.get('destructor',0)} dtors)\n")

_KIND_ICON = {'function':'⚙', 'method':'◈', 'constructor':'✦', 'destructor':'✕'}

if __name__ == '__main__':
    main()