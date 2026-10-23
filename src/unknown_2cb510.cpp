// @flags /O2 /Gr
/* UNKNOWN_2CB510.CPP: one-call compression and decompression of a block (the
   SDK's zlib 1.1.4: compress2 and uncompress given the caller's allocator,
   deflateEnd and inflateInit2_). Retail builds them with register
   conventions, so 0x1995a0 and 0x199740 only build like retail with these
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
	char const *message;
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

/* the decompressor's state (0x18 bytes) */
struct s_zlib_inflate_state
{
	long unknown00;
	byte unknown04[8];
	long no_header;
	long window_bits;
	void *blocks;
};
typedef char assert_s_zlib_inflate_state_size[(sizeof(s_zlib_inflate_state) == 0x18) ? 1 : -1];

typedef unsigned long (__stdcall *block_checksum)(unsigned long, byte const *, unsigned long);

enum
{
	k_zlib_ok = 0,
	k_zlib_stream_end = 1,
	k_zlib_stream_error = -2,
	k_zlib_data_error = -3,
	k_zlib_memory_error = -4,
	k_zlib_buffer_error = -5,
	k_zlib_version_error = -6,
	k_zlib_finish = 4,
	k_zlib_deflated = 8,
	k_zlib_window_bits = 15,
	k_zlib_minimum_window_bits = 8,
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
long function_2cc8d0(s_zlib_stream *stream, long window_bits, char const *version, long stream_size);
long __stdcall function_2cc9d0(s_zlib_stream *stream, long flush);
long __stdcall function_2cc890(s_zlib_stream *stream);
long __stdcall function_2cc850(s_zlib_stream *stream);
void *__stdcall function_2ce950(s_zlib_stream *stream, block_checksum checksum, unsigned long window_size);
void *__stdcall function_2ce780(void *opaque, long count, long size);
void __stdcall function_2ce7a0(void *opaque, void *address);
unsigned long __stdcall function_2ce7b0(unsigned long checksum, byte const *buffer, unsigned long size);

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

/* decompresses source into destination; destination_size holds the room on
   entry and the decompressed size on return. Its stream calls are 0x2cc8d0
   (setting up the decompressor), 0x2cc9d0 (decompressing) and 0x2cc890
   (releasing it). */
// @retail 0x2cb5b0
long function_2cb5b0(byte *destination, long *destination_size, byte const *source, long source_size, block_allocate allocate, block_free release, void *opaque)
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

	error = function_2cc8d0(&stream, k_zlib_window_bits, "1.1.4", sizeof(stream));
	if (error != k_zlib_ok)
		return error;

	error = function_2cc9d0(&stream, k_zlib_finish);
	if (error != k_zlib_stream_end)
	{
		function_2cc890(&stream);
		return error == k_zlib_ok ? k_zlib_buffer_error : error;
	}
	*destination_size = stream.total_output;

	return function_2cc890(&stream);
}

static inline s_zlib_inflate_state *zlib_inflate_state(s_zlib_stream *stream)
{
	return (s_zlib_inflate_state *)stream->state;
}

/* sets up a decompressor: the default allocator where the stream has none,
   its state, and the block decoder for a window of 2^window_bits bytes (a
   negative count means raw data with no header or checksum) */
// @retail 0x2cc8d0
long function_2cc8d0(s_zlib_stream *stream, long window_bits, char const *version, long stream_size)
{
	if (version == NULL || version[0] != '1' || stream_size != sizeof(s_zlib_stream))
		return k_zlib_version_error;
	if (stream == NULL)
		return k_zlib_stream_error;

	stream->message = NULL;
	if (stream->allocate == NULL)
	{
		stream->allocate = function_2ce780;
		stream->opaque = NULL;
	}
	if (stream->release == NULL)
		stream->release = function_2ce7a0;

	stream->state = stream->allocate(stream->opaque, 1, sizeof(s_zlib_inflate_state));
	if (stream->state == NULL)
		return k_zlib_memory_error;
	zlib_inflate_state(stream)->blocks = NULL;

	zlib_inflate_state(stream)->no_header = 0;
	if (window_bits < 0)
	{
		window_bits = -window_bits;
		zlib_inflate_state(stream)->no_header = 1;
	}
	if (window_bits < k_zlib_minimum_window_bits || window_bits > k_zlib_window_bits)
	{
		function_2cc890(stream);
		return k_zlib_stream_error;
	}
	zlib_inflate_state(stream)->window_bits = window_bits;

	zlib_inflate_state(stream)->blocks = function_2ce950(stream, zlib_inflate_state(stream)->no_header ? NULL : function_2ce7b0, 1 << window_bits);
	if (zlib_inflate_state(stream)->blocks == NULL)
	{
		function_2cc890(stream);
		return k_zlib_memory_error;
	}

	function_2cc850(stream);
	return k_zlib_ok;
}
