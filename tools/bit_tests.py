"""Lists the one-bit tests retail makes that a function's source does not read
as one-bit values.

VC7.1 LTCG compiles a one-bit value converted to bool and tested in a
condition to `shr reg, N; test reg8, 1` followed by `je` or `jne`. The source
forms that give it are a one-bit field read through TEST_FIELD_BIT (see the
note in include/unknown_11c920.h) and an explicit `(bool)((x >> n) & 1)`. A
plain mask (`x & 0x1000`) or a bare `(x >> n) & 1` in a condition compiles to
a single `test reg, mask` instead, so a source that writes retail's bit tests
that way cannot match.

For each function this counts:
- retail's `shr; test 1; jcc` sequences;
- the source's one-bit reads, which are:
  - TEST_FIELD_BIT calls;
  - `(bool)` casts of a one-bit member or of `(x >> n) & 1`;
  - other reads of members declared as one-bit fields (`<type> name : 1;`
    in src/ or include/), when the name is never declared as anything else;
  - calls of a macro whose replacement reads one.

It lists each function whose retail has more such tests than its source has
reads. For each one it prints retail's sites: the address, the bit, and the
instruction that loaded the value. A function's source is the text from its
@retail marker to the first lone closing brace (as in tools/masked.py).

A listing is a lead, not proof. A helper inlined into the function tests bits
in its own text, which this does not count; that is most of the 0.24% of
matched functions it lists with --all. Only game functions not yet matched
are scanned unless addresses are given or --all is set. The exit status is 1
when something is listed.

    python tools/bit_tests.py [--all] [va ...]
"""
import argparse
import glob
import os
import re
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from inventory import read_rows
from masked import bodies, defines
from xbe import FUNCTIONS_CSV, ROOT, load, retail_xbe_path

# a one-bit field declaration: `dword flag12 : 1;`
_ONE_BIT = re.compile(r'\b(?:unsigned\s+)?(?:dword|word|byte|long|short|char|int|bool|unsigned)\s+(\w+)\s*:\s*1\s*;')
# (bool)((x >> n) & 1), possibly with casts or member accesses inside x
_BOOL_SHIFT = re.compile(r'\(\s*bool\s*\)\s*\(\s*\([^;{}]*?>>[^;{}]*?\)\s*&\s*1\s*\)')
# the low byte of each register a shift can test
_LOW_BYTE = {'eax': 'al', 'ebx': 'bl', 'ecx': 'cl', 'edx': 'dl', 'ax': 'al', 'bx': 'bl', 'cx': 'cl',
             'dx': 'dl', 'al': 'al', 'bl': 'bl', 'cl': 'cl', 'dl': 'dl'}


def sites(code):
    """(address of the shr, bit, the instruction that loaded the register) for
    each `shr reg, N; test reg8, 1; je/jne` in code (capstone instructions)."""
    found = []
    for n, ins in enumerate(code[:-2]):
        if ins.mnemonic != 'shr':
            continue
        operands = [o.strip() for o in ins.op_str.split(',')]
        if len(operands) != 2 or operands[0] not in _LOW_BYTE:
            continue
        try:
            bit = int(operands[1], 0)
        except ValueError:
            continue
        test, jump = code[n + 1], code[n + 2]
        if test.mnemonic != 'test' or test.op_str.replace(' ', '') != _LOW_BYTE[operands[0]] + ',1':
            continue
        if jump.mnemonic not in ('je', 'jne'):
            continue
        loaded = next((f'{p.mnemonic} {p.op_str}' for p in reversed(code[max(0, n - 4):n])
                       if p.op_str.split(',')[0].strip() == operands[0]), '')
        found.append((ins.address, bit, loaded))
    return found


def one_bit_names(texts):
    """(every name declared as a one-bit field in texts, the subset never
    declared as anything else). A name some other structure declares as an
    ordinary member (several use field_<offset>_<bit> or unknown<offset> both
    ways) cannot tell a read of the bit from a read of the member on its own."""
    texts = list(texts)
    one_bit = {name for text in texts for name in _ONE_BIT.findall(text)}
    ordinary = {m.group(1) for text in texts for m in _ORDINARY.finditer(text)}
    return one_bit, one_bit - ordinary


# an ordinary declaration: `short field_2_3;`, `long weapons[4];`, `byte *data;`
_ORDINARY = re.compile(r'^[ \t]*(?:const\s+)?(?:struct\s+|unsigned\s+)?[A-Za-z_]\w*[ \t*]+(\w+)[ \t]*(?:\[[^\]\n]*\])?[ \t]*;', re.M)
_BOOL_CAST = re.compile(r'\(\s*bool\s*\)\s*')


