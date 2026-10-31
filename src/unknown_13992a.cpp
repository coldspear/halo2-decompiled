// @flags /O1 /Oi /arch:SSE /Gr
#include "unknown_11c920.h"
#include "unknown_0259d0.h"
#include "globals.h"
#include "object_dispatch.h"
#include <string.h>

struct s_unit_weapon_hands
{
    byte unknown00[8];
    bool one;
    bool two;
    bool seat_allows;
    byte unknown0b;
};
struct s_hud_weapon_record;
struct s_13b164_status;
struct s_22bfc0_definition;
struct s_22bff0_definition;
struct s_player_score_display
{
    long type;
    bool teams;
    byte unknown05[3];
    long limit;
    word name[32];
    bool timer;
    byte unknown4d;
    word timer_text[8];
    byte unknown5e[2];
    color3f color;
    bool leading;
    byte unknown6d[3];
    long player_index;
    long score;
    word score_text[8];
    bool rival;
    byte unknown89[3];
    long rival_index;
    color3f rival_color;
    long rival_score;
    word rival_text[8];
    bool charge;
    byte unknownb1[3];
    real charge_fraction;
    long zone_count;
    dword zone_colors[8];
    real zone_progress[8];
};


void function_cb430(long arg_1, s_unit_weapon_hands *arg_2);
short function_ce020(long arg_1);
short function_cdff0(long arg_1, short arg_2);
short function_c8860(long arg_1);
long function_cbd50(long arg_1, short arg_2);
bool function_100f70(long arg_1);
bool function_15fef0(long arg_1);
bool function_15eb20(long arg_1);
real function_c53c0(long arg_1);
long function_22bfc0(s_22bfc0_definition const *arg_1, char arg_2);
long function_22bff0(s_22bff0_definition const *arg_1, short arg_2, char arg_3);
void function_13afa9(long arg_1, long *arg_3, s_hud_weapon_record *arg_2);
short function_cfe40(long arg_1);
long function_cbe60(long arg_1);
void function_13b164(long arg_1, s_13b164_status *arg_2);
bool function_15dea0(void);
bool function_15eaf0(void);
bool function_15f3a0(long arg_1, s_player_score_display *arg_2);
void unicode_string_copy(word *arg_1, word const *arg_2, long arg_3);
long function_162fd0(long arg_1);
bool function_138820(void);
long function_14de70(long arg_1);

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// score 48 (was 48): frame 0x120, scalar homes and both definition-call setups agree. Left: the
// parent index (+0x14) is held in a register from its NONE test into the parent lookup, where retail
// compares it in memory and reads it again after the seat test (one byte longer), so every later call
// and global field sits a byte off. Round 27: 0x22bfc0's and 0x22bff0's parameters put in the order
// retail's caller sets them up, definition first (64 -> 48, both callees still match); before that the
// tests nested 64 and the parent index read as dword 64. On the kept form (fast screens, which show the
// same parent register): the test read as dword or as `((long const *)local_3)[5]`, the parent lookup
// declared before the test, the 0x22bfc0 result through a local: the parent stays in a register each
// time; the seat flag stored before the parent lookup reloads both fields (fast 521); a bool helper for
// the two tests (fast 418).
// Earlier (113): the first branch span and mask/player register roles. Word lookup index 580; byte slot counter 314;
// short suppression 173; cached seat selector 152; missing-player return 134 (plain).
// caller depth 1/2/0 134/134/651; global optimization off 1911; const lookup 134;
// typed lookup through getter / directly / depth one 134/113/134 (plain).
// Mask/player ranks and parent/tag scheduling remain; initial parent value 113;
// seat value 421; parent plus seat 131; direct queried flag 426; display presence 151;
// target query value and primary/both color copies 113 each (plain).
// Earlier: mask/player register roles and parent-load scheduling remain;
// parent/seat/both references 131f; native current/parent-child tags 131f;
// repeated secondary pass 446f; bulk sentinel initialization 527f; frame omission 131f.
// sentinel loop 527f; typed selection arrays 298f; handle/rank/both snapshots 138/131/131f;
// child test order 141f; wider selection scope 131f; cached zone count 145f (control 131f).
// Native output sentinel/flag/scalar/pairs/full/scoped views 131f (control 131f);
// the original prefix stores and all protected query interfaces are retained.
// Earlier: mask/player roles and tag scheduling remain; current/parent/both tag values 113 each;
// parent mode char 113; initialized display 274; native current/parent-child/all selectors 113/113/113 (plain).
// Mask/player roles and parent/tag scheduling remain. First/second/both adjacent
// field copies 113/113/113p; paired hands bytes and prefix copies 465p; checked player accessor / first
// copy / all copies 519/519/518p.
// Seat-word observation, native parent/child links, ordinary child for, combined links,
// child for plus seat word and display/both workspace names each 131f (control 131f).
// Earlier: BLOCKED-BY 0xcb430: retail passes the hands record to it in edi, ours in
// esi, which swaps esi and edi (mask and player pointer) through the whole body.
// Zone destination progress/color/both cursors 161/161/161; independent per-publication
// divisors 216, plus progress/color/both cursors 208/208/208; both input/output cursors 160.
// Mask/player register roles remain tied to the outside-queue hands output convention;
// original score expressions and copy loops retained.
// Round 204: Fresh records at every use: player 650; current object 726; both 977; parent 470;
// player/parent 697; current/parent 785; all three 1094; all three plus child 1094. Every expanded lookup
// graph regresses and is restored. Mask/player register roles and tag-call scheduling remain; protected
// interfaces and all original values/calls preserved.
// No-score-mode arm first, with display-failure/fraction-arm/workspace-lifetime combinations
// 267/268/267/268/267/268/267/268f (unchanged control 131f). Original branches and workspace restored.
// Round 225: own/rival/both native-size color copies, immutable player handle alone/
// with each/both copies, and all plus immutable object receiver: 131f each,
// unchanged fast control 131 versus full 113. Original copies and body restored.
// Round 33 (plain checks; fast is unfaithful here, 70 on the unchanged source): retail reloads
// local_3 into EAX after the 0x22bfc0 store, compares the parent in memory and reads it again
// into EAX (0xd1080 shows the same split); ours reloads local_3 into ECX before the store and
// keeps the parent in EAX. Nested tests 48; the parent lookup through the inline object_get 48;
// the two tests in a named bool 48; `(void)&local_3` 48 (diagnostic); the seat flag stored first
// in the block 520; the parent index in a named local for the test and the lookup 48.
// Round 29 (fast, control 70): an inline seat getter returning the seat element or NULL 438; the
// definition folded into the seat expression 70; the seat index in a short local 97 (the unit reload
// then goes to eax after the result store as in retail, the parent to ecx); NONE-first tests 70; the
// parent's header element through a pointer 70. Permuter: 48 tries, nothing better.
// Round 20 (lane 15, fast; the reduced image gives 48 here): the seat word read into a short local before the
// parent test 46 (the result store then comes first and the unit reloads into EAX as in retail, but the seat
// is read before the parent and the parent is held in EAX), that local also passed to 0x22bff0 392 (1827 bytes);
// the seat assigned inside the condition 48; the parent test nested with the seat local inside it 48; the 0x48
// flag store first in the block (diagnostic) 520.
// @retail 0x13992a
void function_13992a(byte *arg_1, long arg_2)
{
    memset(arg_1, 0, 0x270);
    *(long *)(arg_1 + 0) = NONE;
    *(long *)(arg_1 + 4) = NONE;
    *(long *)(arg_1 + 8) = NONE;
    *(long *)(arg_1 + 0xc) = NONE;
    *(long *)(arg_1 + 0x10) = NONE;
    *(long *)(arg_1 + 0x14) = NONE;
    *(long *)(arg_1 + 0x18) = NONE;
    long local_1 = function_14de70(arg_2);
    if (local_1 != NONE)
    {
        byte const *local_2 = g_4e8c24->data + (local_1 & 0xffff) * 0x21c;
        if (*(long *)(local_2 + 0x2c) != NONE)
        {
            byte const *local_3 = (byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c));
            s_unit_weapon_hands local_4;
            bool local_5 = false;
            function_cb430(*(long *)(local_2 + 0x2c), &local_4);
            arg_1[0x1c] = true;
            *(long *)(arg_1 + 0x20) = *(long *)(local_3 + 0x1e4);
            *(long *)(arg_1 + 0x24) = *(long *)(local_3 + 0x1e8);
            *(long *)(arg_1 + 0x28) = *(long *)(local_3 + 0xec);
            *(long *)(arg_1 + 0x2c) = *(long *)(local_3 + 0xf0);
            *(short *)(arg_1 + 0x36) = function_ce020(*(long *)(local_2 + 0x2c));
            *(short *)(arg_1 + 0x34) = function_cdff0(*(long *)(local_2 + 0x2c), function_ce020(*(long *)(local_2 + 0x2c)));
            arg_1[0x30] = local_4.one;
            arg_1[0x31] = local_4.two;
            *(short *)(arg_1 + 0x3c) = function_c8860(*(long *)(local_2 + 0x2c));
            arg_1[0x32] = local_4.seat_allows;
            *(short *)(arg_1 + 0x38) = function_cdff0(*(long *)(local_2 + 0x2c), 0);
            *(short *)(arg_1 + 0x3a) = function_cdff0(*(long *)(local_2 + 0x2c), 1);
            *(short *)(arg_1 + 0x1e) = *(short *)(local_3 + 0x10a);
            arg_1[0x3e] = false;
            if (*(short *)(arg_1 + 0x3c) != NONE && function_cbd50(*(long *)(local_2 + 0x2c),
                (short)*(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x212)) != NONE)
            {
                if (!function_100f70(function_cbd50(*(long *)(local_2 + 0x2c),
                    (short)*(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x212))))
                    arg_1[0x3e] = true;
            }
            arg_1[0x3f] = function_15fef0(local_1);
            arg_1[0x40] = function_15eb20(local_1);
            *(real *)(arg_1 + 0x44) = function_c53c0(*(long *)(local_2 + 0x2c));
            *(long *)arg_1 = function_22bfc0((s_22bfc0_definition const *)g_4e3b44[*(long *)local_3 & 0xffff].bytes,
                g_4e6948->state == 2);
            if (*(long *)(local_3 + 0x14) != NONE && *(short *)(local_3 + 0x1fc) != NONE)
            {
                byte const *local_7 = (byte const *)OBJECT_DATUM_GET(*(long *)(local_3 + 0x14));
                byte const *local_8 = g_4e3b44[*(long *)local_7 & 0xffff].bytes;
                byte const *local_9 = *(byte *const *)(local_8 + 0x1cc) + *(short *)(local_3 + 0x1fc) * 0xb0;
                arg_1[0x48] = true;
                *(long *)(arg_1 + 4) = function_22bff0((s_22bff0_definition const *)g_4e3b44[*(long *)local_7 & 0xffff].bytes,
                    *(short *)(local_3 + 0x1fc), g_4e6948->state == 2);
                if (!(bool)((*(dword *)local_9 >> 5) & 1)) local_5 = true;
                if ((bool)((*(dword *)local_9 >> 3) & 1))
                {
                    long local_10 = function_cbd50(*(long *)(local_3 + 0x14),
                        *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_3 + 0x14)) + 0x212));
                    if (local_10 == NONE)
                    {
                        long local_11 = *(long *)(local_7 + 0x10);
                        while (local_11 != NONE)
                        {
                            byte const *local_12 = (byte const *)OBJECT_DATUM_GET(local_11);
                            if ((bool)((*(dword *)(local_12 + 4) >> 26) & 1)
                                && ((1 << local_12[0xaa]) & 3)
                                && (bool)((*(dword *)(g_4e3b44[*(long *)local_12 & 0xffff].bytes + 0xbc) >> 26) & 1))
                            {
                                local_10 = function_cbd50(local_11, *(signed char *)(local_12 + 0x212));
                                if (local_10 != NONE) break;
                            }
                            local_11 = *(long *)(local_12 + 0xc);
                        }
                    }
                    if (local_10 != NONE)
                        function_13afa9(local_10, (long *)(arg_1 + 8), (s_hud_weapon_record *)(arg_1 + 0x50));
                }
            }
            if (!local_5)
            {
                if (function_cbd50(*(long *)(local_2 + 0x2c),
                    *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x212)) != NONE)
                {
                    function_13afa9(function_cbd50(*(long *)(local_2 + 0x2c),
                        *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x212)),
                        (long *)(arg_1 + 0xc), (s_hud_weapon_record *)(arg_1 + 0x88));
                }
                if (function_cbd50(*(long *)(local_2 + 0x2c),
                    *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x213)) != NONE)
                {
                    function_13afa9(function_cbd50(*(long *)(local_2 + 0x2c),
                        *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x213)),
                        (long *)(arg_1 + 0x10), (s_hud_weapon_record *)(arg_1 + 0xc0));
                }
                if (function_cbd50(*(long *)(local_2 + 0x2c),
                    *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x213)) != NONE)
                {
                    function_13afa9(function_cbd50(*(long *)(local_2 + 0x2c),
                        *(signed char *)((byte const *)OBJECT_DATUM_GET(*(long *)(local_2 + 0x2c)) + 0x213)),
                        (long *)(arg_1 + 0x10), (s_hud_weapon_record *)(arg_1 + 0xc0));
                }
            }
            long local_13 = function_cfe40(*(long *)(local_2 + 0x2c));
            long local_14 = function_cbe60(*(long *)(local_2 + 0x2c));
            if (local_13 != NONE)
            {
                long local_15 = NONE;
                local_14 = NONE;
                for (long local_16 = 0; local_16 < 4; local_16++)
                {
                    if (local_16 != local_13 && *(long *)(local_3 + 0x218 + local_16 * 4) != NONE
                        && *(long *)(local_3 + 0x228 + local_16 * 4) > local_15)
                    {
                        local_15 = *(long *)(local_3 + 0x228 + local_16 * 4);
                        local_14 = *(long *)(local_3 + 0x218 + local_16 * 4);
                    }
                }
            }
            if (!local_5 && local_14 != NONE)
                function_13afa9(local_14, (long *)(arg_1 + 0x14), (s_hud_weapon_record *)(arg_1 + 0xf8));
            function_13b164(*(long *)(local_2 + 0x2c), (s_13b164_status *)arg_1);
        }
        if (function_15dea0())
        {
            byte const *local_17 = *(byte **)(g_4e3b44[*(long *)((byte *)g_4e034c + 0x16c) & 0xffff].bytes + 0xc);
            arg_1[0x131] = 2 + (function_15eaf0() ? 1 : 0);
            s_player_score_display local_18;
            if (function_15f3a0(arg_2, &local_18))
            {
                arg_1[0x130] = true;
                *(long *)(arg_1 + 0x18) = *(long *)(local_17 + 0x544);
                arg_1[0x174] = local_18.timer;
                unicode_string_copy((word *)(arg_1 + 0x176), local_18.timer_text, 8);
                arg_1[0x132] = local_18.teams;
                arg_1[0x133] = local_18.leading;
                arg_1[0x1d0] = local_18.rival;
                arg_1[0x21c] = local_18.charge;
                *(real *)(arg_1 + 0x220) = local_18.charge_fraction;
                unicode_string_copy((word *)(arg_1 + 0x134), local_18.name, 0x20);
                *(color3f *)(arg_1 + 0x18c) = local_18.color;
                *(long *)(arg_1 + 0x188) = local_18.player_index;
                unicode_string_copy((word *)(arg_1 + 0x1bc), local_18.score_text, 8);
                *(real *)(arg_1 + 0x1cc) = !local_18.limit ? (real)local_18.score / (real)MAX(local_18.score, MAX(local_18.rival_score, 1))
                    : (real)local_18.score / (real)local_18.limit;
                if (local_18.rival)
                {
                    *(color3f *)(arg_1 + 0x1d8) = local_18.rival_color;
                    *(long *)(arg_1 + 0x1d4) = local_18.rival_index;
                    unicode_string_copy((word *)(arg_1 + 0x208), local_18.rival_text, 8);
                    *(real *)(arg_1 + 0x218) = !local_18.limit ? (real)local_18.rival_score / (real)MAX(local_18.score, MAX(local_18.rival_score, 1))
                    : (real)local_18.rival_score / (real)local_18.limit;
                }
                *(long *)(arg_1 + 0x22c) = local_18.zone_count;
                for (long local_20 = 0; local_20 < *(long *)(arg_1 + 0x22c); local_20++)
                {
                    *(dword *)(arg_1 + 0x230 + local_20 * 4) = local_18.zone_colors[local_20];
                    *(real *)(arg_1 + 0x250 + local_20 * 4) = local_18.zone_progress[local_20];
                }
            }
            else arg_1[0x130] = false;
            *(long *)(arg_1 + 0x228) = function_162fd0(arg_2);
            arg_1[0x224] = *(long *)(arg_1 + 0x228) != NONE;
        }
        else
        {
            memset(arg_1 + 0x130, 0, 0x140);
            arg_1[0x130] = true;
            arg_1[0x131] = function_138820() ? 1 : 0;
        }
    }
}
