"""Lists the absolute address fields in matched functions that reach a
different global in our build than in retail.

tools/check.py masks every absolute field (a base relocation) and asks only
that retail's value be an address inside the image, so a function that loads
g_a where retail loads g_b, or reads field +0x8 of a record where retail reads
+0xc, still matches. This reads the last full build (build/halo2.map and
build/halo2.exe) and the retail XBE, builds nothing, and compares what each
field reaches.

Our field's value is resolved to a map symbol plus an offset. When that
symbol's retail address is known, retail's value is expected to be that
address plus the same offset. A symbol's retail address is known when:

- it is a function marked "// @retail 0x..." or "// @stub 0x..." (the marker
  gives it);
- it is a global named g_<retail address> or g_<retail address>_<suffix>, as
  src/ names the data it recreates, and the address lies in a retail section.

A value that lands exactly at a symbol's start may also be one past the end
of the symbol before it (an end pointer), so the field is accepted when either
reading agrees with retail. Fields whose symbol has no retail address (string
literals, vtables, float constants, static locals, library data) are counted
as untied and not compared. Jump table entries, which point inside their own
function, are check.py's business and are skipped.

Exit status 1 when a mismatch is listed. Nothing here changes a function's
status: the list is for reading, until it is empty and the check can move into
tools/check.py.

    python tools/globals.py [<retail va> ...] [--untied]
"""
import argparse
import bisect
import os
import re
import sys

import build
from check import Identity, StandinCalls, extract, points_inside, resolve
from inventory import read_rows
from linkmap import plain_name
from reduced import Image, fields_within
from xbe import FUNCTIONS_CSV, ROOT, Xbe, retail_xbe_path

GLOBAL = re.compile(r'^g_([0-9a-f]{5,6})(?:_|$)')


def global_address(plain, in_retail):
    """The retail address a g_<address> or g_<address>_<suffix> global names,
    when in_retail(address) says it lies in the retail image, else None."""
    m = GLOBAL.match(plain)
    if not m:
        return None
    va = int(m.group(1), 16)
    return va if in_retail(va) else None


def _dword(data, k):
    return int.from_bytes(data[k:k + 4], 'little')


class Resolver:
    """The retail address of each of our symbols that has one: functions by
    their markers (address_of, from check.Identity), data by its name."""

    def __init__(self, linkmap, address_of, in_retail):
        self.linkmap = linkmap
        self.base_of = {}  # our symbol va -> retail address
        for s in linkmap.symbols:
            retail = address_of.get(s.name)
            if retail is None:
                retail = global_address(plain_name(s.name), in_retail)
            if retail is not None:
                self.base_of.setdefault(s.va, (retail, s))
        self._vas = sorted(self.base_of)

    def expectations(self, value):
        """[(symbol, offset, expected retail value)] for our field value: the
        symbol whose extent holds it, and, when it sits at a symbol's start,
        the symbol before it read as an end pointer."""
        out = []
        symbol = self.linkmap.symbol_at(value)
        candidates = []
        if symbol is not None:
            candidates.append(symbol.va)
        if symbol is None or symbol.va == value:
            # one past the end of the symbol before: the previous tied symbol
            i = bisect.bisect_left(self._vas, value)
            if i:
                candidates.append(self._vas[i - 1])
        for va in candidates:
            tied = self.base_of.get(va)
            if tied is not None:
                retail, s = tied
                out.append((s, value - va, (retail + value - va) & 0xFFFFFFFF))
        return out

    def describe_retail(self, value):
        """What retail's value points at, by the tied symbols: 'g_xxxx+0x8', or ''."""
        best = None
        for va, (retail, s) in self.base_of.items():
            if retail <= value and (best is None or retail > best[0]):
                best = (retail, s)
        if best is None:
            return ''
        retail, s = best
        return f'{plain_name(s.name)}+{value - retail:#x}'


