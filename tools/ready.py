"""Lists the game functions ready to decompile: not decompiled yet (no source in
src/), and every function they call is decompiled already (matched or not: a
real callee, unlike a stub, gets the register convention LTCG gives it in
retail, so the caller can match), belongs to a library, or is part of the same
mutual recursion. Smallest first, one line per function:
va, size, likely source file, calls. The file is a guess, marked "~": the
source file of the nearest decompiled function before it (functions of one
source file sit together). --by-file groups the functions by that file.

    python tools/ready.py [N] [--by-file] [--claims FILE]

--claims FILE is a saved copy of the Active claims table on issue #9
(markdown). A function is left out when its address sits in one of those
ranges. The Finished section is not claimed. A parenthetical "(except ...)"
is a hole, claimed only if another row lists it. Without --claims, the list
is unchanged. The tool does not read GitHub.
"""
import argparse
import bisect
import os
import re
import sys

from inventory import read_rows
from xbe import FUNCTIONS_CSV, Xbe, retail_xbe_path

_RANGE = re.compile(r'`(0x[0-9a-fA-F]+)`\s*[–—-]\s*`(0x[0-9a-fA-F]+)`')
_ADDR = re.compile(r'`(0x[0-9a-fA-F]+)`')
_EXCEPT = re.compile(r'\(([^)]*\bexcept\b[^)]*)\)', re.IGNORECASE)


def components(graph):
    """Strongly connected components (Tarjan), iteratively."""
    index, low, on, stack, out, counter = {}, {}, set(), [], [], [0]
    for root in graph:
        if root in index:
            continue
        work = [(root, iter(graph.get(root, ())))]
        index[root] = low[root] = counter[0]; counter[0] += 1
        stack.append(root); on.add(root)
        while work:
            node, edges = work[-1]
            for nxt in edges:
                if nxt not in graph:
                    continue
                if nxt not in index:
                    index[nxt] = low[nxt] = counter[0]; counter[0] += 1
                    stack.append(nxt); on.add(nxt)
                    work.append((nxt, iter(graph.get(nxt, ()))))
                    break
                if nxt in on:
                    low[node] = min(low[node], index[nxt])
            else:
                work.pop()
                if work:
                    low[work[-1][0]] = min(low[work[-1][0]], low[node])
                if low[node] == index[node]:
                    group = set()
                    while True:
                        n = stack.pop(); on.discard(n); group.add(n)
                        if n == node:
                            break
                    out.append(group)
    return out


def _span_list(text):
    spans = []
    for m in _RANGE.finditer(text):
        a, b = int(m.group(1), 16), int(m.group(2), 16)
        if a > b:
            a, b = b, a
        spans.append((a, b))
    rest = _RANGE.sub(' ', text)
    for m in _ADDR.finditer(rest):
        v = int(m.group(1), 16)
        spans.append((v, v))
    return spans


def _merge(spans):
    spans = sorted(s for s in spans if s[0] <= s[1])
    out = []
    for a, b in spans:
        if out and a <= out[-1][1] + 1:
            out[-1] = (out[-1][0], max(out[-1][1], b))
        else:
            out.append((a, b))
    return out


def _subtract(claimed, holes):
    parts = list(claimed)
    for hlo, hhi in holes:
        nxt = []
        for a, b in parts:
            if hhi < a or hlo > b:
                nxt.append((a, b))
                continue
            if a < hlo:
                nxt.append((a, hlo - 1))
            if b > hhi:
                nxt.append((hhi + 1, b))
        parts = nxt
    return parts


def parse_claims(text):
    """Merged inclusive ranges from an issue #9 Active claims table.

    Stops at the next heading, so the Finished section is ignored. Returns
    [] when the table has no addresses."""
    text = text.replace('\r\n', '\n').replace('\r', '\n')
    low = text.lower()
    start = low.find('## active claims')
    if start != -1:
        nl = text.find('\n', start)
        text = text[nl + 1:] if nl != -1 else ''
        low = text.lower()
    end = low.find('\n## ')
    if end != -1:
        text = text[:end]
    spans = []
    for line in text.splitlines():
        raw = line.strip()
        if not raw.startswith('|'):
            continue
        cells = [c.strip() for c in raw.strip('|').split('|')]
        if len(cells) < 2:
            continue
        head = cells[1].lower().replace(' ', '')
        if head in ('retailrange', '---') or set(head) <= set('-:'):
            continue
        cell = cells[1]
        holes = []
        for m in _EXCEPT.finditer(cell):
            holes += _span_list(m.group(1))
        # A hole applies only to this row. Another row may claim the same
        # addresses (lane I's "except the UI screens", which the UI lane lists).
        spans += _subtract(_span_list(_EXCEPT.sub(' ', cell)), holes)
    return _merge(spans)


