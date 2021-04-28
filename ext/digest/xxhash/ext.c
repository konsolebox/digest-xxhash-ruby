/*
 * Copyright (c) 2017 konsolebox
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ruby.h>
#include <ruby/digest.h>
#include <stdint.h>

#define XXH_STATIC_LINKING_ONLY
#include "xxhash.h"
#include "utils.h"

#define _DIGEST_API_VERSION_IS_SUPPORTED(version) (version == 3)

#if !_DIGEST_API_VERSION_IS_SUPPORTED(RUBY_DIGEST_API_VERSION)
#	error Digest API version is not supported.
#endif

#define _XXH32_DIGEST_SIZE 4
#define _XXH64_DIGEST_SIZE 8

#define _XXH32_BLOCK_SIZE 4
#define _XXH64_BLOCK_SIZE 8

#define _XXH32_DEFAULT_SEED 0
#define _XXH64_DEFAULT_SEED 0

#if 0
#	define _DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#	define _DEBUG(...) (void)0;
#endif

static ID _id_digest;
static ID _id_finish;
static ID _id_hexdigest;
static ID _id_idigest;
static ID _id_ifinish;
static ID _id_new;
static ID _id_reset;
static ID _id_update;

static VALUE _Digest;
static VALUE _Digest_Class;
static VALUE _Digest_XXHash;
static VALUE _Digest_XXH32;
static VALUE _Digest_XXH64;

/*
 * Data types
 */

static const rb_data_type_t _xxh32_state_data_type = {
	"xxh32_state_data",
	{ 0, RUBY_TYPED_DEFAULT_FREE, 0, }, 0, 0,
	RUBY_TYPED_FREE_IMMEDIATELY|RUBY_TYPED_WB_PROTECTED
};

static const rb_data_type_t _xxh64_state_data_type = {
	"xxh64_state_data",
	{ 0, RUBY_TYPED_DEFAULT_FREE, 0, }, 0, 0,
	RUBY_TYPED_FREE_IMMEDIATELY|RUBY_TYPED_WB_PROTECTED
};

/*
 * Common functions
 */

static XXH32_state_t *_get_state_32(VALUE self)
{
	XXH32_state_t *state_p;
	TypedData_Get_Struct(self, XXH32_state_t, &_xxh32_state_data_type, state_p);
	return state_p;
}

static XXH64_state_t *_get_state_64(VALUE self)
{
	XXH64_state_t *state_p;
	TypedData_Get_Struct(self, XXH64_state_t, &_xxh64_state_data_type, state_p);
	return state_p;
}

static void _xxh32_reset(XXH32_state_t *state_p, uint32_t seed)
{
	if (XXH32_reset(state_p, seed) != XXH_OK)
		rb_raise(rb_eRuntimeError, "Failed to reset state.");
}

static void _xxh64_reset(XXH64_state_t *state_p, uint64_t seed)
{
	if (XXH64_reset(state_p, seed) != XXH_OK)
		rb_raise(rb_eRuntimeError, "Failed to reset state.");
}

static VALUE _encode_big_endian_32(uint32_t num)
{
	uint32_t temp;

	if (is_little_endian()) {
		temp = swap_uint32(num);
	} else {
		temp = num;
	}

	return rb_usascii_str_new((unsigned char *) &temp, sizeof(uint32_t));
}

static uint32_t _decode_big_endian_32_cstr(unsigned char *str)
{
	uint32_t temp = read32(str);

	if (is_little_endian()) {
		return swap_uint32(temp);
	} else {
		return temp;
	}
}

static uint32_t _decode_big_endian_32(VALUE str)
{
	return _decode_big_endian_32_cstr(RSTRING_PTR(str));
}

static VALUE _encode_big_endian_64(uint64_t num)
{
	uint64_t temp;

	if (is_little_endian()) {
		temp = swap_uint64(num);
	} else {
		temp = num;
	}

	return rb_usascii_str_new((unsigned char *) &temp, sizeof(uint64_t));
}

static uint64_t _decode_big_endian_64_cstr(unsigned char *str)
{
	uint64_t temp = read64(str);

	if (is_little_endian()) {
		return swap_uint64(temp);
	} else {
		return temp;
	}
}

