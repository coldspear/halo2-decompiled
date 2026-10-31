// @flags /O2 /arch:SSE /Gr
/* UNKNOWN_1FDBF0.CPP: tests of an actor's combat state (+0x6fe, +0x722) */

#include "unknown_11c920.h"
#include "globals.h"
#include "unknown_1e3920.h"
#include "unknown_0259d0.h"
#include "slot_handler.h"
#include "props.h"

/* the prop view (unknown_25d690.cpp) starts with its state */
struct s_type_f95cd3;
s_type_f95cd3 *function_25d700(long index);

struct s_combat_prop_view
{
	short state;
};

PRIVATE __forceinline bool function_1fdbf1(s_actor_moving *arg_0)
{
    bool local_0 = false;
    s_combat_prop_view *local_1;
    local_0 = arg_0->unknown722 == 0 && arg_0->prop_index != NONE &&
        (local_1 = (s_combat_prop_view *)function_25d700(arg_0->prop_index)) != 0 && local_1->state >= 4 &&
        (real)arg_0->unknown350 * g_510c54->rate >= 2.5f;
    return local_0;
}

// @retail 0x1fdbf0
bool function_1fdbf0(long actor_index, short type)
{
	s_actor_moving *actor = actor_moving_get(actor_index);
	bool result = false;
	s_combat_prop_view *view;

	switch (type)
	{
	case 1:
		result = actor->unknown722 == 1 && actor->prop_index != NONE &&
			(view = (s_combat_prop_view *)function_25d700(actor->prop_index)) != 0 && view->state >= 6;
		break;
	case 2:
		result = function_1fdbf1(actor);
		break;
	case 3:
		result = actor->unknown722 == 1 && actor->prop_index != NONE &&
			(view = (s_combat_prop_view *)function_25d700(actor->prop_index)) != 0 && view->state >= 6 &&
			actor->unknown268;
		break;
	}
	return result;
}
// @retail 0x1fdd90
bool function_1fdd90(long actor_index)
{
	s_actor_moving *actor = actor_moving_get(actor_index);
	bool result = false;

	if (actor->unknown722 > 0)
		result = actor->unknown6fe == 2;
	return result;
}

/* the weapon object the actor holds (unknown_1e1f20.cpp) */
long function_1e1f20(long actor_index);

struct s_type_67e06b;

// @retail 0x1fe0e0
s_type_67e06b *function_1fe0e0(long actor_index)
{
	s_type_67e06b *result = 0;
	long weapon_index = function_1e1f20(actor_index);

	if (weapon_index != NONE)
	{
		s_moving_object *weapon = moving_object_get(weapon_index);

		result = (s_type_67e06b *)g_4e3b44[weapon->tag_index & 0xffff].bytes;
	}
	return result;
}

/* the seconds range of a character's combat behaviour */
struct s_combat_delay
{
	byte unknown00[0x48];
	real delay_lower;
	real delay_upper;
};

real function_1e96a0(short column, short row);

/* starts the actor's combat delay (+0x700, in ticks); false once it is engaged */
// @retail 0x1fee20
bool function_1fee20(long actor_index, s_combat_delay const *delay)
{
	s_actor_moving *actor = actor_moving_get(actor_index);
	bool engaged = actor->unknown48b;

	if (actor->unknown722 == 1 && prop_node_get(actor->unknown724)->unknown24 >= 3)
		engaged = true;
	if (!engaged && delay)
	{
		real seconds = function_259d0(&g_4e7408->unknown0, __FILE__, __LINE__, delay->delay_lower, delay->delay_upper);
		real ticks;
		long result;

		seconds *= function_1e96a0(g_4e6948->state == 1 ? g_4e6948->difficulty : 1, 13);
		ticks = seconds * (real)g_510c54->field_2_3;
		__asm
		{
			fld ticks
			fistp result
		}
		actor->unknown700 = (short)result;
	}
	else
	{
		actor->unknown700 = 0;
	}

	return !engaged;
}

real function_1e9720(short kind, short team);

struct s_combat_wait_range
{
	byte unknown00[0x28];
	real lower;
	real upper;
};

// @retail 0x1fed70
void function_1fed70(long actor_index, s_combat_wait_range const *range)
{
	s_actor_moving *actor = actor_moving_get(actor_index);
	double upper = range->upper;
	double lower = range->lower;
	real seconds = (real)(lower + (upper - lower) * slot_random());
	real ticks;
	long result;
	seconds *= function_1e9720(14, *(short *)((byte *)actor + 0x24));
	ticks = (real)g_510c54->field_2_3 * seconds;
	__asm
	{
		fld ticks
		fistp result
	}
	actor->unknown700 = (short)result;
}