def one_bit_macros(macros, names):
    """The macros (other than TEST_FIELD_BIT) whose replacement reads a one-bit value."""
    member = _member_pattern(names)
    return {name for name, body in macros.items() if name != 'TEST_FIELD_BIT' and (
        'TEST_FIELD_BIT' in body or _BOOL_SHIFT.search(body) or (member and member.search(body)))}


def _member_pattern(names):
    if not names:
        return None
    return re.compile(r'(?:\.|->)(?:' + '|'.join(sorted(map(re.escape, names), key=len, reverse=True)) + r')\b')


def _operand_end(text, start):
    """The end of the operand that starts at text[start]: a balanced
    parenthesised group with any member accesses after it, or a run of
    identifiers, member accesses and subscripts."""
    i = start
    while i < len(text):
        c = text[i]
        if c == '(' or c == '[':
            depth, close = 1, ')' if c == '(' else ']'
            i += 1
            while i < len(text) and depth:
                depth += 1 if text[i] == c else -1 if text[i] == close else 0
                i += 1
        elif c.isalnum() or c == '_' or c == '.':
            i += 1
        elif text.startswith('->', i):
            i += 2
        else:
            break
    return i


def _take(text, opener, keep):
    """text without each match of opener and the operand after it, and how
    many of those operands keep() accepts."""
    out, i, count = [], 0, 0
    for m in opener.finditer(text):
        if m.start() < i:
            continue
        end = _operand_end(text, m.end())
        if keep(text[m.end():end]):
            out.append(text[i:m.start()])
            out.append(' ')
            count += 1
            i = end
    out.append(text[i:])
    return ''.join(out), count


def reads(body, names, macros, any_one_bit=None):
    """How many one-bit values body reads: TEST_FIELD_BIT calls, (bool) casts
    of a one-bit member or of (x >> n) & 1, other reads of one-bit members
    never declared as anything else, and calls of the macros that read one."""
    any_member = _member_pattern(any_one_bit or names)
    rest, count = _take(body, re.compile(r'\bTEST_FIELD_BIT\s*(?=\()'), lambda operand: True)
    rest, casts = _take(rest, _BOOL_CAST, lambda operand: bool(
        re.search(r'>>[^;]*&\s*1\b', operand) or (any_member and any_member.search(operand))))
    count += casts
    member = _member_pattern(names)
    count += len(member.findall(rest)) if member else 0
    for name in macros:
        count += len(re.findall(r'(?<![\w.>])' + re.escape(name) + r'\s*\(', rest))
    return count


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('va', nargs='*', help='only these functions (hex)')
    ap.add_argument('--all', action='store_true', help='also scan matched functions')
    ap.add_argument('--xbe', default=retail_xbe_path(), help='the retail XBE')
    args = ap.parse_args(argv)
    wanted = {int(v, 16) for v in args.va}
    rows = read_rows(FUNCTIONS_CSV)
    paths = sorted(glob.glob(os.path.join(ROOT, 'src', '**', '*.cpp'), recursive=True))
    headers = sorted(glob.glob(os.path.join(ROOT, 'include', '*.h')))
    texts = {p: open(p, encoding='utf-8', errors='replace').read() for p in paths + headers}
    every_name, names = one_bit_names(texts.values())
    macros = one_bit_macros(defines(texts.values()), every_name)
    image = load(args.xbe)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.skipdata = True  # jump tables inside a function
    listed = []
    for p in paths:
        rel = os.path.relpath(p, ROOT).replace(os.sep, '/')
        for va, (path, line, body) in bodies(rel, texts[p]).items():
            row = rows.get(va)
            if row is None or row['owner'] != 'game':
                continue
            if wanted and va not in wanted:
                continue
            if not wanted and not args.all and row['status'] == 'matched':
                continue
            found = sites(list(md.disasm(image.read(va, int(row['size'])), va)))
            count = reads(body, names, macros, every_name)
            if len(found) > count:
                listed.append((len(found) - count, va, path, line, row['status'], len(found), count, found))
    listed.sort(key=lambda item: (-item[0], item[1]))
    for missing, va, path, line, status, n, count, found in listed:
        print(f'{va:#08x}  {path}:{line}  {status}: retail tests {n} one-bit values, the source reads {count}')
        for at, bit, loaded in found:
            print(f'    {at:#08x}  bit {bit:<2}  {loaded}')
    if not wanted:
        print(f'{len(listed)} functions listed')
    return 1 if listed else 0


if __name__ == '__main__':
    sys.exit(main())
