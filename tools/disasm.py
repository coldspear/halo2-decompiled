"""Prints a retail function's disassembly, with its inventory row and the
names of what it calls. Jump tables inside the function print as data
(``dd`` for a label table, ``db`` for a byte index table), not as instructions.

    python tools/disasm.py <va>
"""
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone import x86

from functions import MAX_BODY, _switch_tables
from inventory import check_retail, read_rows
from xbe import FUNCTIONS_CSV, load, retail_xbe_path


class _Slice:
    """The function's own bytes, addressed at its retail VA, for _switch_tables."""

    def __init__(self, data, va):
        self.bytes = data
        self.lo = va
        self.hi = va + len(data)
        self.md = Cs(CS_ARCH_X86, CS_MODE_32)
        self.md.detail = True
        self.md.skipdata = True

    def inside(self, va):
        return self.lo <= va < self.hi

    def dword(self, va):
        o = va - self.lo
        return struct.unpack_from('<I', self.bytes, o)[0] if 0 <= o <= len(self.bytes) - 4 else None

    def byte(self, va):
        o = va - self.lo
        return self.bytes[o] if 0 <= o < len(self.bytes) else None


def _remember(tables, found, after):
    """Tables that start after the switch jump. A later jump may name one again."""
    known = {start for start, _, _ in tables}
    for start, width, count in found:
        if width > 0 and count > 0 and start >= after and start not in known:
            tables.append((start, width, count))


def _cover(tables, addr):
    """The table containing addr, else None."""
    for start, width, count in tables:
        if start <= addr < start + width * count:
            return start, width
    return None


def _note(rows, va):
    callee = rows.get(va)
    if not callee:
        return ''
    return f"   ; {callee['va']} [{callee['status']}]"


def listing(data, va, rows):
    """Lines of the disassembly of ``data``, which begins at ``va``.

    ``rows`` is the inventory (``{address: row}``), used to name direct calls
    and jumps. A switch's label table and byte index table, the ones
    ``functions._switch_tables`` would attach to this function, print as
    ``dd`` and ``db``. MSVC lays those tables after the jump that reads them;
    a table that sits before its jump is still disassembled as code.
    """
    code = _Slice(data, va)
    md = code.md
    tables, lines, block = [], [], []
    off = 0
    while off < len(data):
        addr = va + off
        hit = _cover(tables, addr)
        if hit:
            start, width = hit
            rel = addr - start
            if width == 4 and rel % 4 == 0 and off + 4 <= len(data):
                value, = struct.unpack_from('<I', data, off)
                note = '   ; jump table' if addr == start else ''
                note += _note(rows, value)
                lines.append(f'  {addr:08x}  dd 0x{value:x}{note}')
                off += 4
            else:
                note = '   ; jump table' if addr == start else ''
                lines.append(f'  {addr:08x}  db 0x{data[off]:02x}{note}')
                off += 1
            block = []
            continue
        ins = next(md.disasm(data[off:off + 16], addr), None)
        if ins is None or ins.size <= 0:
            lines.append(f'  {addr:08x}  .byte 0x{data[off]:02x}')
            off += 1
            block = []
            continue
        block.append(ins)
        note = ''
        op = ins.operands[0] if ins.operands else None
        if ins.mnemonic in ('call', 'jmp') and op is not None and op.type == x86.X86_OP_IMM:
            note = _note(rows, op.imm)
        if (ins.mnemonic == 'jmp' and op is not None and op.type == x86.X86_OP_MEM
                and op.mem.base == 0 and op.mem.index != 0 and op.mem.scale == 4
                and code.inside(op.mem.disp)):
            # the same rule as functions._descend: the straight-line block
            # still holds the cmp/ja bound check and a movzx of the index table
            valid = lambda t, start=va: code.inside(t) and 0 <= t - start <= MAX_BODY
            _remember(tables, _switch_tables(code, block, ins, valid), addr + ins.size)
            block = []
        elif ins.mnemonic == 'jmp' or ins.mnemonic in ('ret', 'int3', 'hlt'):
            block = []
        lines.append(f'  {addr:08x}  {ins.mnemonic} {ins.op_str}{note}')
        off += ins.size
    return lines


def main():
    if len(sys.argv) != 2:
        sys.exit('usage: python tools/disasm.py <va>')
    try:
        va = int(sys.argv[1], 16)
    except ValueError:
        sys.exit(f'{sys.argv[1]!r} is not a hexadecimal retail address')
    rows = read_rows(FUNCTIONS_CSV)
    row = rows.get(va)
    if row is None:
        raise SystemExit(f'{va:#x} is not a function start in config/functions.csv')
    path = retail_xbe_path()
    check_retail(path)
    image = load(path)
    print(f"{row['va']} size {row['size']} {row['owner']} {row['style']} ({row['evidence']}) {row.get('source') or '-'}")
    for line in listing(image.read(va, int(row['size'])), va, rows):
        print(line)


if __name__ == '__main__':
    main()