void function_118e80(long object_index, vector3f *forward);
bool __stdcall function_cba50(long unit_index, vector3f *direction, bool looking);

struct s_aim_definition
{
	byte unknown00[0x1ec];
	dword flags;
};

// @retail 0x1fe120
void function_1fe120(long actor_index, vector3f *direction)
{
	s_actor_moving *actor = actor_moving_get(actor_index);
	long object_index = actor->unit_index;
	if (actor->unknown268)
	{
		if (actor->unknown274 != NONE)
		{
			object_index = actor->unknown274;
			s_object_header_view *header = object_header_get(actor->unknown274);
			if (header->type == 1)
			{
				s_moving_object *object = (s_moving_object *)header->object;
				s_aim_definition *definition = (s_aim_definition *)g_4e3b44[object->tag_index & 0xffff].bytes;
				if ((bool)((definition->flags >> 8) & 1))
				{
					function_118e80(actor->unknown274, direction);
					return;
				}
			}
		}
	}
	*direction = *(vector3f *)(object_header_get(object_index)->object + 0x168);
	function_cba50(object_index, direction, false);
}

struct s_combat_blend_values
{
	real field_0;
	real field_4;
	real field_8;
	real field_c;
	real field_10;
	real field_14;
	real field_18;
	real field_1c;
	real field_20;
	real field_24;
	real field_28;
	real field_2c;
	real field_30;
	real field_34;
	real field_38;
	real field_3c;
};

struct s_combat_blend_block
{
	byte unknown00[8];
	long count;
	s_combat_blend_values *values;
};

void *actor_get_character_weapon_extra(long actor_index, long key);

// @retail 0x1fe1e0
bool function_1fe1e0(long actor_index, real blend, s_combat_blend_values *out)
{
	bool result = false;
	long weapon_index = function_1e1f20(actor_index);
	if (weapon_index != NONE)
	{
		s_combat_blend_block *block = (s_combat_blend_block *)actor_get_character_weapon_extra(actor_index, moving_object_get(weapon_index)->tag_index);
		if (block && block->count > 0)
		{
			s_combat_blend_values const *lower = &block->values[0];
			result = true;
			if (block->count < 2)
				*out = *lower;
			else
			{
				s_combat_blend_values const *upper = &block->values[1];
				real local_0 = *(real const volatile *)&upper->field_0;
				local_0 *= blend;
				local_0 += lower->field_0 * (1.0f - blend);
				out->field_0 = local_0;
				out->field_4 = lower->field_4 * (1.0f - blend) + upper->field_4 * blend;
				out->field_8 = lower->field_8 * (1.0f - blend) + upper->field_8 * blend;
				out->field_c = lower->field_c * (1.0f - blend) + upper->field_c * blend;
				out->field_10 = lower->field_10 * (1.0f - blend) + upper->field_10 * blend;
				out->field_14 = lower->field_14 * (1.0f - blend) + upper->field_14 * blend;
				out->field_18 = lower->field_18 * (1.0f - blend) + upper->field_18 * blend;
				out->field_1c = lower->field_1c * (1.0f - blend) + upper->field_1c * blend;
				out->field_20 = lower->field_20 * (1.0f - blend) + upper->field_20 * blend;
				out->field_24 = lower->field_24 * (1.0f - blend) + upper->field_24 * blend;
				out->field_28 = lower->field_28 * (1.0f - blend) + upper->field_28 * blend;
				out->field_2c = lower->field_2c * (1.0f - blend) + upper->field_2c * blend;
				out->field_30 = lower->field_30 * (1.0f - blend) + upper->field_30 * blend;
				out->field_34 = lower->field_34 * (1.0f - blend) + upper->field_34 * blend;
				out->field_38 = lower->field_38 * (1.0f - blend) + upper->field_38 * blend;
				out->field_3c = lower->field_3c * (1.0f - blend) + upper->field_3c * blend;
			}
		}
	}
	return result;
}

bool function_1ff7d0(long actor_index, point3f const *point, real enemy_radius, real friendly_radius, short *count_out);
void *function_1e5380(long actor_index);