def covers(claims, va):
    """True when va lies in one of the merged inclusive ranges."""
    i = bisect.bisect_right(claims, (va, 1 << 64)) - 1
    return i >= 0 and claims[i][0] <= va <= claims[i][1]


def without_claims(rows, claims):
    return [r for r in rows if not covers(claims, int(r['va'], 16))]


def _by_size(r):
    return int(r['size']), r['va']


def ready(rows):
    def calls(va):
        text = rows[va]['calls']
        return {int(c, 16) for c in text.split()} if text else set()

    def done(va):  # decompiled: matched, or has source the checker re-tests
        return rows[va]['status'] == 'matched' or bool(rows[va].get('source'))

    game = {va for va, r in rows.items() if r['owner'] == 'game'}
    graph = {va: calls(va) & game for va in game}
    result = []
    for group in components(graph):
        if all(done(va) for va in group):
            continue
        outside = set().union(*(calls(va) for va in group)) - group
        if all(c not in rows or rows[c]['owner'] != 'game' or done(c) for c in outside):
            result += [rows[va] for va in group if not done(va)]
    return sorted(result, key=_by_size)


def likely_sources(rows, ready_rows, boundaries=()):
    """{va: file} for the ready rows: "~" and the source file of the nearest
    preceding function that has one (not across a section start in
    boundaries), else ''. A ready function has no source of its own."""
    placed = sorted(va for va, r in rows.items() if r.get('source'))
    cuts = sorted(boundaries)
    out = {}
    for r in ready_rows:
        va = int(r['va'], 16)
        i = bisect.bisect_left(placed, va)
        n = bisect.bisect_right(cuts, va)
        floor = cuts[n - 1] if n else 0
        out[va] = '~' + rows[placed[i - 1]]['source'] if i and placed[i - 1] >= floor else ''
    return out


def by_file(ready_rows, files):
    """[(file, rows)], groups ordered by their smallest member, rows by size."""
    groups = {}
    for r in ready_rows:
        groups.setdefault(files[int(r['va'], 16)].lstrip('~'), []).append(r)
    ordered = [(o, sorted(g, key=_by_size)) for o, g in groups.items()]
    return sorted(ordered, key=lambda g: _by_size(g[1][0]))


def line(r, files):
    return ' '.join((r['va'], r['size'], files[int(r['va'], 16)] or '-', r['calls'] or '-'))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('count', nargs='?', type=int, default=20)
    ap.add_argument('--by-file', action='store_true')
    ap.add_argument('--claims', metavar='FILE',
                    help='markdown copy of the issue #9 Active claims table')
    args = ap.parse_args()
    rows = read_rows(FUNCTIONS_CSV)
    retail = retail_xbe_path()
    boundaries = []
    if os.path.exists(retail):
        # section starts only keep a file guess from crossing a section.
        # a missing or corrupt XBE must not turn the ready list into a traceback.
        try:
            boundaries = [s.va for s in Xbe(retail).sections]
        except (OSError, ValueError) as e:
            print(f'warning: {e}; ignoring section boundaries', file=sys.stderr)
    found = ready(rows)
    if args.claims:
        with open(args.claims, encoding='utf-8') as fh:
            claims = parse_claims(fh.read())
        if not claims:
            ap.error('no address claims found (expected the Active claims table from issue #9)')
        kept = without_claims(found, claims)
        print(f'hiding {len(found) - len(kept)} of {len(found)} ready functions '
              f'in {len(claims)} claimed ranges', file=sys.stderr)
        found = kept
    shown = found[:args.count]
    files = likely_sources(rows, shown, boundaries)
    if args.by_file:
        for name, group in by_file(shown, files):
            print(f'{name or "-"} ({len(group)})')
            for r in group:
                print('  ' + line(r, files))
    else:
        for r in shown:
            print(line(r, files))


if __name__ == '__main__':
    main()