static uint64_t _decode_big_endian_64(VALUE str)
{
	return _decode_big_endian_64_cstr(RSTRING_PTR(str));
}

static VALUE _hex_encode_str(VALUE str)
{
	int len = RSTRING_LEN(str);
	VALUE hex = rb_str_new(0, len * 2);
	hex_encode_str_implied(RSTRING_PTR(str), len, RSTRING_PTR(hex));
	return hex;
}

/*
 * Document-class: Digest::XXHash
 *
 * This is the base class of Digest::XXH32 and Digest::XXH64.
 */

static VALUE _Digest_XXHash_internal_allocate(VALUE klass)
{
	if (klass == _Digest_XXHash)
		rb_raise(rb_eRuntimeError, "Digest::XXHash is an incomplete class and cannot be instantiated.");

	rb_raise(rb_eNotImpError, "Allocator function not implemented.");
}

/*
 * call-seq:
 *     new -> instance
 *     new(seed) -> instance
 *
 * Returns a new hash instance.
 *
 * If seed is provided, the state is reset with its value, otherwise the default
 * seed (0) is used.
 *
 * +seed+ can be in the form of a string, a hex string, or a number.
 */
static VALUE _Digest_XXHash_initialize(int argc, VALUE* argv, VALUE self)
{
	if (argc > 0)
		rb_funcallv(self, _id_reset, argc, argv);

	return self;
}

/* :nodoc: */
static VALUE _Digest_XXHash_ifinish(VALUE self)
{
	rb_raise(rb_eNotImpError, "Method not implemented.");
}

static VALUE _do_digest(int argc, VALUE* argv, VALUE self, ID finish_method_id)
{
	VALUE str, seed;
	int argc2 = argc > 0 ? rb_scan_args(argc, argv, "02", &str, &seed) : 0;

	if (argc2 > 0) {
		if (TYPE(str) != T_STRING)
			rb_raise(rb_eTypeError, "Argument type not string.");

		if (argc2 > 1)
			rb_funcall(self, _id_reset, 1, seed);
		else
			rb_funcall(self, _id_reset, 0);

		rb_funcall(self, _id_update, 1, str);
	}

	VALUE result = rb_funcall(self, finish_method_id, 0);

	if (argc2 > 0)
		rb_funcall(self, _id_reset, 0);

	return result;
}

/*
 * call-seq:
 *     digest -> str
 *     digest(str, seed = 0) -> str
 *
 * Returns digest value in string form.
 *
 * If no argument is provided, the current digest value is returned, and no
 * reset happens.
 *
 * If a string argument is provided, the string's digest value is calculated
 * with +seed+, and is used as the return value.  The instance's state is reset
 * to default afterwards.
 *
 * Providing an argument means that previous calculations done with #update
 * would be discarded, so be careful with its use.
 *
 * +seed+ can be in the form of a string, a hex string, or a number.
 */
static VALUE _Digest_XXHash_digest(int argc, VALUE* argv, VALUE self)
{
	return _do_digest(argc, argv, self, _id_finish);
}

/*
 * call-seq:
 *     hexdigest -> hex_str
 *     hexdigest(str) -> hex_str
 *     hexdigest(str, seed) -> hex_str
 *
 * Same as #digest but returns the digest value in hex form.
 */
static VALUE _Digest_XXHash_hexdigest(int argc, VALUE* argv, VALUE self)
{
	VALUE result = _do_digest(argc, argv, self, _id_finish);
	return _hex_encode_str(result);
}

/*
 * call-seq:
 *     idigest -> num
 *     idigest(str) -> num
 *     idigest(str, seed) -> num
 *
 * Same as #digest but returns the digest value in numerical form.
 */
static VALUE _Digest_XXHash_idigest(int argc, VALUE* argv, VALUE self)
{
	return _do_digest(argc, argv, self, _id_ifinish);
}

/*
 * call-seq: idigest!
 *
 * Returns current digest value and resets state to default form.
 */
static VALUE _Digest_XXHash_idigest_bang(VALUE self)
{
	VALUE result = rb_funcall(self, _id_ifinish, 0);
	rb_funcall(self, _id_reset, 0);
	return result;
}

