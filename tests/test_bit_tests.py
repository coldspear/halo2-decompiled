from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from bit_tests import main, one_bit_macros, one_bit_names, reads, sites

VA = 0x10000


def code(hex_bytes):
    return list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(bytes.fromhex(hex_bytes), VA))


def test_sites_find_a_shift_and_test_before_a_branch():
    # mov eax, [eax+0x12c]; shr eax, 0xc; test al, 1; je +2; nop; nop
    found = sites(code('8b802c010000' 'c1e80c' 'a801' '7402' '9090'))
    assert found == [(VA + 6, 12, 'mov eax, dword ptr [eax + 0x12c]')]


def test_sites_ignore_masks_values_and_other_registers():
    # test dword ptr [eax+0x12c], 0x1000; je +2: a mask test
    assert sites(code('f7802c01000000100000' '7402' '9090')) == []
    # shr eax, 3; and al, 1; ret: a value, not a branch
    assert sites(code('c1e803' '2401' 'c3' '90')) == []
    # shr eax, 3; test bl, 1; je +2: a different register
    assert sites(code('c1e803' 'f6c301' '7402' '9090')) == []


def test_one_bit_names_leave_out_names_declared_both_ways():
    texts = ['struct a\n{\n\tdword flag12 : 1;\n\tword field_2_3 : 1;\n};\n',
             'struct b\n{\n\tshort field_2_3;\n\tlong weapons[4];\n};\n']
    every, only_bits = one_bit_names(texts)
    assert every == {'flag12', 'field_2_3'}
    assert only_bits == {'flag12'}


def test_reads_count_each_one_bit_form_once():
    names, every = {'flag12'}, {'flag12', 'held'}
    macros = one_bit_macros({'BIT': '((bool)((g_x >> (n)) & 1))', 'SIZE': '(4)'}, every)
    assert macros == {'BIT'}
    body = ('if (TEST_FIELD_BIT(((s_flags *)&d->source)->flag12)) x();\n'
            'if ((bool)((flags >> 3) & 1)) y();\n'
            'if ((bool)((s_items *)object)->held) z();\n'
            'if (d->flag12) w();\n'
            'if (BIT(4)) v();\n')
    assert reads(body, names, macros, every) == 5


def test_reads_ignore_masks_and_bare_shifts():
    body = 'if (flags & 0x1000) a();\nif ((flags >> 2) & 1) b();\nif (x & (1 << 11)) c();\n'
    assert reads(body, {'flag12'}, set(), {'flag12'}) == 0


def test_main_passes_a_matched_reader_of_a_seat_flag(retail_xbe):
    # function_1bfc30 reads the seat flag with TEST_FIELD_BIT and matches
    assert main(['1bfc30', '--xbe', retail_xbe]) == 0