def check_fields(ours, start, theirs, theirs_va, absolute, resolver):
    """Compares the absolute fields of one function. ours and theirs are its
    code in our image (from start) and in retail (from theirs_va); absolute
    holds the field addresses in our image. Returns (mismatches, tied, untied):
    mismatches are (offset, our symbol, our offset, expected, retail value)."""
    mismatches, tied, untied = [], 0, 0
    size = min(len(ours), len(theirs))
    for field in absolute:
        k = field - start
        if k < 0 or k + 4 > size:
            continue
        if points_inside(ours, start, theirs, theirs_va, k):
            continue  # a jump table entry or its table: check.py compares these
        value, retail_value = _dword(ours, k), _dword(theirs, k)
        expected = resolver.expectations(value)
        if not expected:
            untied += 1
            continue
        tied += 1
        if not any(e == retail_value for _, _, e in expected):
            s, offset, e = expected[0]
            mismatches.append((k, s, offset, e, retail_value))
    return mismatches, tied, untied


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0], fromfile_prefix_chars='@')
    ap.add_argument('addresses', nargs='*', help='retail addresses of the functions to check (default: every matched one)')
    ap.add_argument('--untied', action='store_true', help='also list the fields no retail address could be tied to')
    args = ap.parse_args()
    wanted = {int(a, 16) for a in args.addresses}

    retail_path = retail_xbe_path()
    if not os.path.exists(retail_path):
        raise SystemExit(f'retail XBE not found at {retail_path}: put it at orig/default.xbe or set RETAIL_XBE')
    map_path = os.path.join(ROOT, 'build', build.MAP_NAME)
    exe_path = os.path.join(ROOT, 'build', build.EXE_NAME)
    if not (os.path.exists(map_path) and os.path.exists(exe_path)):
        raise SystemExit('no full build (build/halo2.exe and build/halo2.map); run python tools/check.py first')

    retail = Xbe(retail_path)
    ours = Image.read(map_path, exe_path)
    rows = read_rows(FUNCTIONS_CSV)
    marked, stubs = build.marked_sources(), build.stub_sources()
    standin_calls = StandinCalls(ours.linkmap, ours.image, marked)
    identity = Identity(ours.linkmap, rows, marked + stubs, standin_calls)

    def in_retail(va):
        return any(s.va <= va < s.va + s.vsize for s in retail.sections)

    resolver = Resolver(ours.linkmap, identity.address_of, in_retail)

    functions = total_tied = total_untied = total_mismatched = 0
    for m in marked:
        row = rows.get(m.retail)
        if row is None or row['status'] != 'matched' or (wanted and m.retail not in wanted):
            continue
        symbol = resolve(ours.linkmap, m, standin_calls)
        if symbol is None:
            continue
        start, end = ours.linkmap.extent(symbol)
        theirs = retail.read(m.retail, int(row['size']))
        code = extract(ours.image.read(start, end - start), theirs)
        mismatches, tied, untied = check_fields(code, start, theirs, m.retail,
                                                fields_within(ours.absolute, start, end), resolver)
        functions += 1
        total_tied += tied
        total_untied += untied
        if mismatches:
            total_mismatched += 1
        for k, s, offset, expected, retail_value in mismatches:
            where = resolver.describe_retail(retail_value)
            print(f'MISMATCH {m.retail:08x} {m.name} at +{k:#x}: ours {plain_name(s.name)}+{offset:#x} '
                  f'(retail {expected:#x}), retail holds {retail_value:#x}' + (f' ({where})' if where else ''))
        if args.untied and untied:
            print(f'UNTIED   {m.retail:08x} {m.name}: {untied} field{"" if untied == 1 else "s"} with no retail address')
    print(f'globals: {functions} matched functions, {total_tied} fields tied to a retail address, '
          f'{total_untied} untied, {total_mismatched} function{"" if total_mismatched == 1 else "s"} with a mismatch')
    sys.exit(1 if total_mismatched else 0)


if __name__ == '__main__':
    main()