/*
 * call-seq: initialize_copy(orig) -> self
 *
 * This method is called when instances are cloned.  It is responsible for
 * replicating internal data.
 */
static VALUE _Digest_XXHash_initialize_copy(VALUE self, VALUE orig)
{
	rb_raise(rb_eNotImpError, "initialize_copy method not implemented.");
}

/*
 * call-seq: inspect -> str
 *
 * Returns a string in the form of <tt>#<class_name|hex_digest></tt>.
 */
static VALUE _Digest_XXHash_inspect(VALUE self)
{
	VALUE klass = rb_obj_class(self);
	VALUE klass_name = rb_class_name(klass);

	if (klass_name == Qnil)
		klass_name = rb_inspect(klass);

	VALUE hexdigest = rb_funcall(self, _id_hexdigest, 0);

	VALUE args[] = { klass_name, hexdigest };
	return rb_str_format(sizeof(args), args, rb_str_new_literal("#<%s|%s>"));
}

static VALUE _instantiate_and_digest(int argc, VALUE* argv, VALUE klass, ID digest_method_id)
{
	VALUE str, seed;
	int argc2 = rb_scan_args(argc, argv, "11", &str, &seed);

	if (TYPE(str) != T_STRING)
		rb_raise(rb_eTypeError, "Argument type not string.");

	VALUE instance = rb_funcall(klass, _id_new, 0);

	if (argc2 > 1)
		return rb_funcall(instance, digest_method_id, 2, str, seed);
	else
		return rb_funcall(instance, digest_method_id, 1, str);
}

/*
 * call-seq: Digest::XXHash::digest(str, seed = 0) -> str
 *
 * Returns the digest value of +str+ in string form with +seed+ as its seed.
 *
 * +seed+ can be in the form of a string, a hex string, or a number.
 *
 * If +seed+ is not provided, the default value would be 0.
 */
static VALUE _Digest_XXHash_singleton_digest(int argc, VALUE* argv, VALUE self)
{
	return _instantiate_and_digest(argc, argv, self, _id_digest);
}

/*
 * call-seq: Digest::XXHash::hexdigest -> hex_str
 *
 * Same as ::digest but returns the digest value in hex form.
 */
static VALUE _Digest_XXHash_singleton_hexdigest(int argc, VALUE* argv, VALUE self)
{
	return _instantiate_and_digest(argc, argv, self, _id_hexdigest);
}

/*
 * call-seq: Digest::XXHash::idigest -> num
 *
 * Same as ::digest but returns the digest value in numerical form.
 */
static VALUE _Digest_XXHash_singleton_idigest(int argc, VALUE* argv, VALUE self)
{
	return _instantiate_and_digest(argc, argv, self, _id_idigest);
}

/*
 * Document-class: Digest::XXH32
 *
 * This class implements XXH32.
 */

static VALUE _Digest_XXH32_internal_allocate(VALUE klass)
{
	XXH32_state_t *state_p;
	VALUE obj = TypedData_Make_Struct(klass, XXH32_state_t, &_xxh32_state_data_type, state_p);
	_xxh32_reset(state_p, 0);
	return obj;
}

/*
 * call-seq: update(str) -> self
 *
 * Updates current digest value with string.
 */
static VALUE _Digest_XXH32_update(VALUE self, VALUE str)
{
	if (XXH32_update(_get_state_32(self), RSTRING_PTR(str), RSTRING_LEN(str)) != XXH_OK)
		rb_raise(rb_eRuntimeError, "Failed to update state.");

	return self;
}

/* :nodoc: */
static VALUE _Digest_XXH32_finish(VALUE self)
{
	uint32_t result = XXH32_digest(_get_state_32(self));
	return _encode_big_endian_32(result);
}

/* :nodoc: */
static VALUE _Digest_XXH32_ifinish(VALUE self)
{
	uint32_t result = XXH32_digest(_get_state_32(self));
	return ULONG2NUM(result);
}