PRIVATE __forceinline bool function_1fdce1(long arg_0)
{
	bool result = true;
	s_actor_view *actor = actor_get(arg_0);
	s_prop_node_view *node = prop_node_get(*(long *)((byte *)actor + 0x724));
	s_type_5cfb45 *state = function_25d690((s_prop_datum *)node);
	if (state->unknown3c != NONE)
		return true;
	byte *prop = g_50241c->data + (node->unknown08 & 0xffff) * 0xc4;
	if (prop[0x25])
		return false;
	short count = 0;
	function_1ff7d0(arg_0, &state->position, 6.0f, 0.0f, &count);
	result = count >= 3;
	return result;
}

// @retail 0x1fdce0
bool function_1fdce0(long actor_index, short type)
{
	bool result = true;
	if (type == 3)
	{
		result = function_1fdce1(actor_index);
	}
	return result;
}

// @retail 0x1fef40
bool function_1fef40(s_type_c3b527 const *target, long actor_index)
{
	bool result = false;
	s_actor_view *actor = actor_get(actor_index);
	byte *ranges = (byte *)function_1e5380(actor_index);
	point3f point;
	if (target->output_index != NONE)
	{
		if (!function_2104b0(target->output_index, &target->point, &point))
			point = target->point;
	}
	else
	{
		point = target->point;
	}
	if (ranges && function_1ff7d0(actor_index, &point, *(real *)(ranges + 0xc), *(real *)(ranges + 0x20), NULL))
	{
		*(s_type_c3b527 *)((byte *)actor + 0x7d0) = *target;
		result = true;
	}
	return result;
}

real function_30bf0(vector3f *v);
vector3f *function_11d000(vector3f const *v, vector3f *out);

