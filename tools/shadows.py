"""Lists the globals that src/ and include/ view through more than one type:
declared under one type here and another there, cast to a third, or walked
as bytes with literal offsets.

A byte match cannot tell two views of one record apart, because every view
compiles to the same offset arithmetic, so a record that each file describes
in its own partial struct stays matched while the structs disagree. This
reads the sources only, builds nothing, and reports, per global, every type
it is declared or cast as, with the files that use each; --walks also counts
the places it is read as (byte *) plus an offset. The list is the work list
for unifying each record's views into one definition. A base that is cast by
design, such as the tag instance table, heads the list and can be left alone.

    python tools/shadows.py [--min N] [--walks] [g_<address> ...]
"""
import argparse
import collections
import glob
import os
import re
import sys

from xbe import ROOT

GLOBAL_NAME = r'g_[0-9a-f]{5,6}(?:_\w+)?'
# "extern s_foo *g_4e9bd4;", "s_foo g_4e9bd4[4];", "static s_foo *const g_4e9bd4 = ..."
DECLARATION = re.compile(r'^\s*(?:extern\s+|static\s+)?((?:struct\s+|class\s+)?[A-Za-z_]\w*(?:\s*::\s*\w+)*(?:\s+const)?)'
                         r'\s*(\*+\s*(?:const\s*)?|\s)\s*(' + GLOBAL_NAME + r')\s*(?:\[[^\]]*\])*\s*(?:=|;)', re.M)
# "(s_bar *)g_4e9bd4", "(s_bar const *)&g_4e9bd4[i]", "(byte *)g_4e9bd4 + 0x20"
CAST = re.compile(r'\(\s*((?:struct\s+)?[A-Za-z_]\w*(?:\s*::\s*\w+)*(?:\s+const)?)\s*(\*+)\s*(?:const\s*)?\)\s*'
                  r'\(?\s*&?\s*(' + GLOBAL_NAME + r')\b')
BYTE_TYPES = {'byte', 'char', 'unsigned char', 'uint8', 'int8', 'void'}
KEYWORDS = {'return', 'sizeof', 'typedef', 'else', 'if', 'const'}


def _normalize(text):
    return ' '.join(text.replace('struct ', '').replace('class ', '').split())


def views(path, text):
    """[(global, kind, type, pointer depth)] for one source: 'declared' from
    its declarations, 'cast' from the casts applied to the global."""
    out = []
    for m in DECLARATION.finditer(text):
        kind, stars, name = _normalize(m.group(1)), m.group(2).count('*'), m.group(3)
        if kind in KEYWORDS:
            continue
        out.append((name, 'declared', kind, stars))
    for m in CAST.finditer(text):
        kind, stars, name = _normalize(m.group(1)), len(m.group(2)), m.group(3)
        if kind in KEYWORDS:
            continue
        out.append((name, 'cast', kind, stars))
    return out


def collect(paths):
    """{global: {(kind, type, depth): {file, ...}}} across the given sources,
    with the byte-walk count per global as {global: n}."""
    found = collections.defaultdict(lambda: collections.defaultdict(set))
    walks = collections.Counter()
    for path in paths:
        with open(path, encoding='utf-8', errors='replace') as f:
            text = f.read()
        rel = os.path.relpath(path, ROOT)
        for name, kind, typ, stars in views(path, text):
            if typ in BYTE_TYPES and kind == 'cast':
                walks[name] += 1
                continue
            found[name][(kind, typ, stars)].add(rel)
    return found, walks


def shadows(found, minimum=2):
    """The globals viewed through at least minimum distinct types, most first."""
    out = []
    for name, by_view in found.items():
        types = {typ for _, typ, _ in by_view}
        if len(types) >= minimum:
            out.append((name, by_view))
    return sorted(out, key=lambda item: (-len({t for _, t, _ in item[1]}), item[0]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('names', nargs='*', help='globals to report (default: every one with several views)')
    ap.add_argument('--min', type=int, default=2, help='distinct types a global needs to be listed (default 2)')
    ap.add_argument('--walks', action='store_true', help='also count (byte *) walks of each listed global')
    args = ap.parse_args()
    paths = sorted(glob.glob(os.path.join(ROOT, 'src', '**', '*.cpp'), recursive=True)
                   + glob.glob(os.path.join(ROOT, 'include', '**', '*.h'), recursive=True))
    found, walks = collect(paths)
    wanted = set(args.names)
    listed = [(n, v) for n, v in shadows(found, 1 if wanted else args.min) if not wanted or n in wanted]
    for name, by_view in listed:
        types = {t for _, t, _ in by_view}
        line = f'{name}: {len(types)} types'
        if args.walks and walks[name]:
            line += f', {walks[name]} byte walks'
        print(line)
        for (kind, typ, stars), files in sorted(by_view.items(), key=lambda kv: (kv[0][1], kv[0][0])):
            print(f'  {kind:8} {typ}{" " + "*" * stars if stars else ""}: {", ".join(sorted(files))}')
    several = sum(1 for _, v in shadows(found, 2))
    print(f'shadows: {len(found)} globals seen, {several} viewed through more than one type')
    sys.exit(0)


if __name__ == '__main__':
    main()
