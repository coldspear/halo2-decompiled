"""disasm listing. No retail XBE and no SDK, except the one retail check."""
import sys

import pytest

from disasm import listing, main


def _row(va, status='todo', source=''):
    return dict(va=f'{va:08x}', status=status, source=source)


def test_bound_check_keeps_a_following_label_out_of_the_table():
    """cmp/ja is the bound. A third dword that looks like a label is code.

    Dropping the straight-line block at the ja would miss the bound and
    swallow that dword into the table.
    """
    code = bytes.fromhex(
        '83f801'             # 1000 cmp eax, 1        bound = 2
        '7710'               # 1003 ja 0x1015
        'ff248520100000'     # 1005 jmp [eax*4 + 0x1020]
        'b801000000c3'       # 100c case 0
        '33c0c3'             # 1012 case 1
        'c3'                 # 1015 default
        + 'cc' * 10 +        # 1016 pad to 0x1020
        '0c100000'           # 1020 label 0
        '12100000'           # 1024 label 1
        '0c100000'           # 1028 a real label, past the bound
    )
    lines = listing(code, 0x1000, {})
    text = '\n'.join(lines)
    assert '  00001020  dd 0x100c   ; jump table' in text
    assert '  00001024  dd 0x1012' in text
    assert '  00001028  dd ' not in text
    assert any(line.startswith('  0000100c  mov') for line in lines)
    assert not any('adc' in line or 'push es' in line for line in lines)


def test_two_level_switch_prints_both_tables_as_data():
    code = bytes.fromhex(
        '8b442404'                 # 1000 mov eax, [esp + 4]
        '0fb6882a100000'           # 1004 movzx ecx, byte ptr [eax + 0x102a]
        'ff248d20100000'           # 100b jmp [ecx*4 + 0x1020]
        'b801000000c3'             # 1012 label 0
        '33c0c3'                   # 1018 label 1
        '8bff8d4900'               # 101b alignment
        '12100000' '18100000' '00000000'  # 1020 label table, label 2 null
        '000101000201'             # 102c index table
        + 'cc' * 14 +
        'c3')
    lines = listing(code, 0x1000, {})
    text = '\n'.join(lines)
    assert '  00001020  dd 0x1012   ; jump table' in text
    assert '  00001024  dd 0x1018' in text
    assert '  00001028  dd 0x0' in text
    assert '  0000102c  db 0x00   ; jump table' in text
    assert '  0000102d  db 0x01' in text
    assert '  00001031  db 0x01' in text
    # the index bytes are not decoded as adds
    assert not any(line.split()[1] == 'add' for line in lines if '  0000102' in line)


def test_direct_call_is_noted_with_its_status():
    # e8 0b000000 is call +0xb, landing at 0x1010; then ret; pad; xor eax, eax; ret
    code = bytes.fromhex('e80b000000c3') + b'\xcc' * 10 + bytes.fromhex('33c0c3')
    rows = {0x1010: _row(0x1010, status='matched')}
    text = '\n'.join(listing(code, 0x1000, rows))
    assert '  00001000  call 0x1010   ; 00001010 [matched]' in text
    assert '  00001010  xor eax, eax' in text


def test_function_without_a_switch_has_no_data_lines():
    code = bytes.fromhex('33c0c3')
    assert listing(code, 0x1000, {}) == ['  00001000  xor eax, eax', '  00001002  ret ']


def test_main_prints_the_inventory_row_and_the_listing(monkeypatch, capsys):
    row = dict(va='00001000', size='3', owner='game', style='size', evidence='packed',
               status='todo', source='src/a.cpp')
    monkeypatch.setattr('disasm.read_rows', lambda path: {0x1000: row})
    monkeypatch.setattr('disasm.check_retail', lambda path: None)

    class Image:
        def read(self, va, size):
            assert (va, size) == (0x1000, 3)
            return bytes.fromhex('33c0c3')

    monkeypatch.setattr('disasm.load', lambda path: Image())
    monkeypatch.setattr(sys, 'argv', ['disasm.py', '1000'])
    main()
    out = capsys.readouterr().out
    assert out.splitlines()[0] == '00001000 size 3 game size (packed) src/a.cpp'
    assert '  00001000  xor eax, eax' in out
    assert '  00001002  ret ' in out


@pytest.mark.retail
def test_retail_switch_at_0x6e1e0_prints_its_tables(retail_xbe):
    from inventory import read_rows
    from xbe import FUNCTIONS_CSV, Xbe

    rows = read_rows(FUNCTIONS_CSV)
    row = rows[0x6e1e0]
    data = Xbe(retail_xbe).read(0x6e1e0, int(row['size']))
    lines = listing(data, 0x6e1e0, rows)
    text = '\n'.join(lines)
    assert '  0006e2e0  dd 0x6e212   ; jump table' in text
    assert '  0006e2e4  dd 0x6e285' in text
    assert '  0006e2f8  dd 0x0' in text
    assert '  0006e2fc  db 0x00   ; jump table' in text
    assert '  0006e30e  db 0x05' in text
    # the label table used to disassemble as "adc ah, dl" / "push es"
    assert not any(line.split()[1:3] == ['adc', 'ah'] for line in lines)
    assert not any(line.split()[1:3] == ['push', 'es'] for line in lines)
