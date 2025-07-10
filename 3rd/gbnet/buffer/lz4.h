#pragma once
namespace gb {

//****************************
// Simple Functions
//****************************

int LZ4_compress   (const char* source, char* dest, int isize);
int LZ4_uncompress (const char* source, char* dest, int osize);

/*
LZ4_compress() :
	isize  : is the input size. Max supported value is ~1.9GB
	return : the number of bytes written in buffer dest
			 or 0 if the compression fails (if LZ4_COMPRESSMIN is set)
	note : destination buffer must be already allocated.
		destination buffer must be sized to handle worst cases situations (input data not compressible)
		worst case size evaluation is provided by function LZ4_compressBound()

LZ4_uncompress() :
	osize  : is the output size, therefore the original size
	return : the number of bytes read in the source buffer
			 If the source stream is malformed, the function will stop decoding and return a negative result, indicating the byte position of the faulty instruction
			 This function never writes beyond dest + osize, and is therefore protected against malicious data packets
	note : destination buffer must be already allocated
*/


//****************************
// Advanced Functions
//****************************

int LZ4_compressBound(int isize);

/*
LZ4_compressBound() :
	Provides the maximum size that LZ4 may output in a "worst case" scenario (input data not compressible)
	primarily useful for memory allocation of output buffer.

	isize  : is the input size. Max supported value is ~1.9GB
	return : maximum output size in a "worst case" scenario
	note : this function is limited by "int" range (2^31-1)
*/


int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize);

/*
LZ4_uncompress_unknownOutputSize() :
	isize  : is the input size, therefore the compressed size
	maxOutputSize : is the size of the destination buffer (which must be already allocated)
	return : the number of bytes decoded in the destination buffer (necessarily <= maxOutputSize)
			 If the source stream is malformed, the function will stop decoding and return a negative result, indicating the byte position of the faulty instruction
			 This function never writes beyond dest + maxOutputSize, and is therefore protected against malicious data packets
	note   : Destination buffer must be already allocated.
	         This version is slightly slower than LZ4_uncompress()
*/


int LZ4_compressCtx(void** ctx, const char* source,  char* dest, int isize);
int LZ4_compress64kCtx(void** ctx, const char* source,  char* dest, int isize);

/*
LZ4_compressCtx() :
	This function explicitly handles the CTX memory structure.
	It avoids allocating/deallocating memory between each call, improving performance when malloc is heavily invoked.
	This function is only useful when memory is allocated into the heap (HASH_LOG value beyond STACK_LIMIT)
	Performance difference will be noticeable only when repetitively calling the compression function over many small segments.
	Note : by default, memory is allocated into the stack, therefore "malloc" is not invoked.
LZ4_compress64kCtx() :
	Same as LZ4_compressCtx(), but specific to small inputs (<64KB).
	isize *Must* be <64KB, otherwise the output will be corrupted.

	On first call : provide a *ctx=NULL; It will be automatically allocated.
	On next calls : reuse the same ctx pointer.
	Use different pointers for different threads when doing multi-threading.

*/

} // namespace gb

