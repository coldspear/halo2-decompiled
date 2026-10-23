// @flags /O2 /Gr
/* UNKNOWN_2CB510.CPP: one-call compression of a block (the SDK's zlib 1.1.4,
   compress2 given the caller's allocator, and deflateEnd). Retail builds them
   with register conventions, so 0x1995a0 only builds like retail with these
   bodies in the image. */

#include <stddef.h>
#include "unknown_11c920.h"

typedef void *(__stdcall *block_allocate)(void *, long, long);
typedef void (__stdcall *block_free)(void *, void *);

/* the stream state the zlib calls share (0x38 bytes) */
struct s_zlib_stream
{
	byte const *input;
	unsigned long input_size;
	unsigned long unknown08;
	byte *output;
	unsigned long output_size;
	unsigned long total_output;
	char const *unknown18;
	void *state;
	block_allocate allocate;
	block_free release;
	void *opaque;
	long unknown2c;
	unsigned long unknown30;
	unsigned long unknown34;
};
typedef char assert_s_zlib_stream_size[(sizeof(s_zlib_stream) == 0x38) ? 1 : -1];

/* the parts of the compressor's state that releasing it reads */
struct s_zlib_deflate_state
{
	s_zlib_stream *unknown00;
	long status;
	byte *pending_buffer;
	byte unknown0c[0x30 - 0xc];
	byte *window;
	unsigned long unknown34;
	unsigned short *previous;
	unsigned short *head;
};
typedef char assert_s_zlib_deflate_state_head[(offsetof(s_zlib_deflate_state, head) == 0x3c) ? 1 : -1];

enum
{
	k_zlib_ok = 0,
	k_zlib_stream_end = 1,
	k_zlib_stream_error = -2,
	k_zlib_data_error = -3,
	k_zlib_buffer_error = -5,
	k_zlib_finish = 4,
	k_zlib_deflated = 8,
	k_zlib_window_bits = 15,
	k_zlib_memory_level = 8,
	k_zlib_default_strategy = 0,
};

enum
{
	k_deflate_init_state = 0x2a,
	k_deflate_busy_state = 0x71,
	k_deflate_finish_state = 0x29a,
};

long __stdcall function_2cb640(s_zlib_stream *stream, long level, long method, long window_bits, long memory_level, long strategy, char const *version, long stream_size);
long __stdcall function_2cb960(s_zlib_stream *stream, long flush);
long function_2cbbf0(s_zlib_stream *stream);

/* compresses source into destination; destination_size holds the room on entry
   and the compressed size on return. Its stream calls are 0x2cb640 (setting up
   the compressor), 0x2cb960 (compressing) and 0x2cbbf0 (releasing it). */
// @retail 0x2cb510
long function_2cb510(byte *destination, long *destination_size, byte const *source, long source_size, long level, block_allocate allocate, block_free release, void *opaque)
{
	s_zlib_stream stream;
	long error;

	stream.input = source;
	stream.input_size = source_size;
	stream.output = destination;
	stream.output_size = *destination_size;
	stream.allocate = allocate;
	stream.release = release;
	stream.opaque = opaque;

	error = function_2cb640(&stream, level, k_zlib_deflated, k_zlib_window_bits, k_zlib_memory_level, k_zlib_default_strategy, "1.1.4", sizeof(stream));
	if (error != k_zlib_ok)
		return error;

	error = function_2cb960(&stream, k_zlib_finish);
	if (error != k_zlib_stream_end)
	{
		function_2cbbf0(&stream);
		return error == k_zlib_ok ? k_zlib_buffer_error : error;
	}
	*destination_size = stream.total_output;

	return function_2cbbf0(&stream);
}

static inline s_zlib_deflate_state *zlib_deflate_state(s_zlib_stream *stream)
{
	return (s_zlib_deflate_state *)stream->state;
}

/* releases a compressor's buffers and state; fails on a state that isn't one */
// @retail 0x2cbbf0
long function_2cbbf0(s_zlib_stream *stream)
{
	long status;

	if (stream == NULL || stream->state == NULL)
		return k_zlib_stream_error;

	status = zlib_deflate_state(stream)->status;
	if (status != k_deflate_init_state && status != k_deflate_busy_state && status != k_deflate_finish_state)
		return k_zlib_stream_error;

	if (zlib_deflate_state(stream)->pending_buffer)
		stream->release(stream->opaque, zlib_deflate_state(stream)->pending_buffer);
	if (zlib_deflate_state(stream)->head)
		stream->release(stream->opaque, zlib_deflate_state(stream)->head);
	if (zlib_deflate_state(stream)->previous)
		stream->release(stream->opaque, zlib_deflate_state(stream)->previous);
	if (zlib_deflate_state(stream)->window)
		stream->release(stream->opaque, zlib_deflate_state(stream)->window);
	stream->release(stream->opaque, stream->state);
	stream->state = NULL;

	return status == k_deflate_busy_state ? k_zlib_data_error : k_zlib_ok;
}
