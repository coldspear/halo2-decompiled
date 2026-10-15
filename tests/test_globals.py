from globals import Resolver, check_fields, global_address
from linkmap import LinkMap

# a function at 0x401000 and three data symbols: two recreated globals named
# by their retail addresses (0x4e9bd4 and 0x4e9c00) and one string literal
MAP = """ Preferred load address is 00400000
 0001:00000000 00000100H .text                   CODE
 0003:00000000 00000100H .data                   DATA
 0001:00000000       ?f@@YIXXZ                  00401000 f   a.obj
 0003:00000000       ?g_4e9bd4@@3PAUs@@A        00403000     a.obj
 0003:00000040       ?g_4e9c00_table@@3PAJA     00403040     a.obj
 0003:00000080       ??_C@_05ABC@hello@         00403080     a.obj
"""

RETAIL_LO, RETAIL_HI = 0x10000, 0x600000


def in_retail(va):
    return RETAIL_LO <= va < RETAIL_HI


def resolver():
    return Resolver(LinkMap(MAP), {'?f@@YIXXZ': 0x1000}, in_retail)


def test_global_address_reads_the_retail_address_from_the_name():
    assert global_address('g_4e9bd4', in_retail) == 0x4e9bd4
    assert global_address('g_4e9c00_table', in_retail) == 0x4e9c00
    assert global_address('g_camera_storage', in_retail) is None
    assert global_address('g_4e9bd4x', in_retail) is None


def test_global_address_needs_a_retail_section():
    assert global_address('g_ffffff', in_retail) is None


def test_expectations_add_the_symbols_offset():
    found = resolver().expectations(0x403008)
    assert [(s.name, offset, expected) for s, offset, expected in found] == [('?g_4e9bd4@@3PAUs@@A', 8, 0x4e9bdc)]


def test_expectations_at_a_start_also_read_an_end_pointer():
    # 0x403040 starts g_4e9c00_table and is one past the end of g_4e9bd4 (0x40 bytes)
    found = resolver().expectations(0x403040)
    assert [(s.name, offset, expected) for s, offset, expected in found] == [
        ('?g_4e9c00_table@@3PAJA', 0, 0x4e9c00), ('?g_4e9bd4@@3PAUs@@A', 0x40, 0x4e9c14)]


def test_expectations_know_marked_functions():
    found = resolver().expectations(0x401010)
    assert [(s.name, offset, expected) for s, offset, expected in found] == [('?f@@YIXXZ', 0x10, 0x1010)]


def test_expectations_untied_symbol_is_empty():
    assert resolver().expectations(0x403084) == []


def code(value):
    return bytes.fromhex('a1') + value.to_bytes(4, 'little') + bytes.fromhex('c3')  # mov eax, [value]; ret


def test_matching_global_passes():
    ours, theirs = code(0x403008), code(0x4e9bdc)
    assert check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver()) == ([], 1, 0)


def test_different_global_is_a_mismatch():
    ours, theirs = code(0x403008), code(0x4e9c08)
    mismatches, tied, untied = check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver())
    assert (tied, untied) == (1, 0)
    (k, s, offset, expected, retail_value), = mismatches
    assert (k, s.name, offset, expected, retail_value) == (1, '?g_4e9bd4@@3PAUs@@A', 8, 0x4e9bdc, 0x4e9c08)


def test_different_field_offset_is_a_mismatch():
    ours, theirs = code(0x403008), code(0x4e9bd8)  # +0x8 against retail's +0x4
    mismatches, _, _ = check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver())
    assert len(mismatches) == 1 and mismatches[0][3:] == (0x4e9bdc, 0x4e9bd8)


def test_end_pointer_passes_either_way():
    ours = code(0x403040)
    assert check_fields(ours, 0x401000, code(0x4e9c00), 0x1000, [0x401001], resolver())[0] == []
    assert check_fields(ours, 0x401000, code(0x4e9c14), 0x1000, [0x401001], resolver())[0] == []


def test_untied_symbol_is_counted_not_compared():
    ours, theirs = code(0x403084), code(0x2a0000)
    assert check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver()) == ([], 0, 1)


def test_jump_table_entries_are_skipped():
    # a field that points inside the function on our side is a jump table entry
    ours, theirs = code(0x401003), code(0x1003)
    assert check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver()) == ([], 0, 0)


def test_fields_outside_the_compared_length_are_ignored():
    ours, theirs = code(0x403008), bytes.fromhex('c3')
    assert check_fields(ours, 0x401000, theirs, 0x1000, [0x401001], resolver()) == ([], 0, 0)


def test_describe_retail_names_the_nearest_tied_symbol():
    assert resolver().describe_retail(0x4e9c08) == 'g_4e9c00_table+0x8'
    assert resolver().describe_retail(0x1004) == 'f+0x4'
    assert resolver().describe_retail(0x10) == ''