/*
 * call-seq: reset(seed = 0) -> self
 *
 * Resets state to initial form with seed.
 *
 * This would discard previous calculations with #update.
 *
 * +seed+ can be in the form of a string, a hex string, or a number.
 *
 * If +seed+ is not provided, the default value would be 0.
 */
static VALUE _Digest_XXH32_reset(int argc, VALUE* argv, VALUE self)
{
	VALUE seed;

	if (argc > 0 && rb_scan_args(argc, argv, "01", &seed) > 0) {
		switch (TYPE(seed)) {
		case T_STRING:
			{
				int len = RSTRING_LEN(seed);
				uint32_t decoded_seed;

				if (len == (sizeof(uint32_t) * 2)) {
					unsigned char hex_decoded_seed[sizeof(uint32_t)];

					if (! hex_decode_str_implied(RSTRING_PTR(seed), sizeof(uint32_t) * 2, hex_decoded_seed))
						rb_raise(rb_eArgError, "Invalid hex string seed: %s\n", StringValueCStr(seed));

					decoded_seed = _decode_big_endian_32_cstr(hex_decoded_seed);
				} else if (len == sizeof(uint32_t)) {
					decoded_seed = _decode_big_endian_32_cstr(RSTRING_PTR(seed));
				} else {
					rb_raise(rb_eArgError, "Invalid seed length.  Expecting an 8-character hex string or a 4-byte string.");
				}

				_xxh32_reset(_get_state_32(self), decoded_seed);
			}

			break;
		case T_FIXNUM:
			_xxh32_reset(_get_state_32(self), FIX2UINT(seed));
			break;
		case T_BIGNUM:
			_xxh32_reset(_get_state_32(self), NUM2UINT(seed));
			break;
		default:
			rb_raise(rb_eArgError, "Invalid argument type for seed.  Expecting a string or number.");
		}
	} else {
		_xxh32_reset(_get_state_32(self), _XXH32_DEFAULT_SEED);
	}

	return self;
}

/*
 * call-seq: initialize_copy(orig) -> self
 *
 * This method is called when instances are cloned.  It is responsible for
 * replicating internal data.
 */
static VALUE _Digest_XXH32_initialize_copy(VALUE self, VALUE orig)
{
	XXH32_copyState(_get_state_32(self), _get_state_32(orig));
	return self;
}

/*
 * call-seq: digest_length -> int
 *
 * Returns 4.
 */
static VALUE _Digest_XXH32_digest_length(VALUE self)
{
	return INT2FIX(_XXH32_DIGEST_SIZE);
}

/*
 * call-seq: block_length  -> int
 *
 * Returns 4.
 */
static VALUE _Digest_XXH32_block_length(VALUE self)
{
	return INT2FIX(_XXH32_BLOCK_SIZE);
}

/*
 * call-seq: digest_length -> int
 *
 * Returns 4.
 */
static VALUE _Digest_XXH32_singleton_digest_length(VALUE self)
{
	return INT2FIX(_XXH32_DIGEST_SIZE);
}

/*
 * call-seq: block_length -> int
 *
 * Returns 4.
 */
static VALUE _Digest_XXH32_singleton_block_length(VALUE self)
{
	return INT2FIX(_XXH32_BLOCK_SIZE);
}

/*
 * Document-class: Digest::XXH64
 *
 * This class implements XXH64.
 */

static VALUE _Digest_XXH64_internal_allocate(VALUE klass)
{
	XXH64_state_t *state_p;
	VALUE obj = TypedData_Make_Struct(klass, XXH64_state_t, &_xxh64_state_data_type, state_p);
	_xxh64_reset(state_p, 0);
	return obj;
}

/*
 * call-seq: update(str) -> self
 *
 * Updates current digest value with string.
 */
static VALUE _Digest_XXH64_update(VALUE self, VALUE str)
{
	if (XXH64_update(_get_state_64(self), RSTRING_PTR(str), RSTRING_LEN(str)) != XXH_OK)
		rb_raise(rb_eRuntimeError, "Failed to update state.");

	return self;
}

/* :nodoc: */
static VALUE _Digest_XXH64_finish(VALUE self)
{
	uint64_t result = XXH64_digest(_get_state_64(self));
	return _encode_big_endian_64(result);
}

