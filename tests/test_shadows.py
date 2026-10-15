import collections

from shadows import collect, shadows, views

SOURCE = """#include "globals.h"
extern s_player_4e9bd4 g_4e9bd4[4];
s_record_pool *g_4e0300;
static long const g_444ae0 = 0;

void f(long i)
{
\tbyte *state = (byte *)&g_4e9bd4[i];
\ts_observer *o = (s_observer *)g_4e9bd4 + i;
\ts_director_camera const *d = (s_director_camera const *)&g_4e9bd4[0].state;
\treturn (long)g_444ae0;
}
"""


def test_views_find_declarations_and_casts():
    found = views('src/a.cpp', SOURCE)
    assert ('g_4e9bd4', 'declared', 's_player_4e9bd4', 0) in found
    assert ('g_4e0300', 'declared', 's_record_pool', 1) in found
    assert ('g_444ae0', 'declared', 'long const', 0) in found
    assert ('g_4e9bd4', 'cast', 'byte', 1) in found
    assert ('g_4e9bd4', 'cast', 's_observer', 1) in found
    assert ('g_4e9bd4', 'cast', 's_director_camera const', 1) in found
    assert ('g_444ae0', 'cast', 'long', 1) not in found  # a value cast, not a pointer cast


def test_collect_counts_byte_walks_apart(tmp_path):
    path = tmp_path / 'a.cpp'
    path.write_text(SOURCE, encoding='utf-8')
    found, walks = collect([str(path)])
    assert walks['g_4e9bd4'] == 1
    assert {t for _, t, _ in found['g_4e9bd4']} == {'s_player_4e9bd4', 's_observer', 's_director_camera const'}
    assert len(found['g_4e0300']) == 1


def test_shadows_lists_globals_with_several_types_most_first():
    found = collections.defaultdict(lambda: collections.defaultdict(set))
    found['g_1'][('declared', 'a', 0)].add('x.cpp')
    found['g_2'][('declared', 'a', 0)].add('x.cpp')
    found['g_2'][('cast', 'b', 1)].add('y.cpp')
    found['g_3'][('declared', 'a', 0)].add('x.cpp')
    found['g_3'][('cast', 'b', 1)].add('y.cpp')
    found['g_3'][('cast', 'c', 1)].add('z.cpp')
    assert [name for name, _ in shadows(found)] == ['g_3', 'g_2']
    assert [name for name, _ in shadows(found, 3)] == ['g_3']
    assert [name for name, _ in shadows(found, 1)] == ['g_3', 'g_2', 'g_1']
