// @flags /O2 /Ob1 /arch:SSE /Gr
/* UNKNOWN_19EC40.CPP: finds entries of the 4e0350 globals' second table by
   position and by three optional 16-bit keys */

#include "unknown_11c920.h"
#include "globals.h"
#include "unknown_0259d0.h"
#include "unknown_19ec40.h"
#include <math.h>

real distance_sq3f(point3f const *a, point3f const *b);

// score 65 (was 65): retail keeps the globals in EBX, point in EBP, i in EDI, entry in ESI (pushed
// after the count test) and reloads each key into CX; ours puts entry in ECX, so each key gets BP (the
// first one hoisted to the preheader and the back edge) and the globals, point and i rotate to EDI,
// EBX and ESI: the same seven registers in another assignment. The fast check needs distance_sq3f
// (0x24550) in the image (105 without it; it also reads 105 for any edit, so use a plain check or a
// one-file LTCG harness with src/unknown_023540.cpp, which reproduces the build). Tried: unsigned key
// bits (a word key compared with each field cast to word) plus entry position 61, not kept: the casts
// only steer the compares; entry position alone 112; separate keys 61; read-only globals 61;
// function-scope position 61; wrapped index 105; direct count 61; explicit append 61; height input 61;
// distance inline 65; force-inline 65; count global at use 65; count and inline 65; entry position copy
// 64; entry copy and force-inline 64; capacity lifetime 65; vertical difference 65; the keys read as
// *(short const volatile *)&key_a etc. 99 (the prologue then matches and point_reference is no longer
// needed, but each key loads as a dword into esi, entry stays in ecx and the loop top loses its
// aligning jmp), key_a alone 97, with distance = 0 / if (point) 102, through an inline key-match helper
// 102; distance = 0 / if (point) alone 102 (retail's movaps-before-je block order: aligned 46 against
// 53, same registers); one && chain without the globals and point_reference locals 117;
// point_reference dropped 71 (point becomes a register argument); globals local dropped 113; inline
// key-match helper 65; entry at function scope 65; field-first key compares 65; (void)& of the key
// parameters (no effect, 102 with the if (point) shape); keys compared without the key temporary 65;
// entry declared first 65; point test with the arms swapped worse; position copied from globals or
// from entry 64 (from entry drops retail's table reload); one && chain of the key tests 65; `continue`
// per key test 65 (all three key forms give identical code); every key test through
// globals->marker_entries[i] without entry 138 (378 bytes); i declared at the top 65; count declared
// first 65; globals declared before point_reference 65; the entry copy with distance = 0 / if (point)
// 102. Round 73: formal word keys 65 (two matched callers differ); entry-pointer traversal 106; output
// pointer 65; memcpy append 97; full-width result index 65; mixed parameter/temporary key tests 65/65;
// reused entry-field temporary 65.
// Round 96 (harness, aligned counts; base 43): 121 random combinations of point_reference/plain point,
// globals local/global, key temporary/one condition/continue, entry pointer/indexed, if-else/initialised
// distance, position source and four declaration orders: none below 42 (initialised distance with the
// entry position 42 at 102 bytes); a named `long index = i` for both result stores 43; const entry 43;
// split `results[count] = i; count++;` 43; capacity test inverted 45; plain point with the global 52.
// 102.
// key preheader and global/point/index/entry registers still differ.
// Round 73: formal word keys 65 (two matched callers differ); entry-pointer traversal 106; output pointer 65;
// memcpy append 97; full-width result index 65; mixed parameter/temporary key tests 65/65;
// reused entry-field temporary 65. Opening signature and body restored.
// Unsigned key bits (a word key compared with each field cast to word) plus entry position 61,
// not kept: the casts only steer the compares; entry position alone 112; separate keys 61; read-only globals 61;
// function-scope position 61; wrapped index 105; direct count 61;
// explicit append 61; height input 61. Existing point handling retained.
// preheader key and entry registers remain; distance inline 65; force-inline 65; count global at use 65; count and inline 65; entry position copy 64; entry copy and force-inline 64; capacity lifetime 65; vertical difference 65.
// The fast check needs distance_sq3f (0x24550) in the image (105 without it). Retail reloads each key as a
// word into cx at the loop top, keeps entry in esi, point in ebp, the globals in ebx and i in edi; ours
// hoists key_a into bp (preheader and back edge) and keeps entry in ecx. Tried: the keys read as
// *(short const volatile *)&key_a etc. 99 (the prologue then matches and point_reference is no longer
// needed, but each key loads as a dword into esi, entry stays in ecx and the loop top loses its aligning
// jmp), key_a alone 97, with distance = 0 / if (point) 102, through an inline key-match helper 102;
// distance = 0 / if (point) alone 102 (retail's movaps-before-je shape); one && chain without the
// globals and point_reference locals 117; point_reference dropped 71 (point becomes a register
// argument); globals local dropped 113; inline key-match helper 65; entry at function scope 65;
// field-first key compares 65; (void)& of the key parameters (no effect, 102 with the if (point) shape).
// keys compared without the key temporary 65; entry declared first 65;
// point test with the arms swapped worse; position copied from globals or from entry 64.
// Round 8: declarations reordered (globals before the reference; globals and count first; entry
// before key) 65 each; `distance_squared = 0.0f; if (*point_reference)` 102 positional, but aligned
// 78 -> 48 and register-masked 20 -> 18: then only the register roles differ (ours point EBX, key BP
// with a preheader copy, globals EDI, entry ECX; retail globals EBX, point EBP, entry ESI, key CX);
// with the keys compared directly or as `continue` tests the same code; position from globals 115.
// first one loaded in the preheader and at the back edge) and the globals, point and i rotate to EDI,
// EBX and ESI. The fast check needs distance_sq3f (0x24550) in the image (105 without it). Tried:
// unsigned key bits (a word key compared with each field cast to word) plus entry position 61, not
// kept: the casts only steer the compares; entry position alone 112; separate keys 61; read-only
// globals 61; function-scope position 61; wrapped index 105; direct count 61; explicit append 61;
// height input 61; distance inline 65; force-inline 65; count global at use 65; count and inline 65;
// entry position copy 64; entry copy and force-inline 64; capacity lifetime 65; vertical difference
// 65; the keys read as *(short const volatile *)&key_a etc. 99 (the prologue then matches and
// point_reference is no longer needed, but each key loads as a dword into esi, entry stays in ecx and
// the loop top loses its aligning jmp), key_a alone 97, with distance = 0 / if (point) 102, through an
// inline key-match helper 102; distance = 0 / if (point) alone 102 (retail's movaps-before-je block
// order: aligned 46 against 53, same registers; then only the register roles differ); one && chain
// without the globals and point_reference locals 117; point_reference dropped 71 (point becomes a
// register argument); globals local dropped 113; inline key-match helper 65; entry at function scope
// 65; field-first key compares 65; (void)& of the key parameters (no effect, 102 with the if (point)
// shape); keys compared without the key temporary 65; entry declared first 65; point test with the
// arms swapped worse; position copied from globals or from entry 64 (from entry drops retail's table
// reload); one && chain of the key tests 65; `continue` per key test 65 (all three key forms give
// identical code); every key test through globals->marker_entries[i] without entry 138 (378 bytes); i
// declared at the top 65; count declared first 65; globals declared before point_reference 65; the
// entry copy with distance = 0 / if (point) 102; declarations reordered (globals before the reference;
// globals and count first; entry before key) 65 each; position from globals 115; formal word keys 65
// (two matched callers differ); entry-pointer traversal 106; output pointer 65; memcpy append 97;
// full-width result index 65; mixed parameter/temporary key tests 65/65; reused entry-field temporary
// 65; const entry pointer 65; key declared at function scope 65; radius squared at its use 92; entry
// as `marker_entries + i` 65; count and best declared before radius_squared 65.
// 65; the keys read through volatile casts 99 (the prologue then matches, but each key loads as a
// dword into esi, entry stays in ecx and the loop top loses its aligning jmp), key_a alone 97, with
// distance = 0 / if (point) 102, through an inline key-match helper 102; distance = 0 / if (point)
// alone 102 positional (retail's movaps-before-je block order; aligned 78 -> 48, register-masked
// 20 -> 18: then only the register roles differ); one && chain without the globals and
// point_reference locals 117; point_reference dropped 71 (point becomes a register argument);
// globals local dropped 113; inline key-match helper 65; entry at function scope 65; field-first key
// compares 65; (void)& of the key parameters (no effect); keys compared without the key temporary
// 65; entry declared first 65; point test with the arms swapped worse; position copied from globals
// or from entry 64 (position from globals 115 with the reordered declarations); one && chain of the
// key tests 65; `continue` per key test 65 (all three key forms give identical code); every key test
// through globals->marker_entries[i] without entry 138 (378 bytes); i declared at the top 65; count
// declared first 65; globals declared before point_reference 65; formal word keys 65 (two matched
// callers differ); entry-pointer traversal 106; output pointer 65; memcpy append 97; full-width
// result index 65; mixed parameter/temporary key tests 65/65; reused entry-field temporary 65.
// Round 35: a 40-form screen of key form (temporary / direct / continue) x point test x position
// source (g_4e0350 / globals / entry) x declaration order x entry before the key: best 64 (position
// from entry), every distance = 0 / if (point) row 102-115.
// retail keeps the globals in EBX, point in EBP (so it stays a stack parameter), i in
// EDI, entry in ESI (pushed after the count test) and reloads each key into CX; ours puts entry in ECX, so
// key_a is enregistered in BP (a preheader copy and a reload at the back edge) and the globals, point and
// i rotate to EDI, EBX and ESI. The fast check needs distance_sq3f (0x24550) in the image (105 without
// it). `distance_squared = 0.0f; if (*point_reference)` gives retail's block layout (102 positional,
// Earlier note: retail keeps the globals in EBX, point in EBP (so it stays a stack parameter), i
// in EDI, entry in ESI (pushed after the count test) and reloads each key into CX; ours puts entry in
// ECX, so key_a is enregistered in BP (a preheader copy and a reload at the back edge) and the
// globals, point and i rotate to EDI, EBX and ESI: the same seven registers in another assignment.
// The fast check needs distance_sq3f (0x24550) in the image (105 without it): list 0x24550 with it
// (65 then), or use a plain check or a one-file LTCG harness with src/unknown_023540.cpp.
// `distance_squared = 0.0f; if (*point_reference)` gives retail's block layout (102 positional,
// aligned 78 -> 48, register-masked 20 -> 18): then only the register roles differ.
// Tried: unsigned key bits (a word key compared with each field cast to word) plus entry position 61, not
// kept: the casts only steer the compares; entry position alone 112; separate keys 61; read-only globals
// 61; function-scope position 61; wrapped index 105; direct count 61; explicit append 61; height input 61;
// volatile 99 (prologue matches, but each key loads as a dword into ESI; key_a alone 97, with the if
// (point) shape 102, through an inline key-match helper 102); one && chain without the globals and
// point_reference locals 117; point_reference dropped 71 (point becomes a register argument); globals
// local dropped 113; inline key-match helper 65; entry at function scope 65; field-first key compares 65;
// (void)& of the key parameters (no effect); keys compared without the key temporary 65; entry declared
// first 65; point test with the arms swapped worse; position copied from globals or from entry 64/115;
// `continue` per key test 65; every key test through globals->marker_entries[i] without entry 138 (378
// bytes); i declared at the top 65; count declared first 65; declarations reordered 65 each. Round 73:
// formal word keys 65 (two matched callers differ); entry-pointer traversal 106; output pointer 65; memcpy
// append 97; full-width result index 65; mixed parameter/temporary key tests 65; reused entry-field
// temporary 65.
// Round 102 (one-file harness, exact): (void)&point in place of point_reference 71; the if (point) shape
// with one && key chain and a valid flag 102; the keys read through short const * locals or a pointer
// helper 102 (folded, identical); marker_entries + i 65; const entry 65; position through globals 113;
// height read through its address 100; point_reference after the globals or in the loop, a point copy
// local 102/65; position or distance declared at function scope or the loop top 65. Diagnostics: with no
// key tests the globals/point/i rotation stays, so the key enregistration is not the root.
// Tried: unsigned key bits (a word key compared with each field cast to word) plus entry position
// 61, not kept: the casts only steer the compares; entry position alone 112; separate keys 61;
// read-only globals 61 (an earlier body); function-scope position 61; wrapped index 105; direct
// count 61; explicit append 61; height input 61; distance inline 65; force-inline 65; count global
// at use 65; count and inline 65; entry position copy 64; entry copy and force-inline 64; capacity
// lifetime 65; vertical difference 65; the keys read through volatile casts 99 (the prologue then
// matches and point_reference is no longer needed, but each key loads as a dword into ESI, entry
// stays in ECX and the loop top loses its aligning jmp), key_a alone 97, with distance = 0 / if
// (point) 102, through an inline key-match helper 102; one && chain without the globals and
// point_reference locals 117; point_reference dropped 71 (point becomes a register argument), or
// (void)&point in its place 71; globals local dropped 113; inline key-match helper 65; entry at
// function scope 65; field-first key compares 65; (void)& of the key parameters (no effect, 102
// with the if (point) shape); keys compared without the key temporary 65; entry declared first 65;
// point test with the arms swapped worse; position copied from globals or from entry 64 (from
// entry drops retail's table reload), from globals with reordered declarations 115, through
// globals 113; one && chain of the key tests 65; `continue` per key test 65 (all three key forms
// give identical code); every key test through globals->marker_entries[i] without entry 138 (378
// bytes); i declared at the top 65; count declared first 65; declarations reordered (globals
// before the reference; globals and count first; entry before key) 65 each; the entry copy with
// distance = 0 / if (point) 102; formal word keys 65 (two matched callers differ); entry-pointer
// traversal 106; output pointer 65; memcpy append 97; full-width result index 65; mixed
// parameter/temporary key tests 65/65; reused entry-field temporary 65; const entry pointer 65;
// key declared at function scope 65; radius squared at its use 92; entry as `marker_entries + i`
// 65; count and best declared before radius_squared 65. Round 35: a 40-form screen of key form
// (temporary / direct / continue) x point test x position source (g_4e0350 / globals / entry) x
// declaration order x entry before the key: best 64 (position from entry), every distance = 0 /
// if (point) row 102-115. Round 96 (harness, aligned counts; base 43): 121 random combinations of
// point_reference/plain point, globals local/global, key temporary/one condition/continue, entry
// pointer/indexed, if-else/initialised distance, position source and four declaration orders: none
// below 42 (initialised distance with the entry position 42 at 102 bytes); a named `long index =
// i` for both result stores 43; const entry 43; split `results[count] = i; count++;` 43; capacity
// test inverted 45; plain point with the global 52. Round 102 (one-file harness, exact): the if
// (point) shape with one && key chain and a valid flag 102; the keys read through short const *
// locals or a pointer helper 102 (folded, identical); height read through its address 100;
// point_reference after the globals or in the loop, a point copy local 102/65; position or
// distance declared at function scope or the loop top 65. Diagnostic: with no key tests the
// globals/point/i rotation stays, so the key enregistration is not the root. Round 41 (fast):
// const globals pointer 65, with const entry 65, with distance = 0 / if (point) 102 (aligned 53),
// with the entry position 64, all four 102.
// One shared result-index store, initialized/deferred index ownership, optional-distance
// initialization and read-only table/entry receivers 77/77/105/105/77/77/105/105. Key preheader
// and global/point/index/entry homes remain; original body restored.
// Round 34 (fast, with 0x24550): the entry taken from g_4e0350 (the globals local left for the
// bound) 113; the globals local renamed to sort first/last 65/65 (names don't move registers);
// every point read through point_reference 65 (same code); a long key temporary 116 (370 bytes);
// keys compared without the temporary 65 again as a diagnostic (identical code: the first key
// load is still placed in the preheader and on the back edge, in BP).
// Round 3 (lane 18, plain listing: distance_sq3f is inlined here, so no callee clobber set is involved):
// split result store 65; fabsf for the height test 65; the globals local declared in the loop body (bound
// read from g_4e0350) 113 (334 bytes); capacity test inverted (full arm first) 73; best stored before
// results[0] 66.
// @retail 0x19ec40
long function_19ec40(
	point3f const *point,
	real height,
	short key_a,
	short key_b,
	short key_c,
	long maximum_count,
	long *results,
	real radius)
{
	point3f const *const *point_reference = &point;
	s_palette_source_globals *globals = g_4e0350;
	real radius_squared = radius * radius;
	real best = 0.0f;
	long count = 0;

	for (short i = 0; i < globals->marker_count; i++)
	{
		short key = key_a;
		s_marker_entry *entry = &globals->marker_entries[i];

		if (key == -1 || key == entry->key_a)
		{
			key = key_b;
			if (key == -1 || key == entry->key_b)
			{
				key = key_c;
				if (key == -1 || key == entry->key_c)
				{
					real distance_squared;

					if (!*point_reference)
						distance_squared = 0.0f;
					else
					{
						point3f position = g_4e0350->marker_entries[i].position;
						distance_squared = distance_sq3f(&position, point);
						if (radius >= 0.0f && distance_squared > radius_squared)
							continue;
						if (height > 0.0f && fabs(entry->position.z - point->z) > height)
							continue;
					}

					if (count < maximum_count)
						results[count++] = i;
					else if (count == 1 && best > distance_squared)
					{
						results[0] = i;
						best = distance_squared;
					}
				}
			}
		}
	}

	return count;
}