/* :nodoc: */
static VALUE _Digest_XXH64_ifinish(VALUE self)
{
	uint64_t result = XXH64_digest(_get_state_64(self));
	return ULL2NUM(result);
}

/*
 * call-seq: reset(seed = 0) -> self
 *
 * Resets state to initial form with seed.
 *
 * This would discard previous calculations with #update.
 *
 * +seed+ can be in the form of a string, a hex string, or a number.
 *
 * If +seed+ is not provided, the default value would be 0.
 */
static VALUE _Digest_XXH64_reset(int argc, VALUE* argv, VALUE self)
{
	VALUE seed;

	if (rb_scan_args(argc, argv, "01", &seed) > 0) {
		switch (TYPE(seed)) {
		case T_STRING:
			{
				int len = RSTRING_LEN(seed);
				uint64_t decoded_seed;

				if (len == (sizeof(uint64_t) * 2)) {
					unsigned char hex_decoded_seed[sizeof(uint64_t)];

					if (! hex_decode_str_implied(RSTRING_PTR(seed), sizeof(uint64_t) * 2, hex_decoded_seed))
						rb_raise(rb_eArgError, "Invalid hex string seed: %s\n", StringValueCStr(seed));

					decoded_seed = _decode_big_endian_64_cstr(hex_decoded_seed);
				} else if (len == sizeof(uint64_t)) {
					decoded_seed = _decode_big_endian_64_cstr(RSTRING_PTR(seed));
				} else {
					rb_raise(rb_eArgError, "Invalid seed length.  Expecting a 16-character hex string or an 8-byte string.");
				}

				_xxh64_reset(_get_state_64(self), decoded_seed);
			}

			break;
		case T_FIXNUM:
			_xxh64_reset(_get_state_64(self), FIX2UINT(seed));
			break;
		case T_BIGNUM:
			_xxh64_reset(_get_state_64(self), NUM2ULL(seed));
			break;
		default:
			rb_raise(rb_eArgError, "Invalid argument type for seed.  Expecting a string or number.");
		}
	} else {
		_xxh64_reset(_get_state_64(self), _XXH64_DEFAULT_SEED);
	}

	return self;
}

/*
 * call-seq: initialize_copy(orig) -> self
 *
 * This method is called when instances are cloned.  It is responsible for
 * replicating internal data.
 */
static VALUE _Digest_XXH64_initialize_copy(VALUE self, VALUE orig)
{
	XXH64_copyState(_get_state_64(self), _get_state_64(orig));
	return self;
}

/*
 * call-seq: digest_length -> int
 *
 * Returns 8.
 */
static VALUE _Digest_XXH64_digest_length(VALUE self)
{
	return INT2FIX(_XXH64_DIGEST_SIZE);
}

/*
 * call-seq: block_length -> int
 *
 * Returns 8.
 */
static VALUE _Digest_XXH64_block_length(VALUE self)
{
	return INT2FIX(_XXH64_BLOCK_SIZE);
}

/*
 * call-seq: digest_length -> int
 *
 * Returns 8.
 */
static VALUE _Digest_XXH64_singleton_digest_length(VALUE self)
{
	return INT2FIX(_XXH64_DIGEST_SIZE);
}

/*
 * call-seq: block_length -> int
 *
 * Returns 8.
 */
static VALUE _Digest_XXH64_singleton_block_length(VALUE self)
{
	return INT2FIX(_XXH64_BLOCK_SIZE);
}

/*
 * Initialization
 */