// score 43 (was 43): only the final rotation differs, and only in register colours: retail loads
// axis k/j into xmm1/xmm2 and d.j/d.i into xmm3/xmm4 (ours xmm3/xmm4 and xmm1/xmm2) and scales axis
// j/k by the parallel part in place, where ours copies the part for j (three bytes longer). The rows
// as accumulated sums (round 35) give retail's i row and tail. Tried: products swapped (axis *
// parallel) 57; the four scalars declared before or after the axis or at the top 57, in all 24
// orders (one compile-only screen, again on the accumulated rows: none closer); the rotation as
// 0x11d3b0 writes it, or through an inline (axis, sine, cosine) helper 57; valid flag declared first
// 57; valid as an expression 86; member-wise forward copy 88; negated cosine test 100; the shared
// cross3f for the rotation's cross terms 69, for the axis and the rotation 70, for the axis only 64,
// the same with the cross k term first 60; parallel scale written into each row 78; local_3 before
// local_2 59; rows i, k, j 59; an inline rotate helper with named parallel/cross scalars 57;
// perpendicular/forward query inline and noinline declarations 57; fallback normalization via the
// returned output pointer 71. Screened compile-only (no closer form): the axis statements in all six
// orders or through cross3f, a cross3f/struct cross, named const sine/cosine, the parallel part
// accumulated or through a named dot, an inline (vector, axis, sine, cosine) helper in either
// parameter order (93/97). Round 35: the other seven accumulated term orders (the cosine term first
// in the i row, or the parallel term first in the j/k rows) 43-50 aligned in the harness (current
// 34 aligned), every row computed into locals before the stores 45, the dot accumulated 34, the
// first two terms of a row as one expression 34 (all three rows 35), the sine term second 41, a
// named cross vector 48, a by-value axis helper 42, an axis-pointer helper 35. Round 80 (LTCG harness
// of the file, which reproduces this block exactly): all 128 accumulated or single-expression row
// orders, 24 declaration orders and 6 dot orders on the best; the j/k rows as cosine, sine, parallel
// (one instruction longer; aligned 30 against 33, positional 56); the other files' rotation helper
// forms (projection or cross first, cross struct or scalars, copied components) 57 and up; named
// parallel components, the axis or a copy scaled through vector3f_scale 34-49 aligned (no closer).
// named cross vector 48, a by-value axis helper 42, an axis-pointer helper 35.
// Round 418 on accumulated rows: staged axis/turn/both crosses 67/69/51; complete
// direction scalars 74, with staged axis 81; complete direction value with staged
// turn/both 196/219; both vector inputs as complete scalars 74. All restored;
// final axis/direction carriers and the copied parallel product remain.
// Round 100: the existing cross-file forced-inline rotation helper (vector, axis, 0.5, 0.866) 69;
// identical code at 43: axis-first products in the j/k rows, axis declared before forward, the
// parallel part through dot3f, the i-row cross term computed last (the matched 0x11d3b0 colours
// its axis the way ours does).
// Round 119: retail multiplies axis j/k by the parallel part in their own registers (axis operand
// first), so it frees a register, loads direction j/k before the i store and keeps the cosine in a
// register; every source spelling of the product gives the parallel part first. Parallel term first
// in the j/k rows 57; the rows as single expressions in 0x11d3b0's order 57; axis j/k scaled in
// place after the i row 70; the rotation as a three-pass component loop (unrolled) 70; the i row
// computed before the j/k cross terms 57. Harness only: the axis through a pointer, dot3f, a
// cross3f vector, an inline rotate helper by pointer or by value, the parallel part scaled into a
// vector, operand text orders (identical code).
// Round 34: j/k rows as axis * parallel with the cosine term second 57; each row as one expression,
// parallel product first 57; axis scaled in place before the rows 80; valid at the state block's
// scope with the rotation after the dot block 43; the fallback normalizing 0x11d000's returned
// pointer 57 (axis/forward slots swap).
// Round 302: Actual axis at state/entry scope, forward at state/entry scope,
// each with original/role vector names: all eight 43. Original declarations,
// homes and body restored; axis/direction register colors and parallel copy remain.
// Round 13 (lane 13): retail = projection times axis j/k in place in the axis registers (cos then
// in a register), times axis i into a copy of the projection; ours copies it for i and j. Forms
// that do give the in-place products all add a block copy of the axis (harness: a local copy,
// a by-value inline helper; all three rows in place then); without one: grouped (d cos -
// cross sin) in the j/k rows 56, 0x11d3b0's form with named const sine/cosine 57, projection as
// (dot * c) per row 43, the i projection before the cross terms 43, parallel first in j/k 57;
// harness only: scalar axis locals in any order, all six dot orders as one expression (43) or
// accumulated (ds_kji 43, the others worse), product operands swapped per row (no change).
// Lane 18 r1: the first cross term written in the i row (retail computes it last) 43 (same code); the
// cross terms computed 2, 3, 1 as retail schedules them 43 (same code); the rotation reading the forward
// copy instead of the direction 86; the cross terms named with their 0.5 scale 75; the rows accumulated
// in place in the direction (i as one expression) 57.
// Lane 13 round 18: the matched 0x11d3b0 (same turn-limiting algorithm, out of line) compiles its
// rotation with exactly our colours (axis k/j in xmm3/xmm4, the projection copied for j), so
// retail's 0x1fddd0 comes from another shape. An inline rotation helper (vector, axis,
// projection, sine, cosine) called with the projection 43 (same code); one computing the
// projection itself 43; either with single-expression rows 57 (38/11 aligned against 46/17: the
// cosine is then held in a register as in retail, colours still ours); with the projection term
// first in every row 57.
// Round 45 (lane 9): scalar axis copies read after the parallel factor 43 (same code); the parallel factor computed
// from forward before the direction copy 73; from forward after it 69; cross terms before the parallel factor 53; the
// parallel part through vector3f_scale into a vector 88.
// Round 107 (fast): i, j, k declared before the parallel factor 43; the rows into a rotated vector stored
// once 100 (796 bytes); axis-first parallel products in all rows 43; a named cosine local 43; scalar axis
// copies declared before the parallel factor 43 (all the same code but the copy).
// Lane 14 r19 (fast `0x1fddd0 0x1fe120 0x30bf0 0x11d000` reproduces 43; no other retail code uses
// the 0.1339746 constant, and the matched 0.866 users are not rotations): named j/k projection
// products before the i row 57; the projection defined after the cross terms 53; axis j/k read into
// scalars after the projection, axis first 43 (same code); axis-first j/k products with the i and j
// stores together 59 (776 bytes); projection term first in every row with `axis.i * local_0` 57.
// @retail 0x1fddd0
bool function_1fddd0(long actor_index, point3f const *origin, vector3f *direction,
    long *out_index, real *arg_c9241c, long *out_object)
{
    byte *actor = (byte *)actor_moving_get(actor_index);
    long object_index = NONE;
    bool result = false;
    short state = *(short *)(actor + 0x6fe);
    if (state == 4)
        result = true;
    else if (state == 2)
    {
        result = true;
        if (*(short *)(actor + 0x722) == 1 && *(long *)(actor + 0x724) != NONE)
        {
            s_prop_node_view *node = prop_node_get(*(long *)(actor + 0x724));
            if (node->unknown24 >= 1)
                object_index = *(long *)((byte *)node + 0x20);
        }
        if (actor[0x7a8])
        {
            *direction = *(vector3f *)(actor + 0x7b0);
            if (out_index)
                *out_index = *(long *)(actor + 0x7ac);
        }
        else
        {
            point3f *target = (point3f *)(actor + 0x79c);
            direction->i = target->x - origin->x;
            direction->j = target->y - origin->y;
            direction->k = target->z - origin->z;
            function_30bf0(direction);
        }
        vector3f forward;
        function_1fe120(actor_index, &forward);
        real dot = direction->k * forward.k;
        dot += direction->j * forward.j;
        dot += direction->i * forward.i;
        if (dot < 0.8660253882408142f)
        {
            vector3f axis;
            bool valid = true;
            axis.i = direction->k * forward.j - direction->j * forward.k;
            axis.j = direction->i * forward.k - direction->k * forward.i;
            axis.k = direction->j * forward.i - direction->i * forward.j;
            if (function_30bf0(&axis) == 0.0f)
            {
                function_11d000(&forward, &axis);
                if (function_30bf0(&axis) == 0.0f)
                    valid = false;
            }
            *direction = forward;
            if (valid)
            {
                real local_0 = (direction->k * axis.k + direction->j * axis.j + direction->i * axis.i) * 0.1339746117591858f;
                real local_1 = direction->j * axis.k - direction->k * axis.j;
                real local_2 = direction->k * axis.i - direction->i * axis.k;
                real local_3 = direction->i * axis.j - direction->j * axis.i;
                real i = local_0 * axis.i;
                i += direction->i * 0.8660253882408142f;
                i -= local_1 * 0.5f;
                direction->i = i;
                real j = direction->j * 0.8660253882408142f;
                j += local_0 * axis.j;
                j -= local_2 * 0.5f;
                direction->j = j;
                real k = direction->k * 0.8660253882408142f;
                k += local_0 * axis.k;
                k -= local_3 * 0.5f;
                direction->k = k;
            }
        }
        *arg_c9241c = *(real *)(actor + 0x7bc);
    }
    if (out_object)
        *out_object = object_index;
    return result;
}