void Init_xxhash()
{
	#define DEFINE_ID(x) _id_##x = rb_intern_const(#x);

	DEFINE_ID(digest)
	DEFINE_ID(finish)
	DEFINE_ID(hexdigest)
	DEFINE_ID(idigest)
	DEFINE_ID(ifinish)
	DEFINE_ID(new)
	DEFINE_ID(reset)
	DEFINE_ID(update)

	rb_require("digest");
	_Digest = rb_path2class("Digest");
	_Digest_Class = rb_path2class("Digest::Class");

	#if 0
	/* Tell RDoc about Digest and Digest::Class since it doesn't parse rb_path2class. */
	_Digest = rb_define_module("Digest");
	_Digest_Class = rb_define_class_under(_Digest, "Class", rb_cObject);
	#endif

	/*
	 * Document-class: Digest::XXHash
	 */

	_Digest_XXHash = rb_define_class_under(_Digest, "XXHash", _Digest_Class);

	rb_define_method(_Digest_XXHash, "digest", _Digest_XXHash_digest, -1);
	rb_define_method(_Digest_XXHash, "hexdigest", _Digest_XXHash_hexdigest, -1);
	rb_define_method(_Digest_XXHash, "idigest", _Digest_XXHash_idigest, -1);
	rb_define_method(_Digest_XXHash, "idigest!", _Digest_XXHash_idigest_bang, 0);
	rb_define_method(_Digest_XXHash, "initialize", _Digest_XXHash_initialize, -1);
	rb_define_method(_Digest_XXHash, "inspect", _Digest_XXHash_inspect, 0);
	rb_define_method(_Digest_XXHash, "initialize_copy", _Digest_XXHash_initialize_copy, 1);

	rb_define_protected_method(_Digest_XXHash, "ifinish", _Digest_XXHash_ifinish, 0);

	rb_define_singleton_method(_Digest_XXHash, "digest", _Digest_XXHash_singleton_digest, -1);
	rb_define_singleton_method(_Digest_XXHash, "hexdigest", _Digest_XXHash_singleton_hexdigest, -1);
	rb_define_singleton_method(_Digest_XXHash, "idigest", _Digest_XXHash_singleton_idigest, -1);

	rb_define_alloc_func(_Digest_XXHash, _Digest_XXHash_internal_allocate);

	/*
	 * Document-class: Digest::XXH32
	 */

	_Digest_XXH32 = rb_define_class_under(_Digest, "XXH32", _Digest_XXHash);

	rb_define_alloc_func(_Digest_XXH32, _Digest_XXH32_internal_allocate);

	rb_define_private_method(_Digest_XXH32, "finish", _Digest_XXH32_finish, 0);
	rb_define_private_method(_Digest_XXH32, "ifinish", _Digest_XXH32_ifinish, 0);

	rb_define_method(_Digest_XXH32, "update", _Digest_XXH32_update, 1);
	rb_define_method(_Digest_XXH32, "reset", _Digest_XXH32_reset, -1);
	rb_define_method(_Digest_XXH32, "digest_length", _Digest_XXH32_digest_length, 0);
	rb_define_method(_Digest_XXH32, "block_length", _Digest_XXH32_block_length, 0);
	rb_define_method(_Digest_XXH32, "initialize_copy", _Digest_XXH32_initialize_copy, 1);

	rb_define_singleton_method(_Digest_XXH32, "digest_length", _Digest_XXH32_singleton_digest_length, 0);
	rb_define_singleton_method(_Digest_XXH32, "block_length", _Digest_XXH32_singleton_block_length, 0);

	/*
	 * Document-class: Digest::XXH64
	 */

	_Digest_XXH64 = rb_define_class_under(_Digest, "XXH64", _Digest_XXHash);

	rb_define_alloc_func(_Digest_XXH64, _Digest_XXH64_internal_allocate);

	rb_define_private_method(_Digest_XXH64, "finish", _Digest_XXH64_finish, 0);
	rb_define_private_method(_Digest_XXH64, "ifinish", _Digest_XXH64_ifinish, 0);

	rb_define_method(_Digest_XXH64, "update", _Digest_XXH64_update, 1);
	rb_define_method(_Digest_XXH64, "reset", _Digest_XXH64_reset, -1);
	rb_define_method(_Digest_XXH64, "digest_length", _Digest_XXH64_digest_length, 0);
	rb_define_method(_Digest_XXH64, "block_length", _Digest_XXH64_block_length, 0);
	rb_define_method(_Digest_XXH64, "initialize_copy", _Digest_XXH64_initialize_copy, 1);

	rb_define_singleton_method(_Digest_XXH64, "digest_length", _Digest_XXH64_singleton_digest_length, 0);
	rb_define_singleton_method(_Digest_XXH64, "block_length", _Digest_XXH64_singleton_block_length, 0);
}