#include "slot_handler.h"
#include <math.h>
struct s_collision_result_1697c0;
struct s_point_collision
{
    long type;
    real fraction;
    byte field_8[0x24 - 8];
    short material;
    byte field_26[0x5c - 0x26];
};
bool __stdcall function_1697c0(long flags, point3f const *point, vector3f const *vector,
    long ignore_object_index, long ignore_unit_index, s_collision_result_1697c0 *result);
extern vector3f *g_4687b0;

// @retail 0x1fe410
void function_1fe410(point3f *point, real distance)
{
    point3f start;
    start.x = g_4687b0->i * 1.5f + point->x;
    start.y = g_4687b0->j * 1.5f + point->y;
    start.z = g_4687b0->k * 1.5f + point->z;
    real angle = function_259d0(&g_4e7408->unknown0, __FILE__, __LINE__, -3.1415927410125732f, 3.1415927410125732f);
    vector3f radial;
    radial.i = (real)cos(angle);
    radial.j = (real)sin(angle);
    radial.k = 0.0f;
    point3f destination;
    destination.x = radial.i * distance + start.x;
    destination.y = radial.j * distance + start.y;
    destination.z = radial.k * distance + start.z;
    vector3f direction;
    vector3d_from_points3d(point, &start, &direction);
    s_point_collision collision;
    collision.material = NONE;
    if (function_1697c0(0x15808c2f, point, &direction, NONE, NONE,
        (s_collision_result_1697c0 *)&collision))
        start = *point;
    vector3d_from_points3d(&start, &destination, &direction);
    if (function_1697c0(0x5808c2f, &start, &direction, NONE, NONE,
        (s_collision_result_1697c0 *)&collision))
    {
        distance = collision.fraction * distance - 0.1f;
        if (distance < 0.0f) distance = 0.0f;
        destination.x = radial.i * distance + start.x;
        destination.y = radial.j * distance + start.y;
        destination.z = radial.k * distance + start.z;
    }
    *point = destination;
}
