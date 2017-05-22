/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>

#include <ctype.h>
#include <cstring>

#include "sharedbuffer.h"
#include "platformdefines.h"
#include "hwcdefs_internal.h"

namespace hwcomposer {

static const char32_t kByteMask = 0x000000BF;
static const char32_t kByteMark = 0x00000080;

// Surrogates aren't valid for UTF-32 characters, so define some
// constants that will let us screen them out.
static const char32_t kUnicodeSurrogateHighStart = 0x0000D800;
// Unused, here for completeness:
// static const char32_t kUnicodeSurrogateHighEnd = 0x0000DBFF;
// static const char32_t kUnicodeSurrogateLowStart = 0x0000DC00;
static const char32_t kUnicodeSurrogateLowEnd = 0x0000DFFF;
static const char32_t kUnicodeSurrogateStart = kUnicodeSurrogateHighStart;
static const char32_t kUnicodeSurrogateEnd = kUnicodeSurrogateLowEnd;
static const char32_t kUnicodeMaxCodepoint = 0x0010FFFF;

// Mask used to set appropriate bits in first byte of UTF-8 sequence,
// indexed by number of bytes in the sequence.
// 0xxxxxxx
// -> (00-7f) 7bit. Bit mask for the first byte is 0x00000000
// 110yyyyx 10xxxxxx
// -> (c0-df)(80-bf) 11bit. Bit mask is 0x000000C0
// 1110yyyy 10yxxxxx 10xxxxxx
// -> (e0-ef)(80-bf)(80-bf) 16bit. Bit mask is 0x000000E0
// 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
// -> (f0-f7)(80-bf)(80-bf)(80-bf) 21bit. Bit mask is 0x000000F0
static const char32_t kFirstByteMark[] = {0x00000000, 0x00000000, 0x000000C0,
                                          0x000000E0, 0x000000F0};

#define OS_PATH_SEPARATOR '\\'

// --------------------------------------------------------------------------
// UTF-32
// --------------------------------------------------------------------------

/**
 * Return number of UTF-8 bytes required for the character. If the character
 * is invalid, return size of 0.
 */
static inline size_t utf32_codepoint_utf8_length(char32_t srcChar) {
  // Figure out how many bytes the result will require.
  if (srcChar < 0x00000080) {
    return 1;
  } else if (srcChar < 0x00000800) {
    return 2;
  } else if (srcChar < 0x00010000) {
    if ((srcChar < kUnicodeSurrogateStart) ||
        (srcChar > kUnicodeSurrogateEnd)) {
      return 3;
    } else {
      // Surrogates are invalid UTF-32 characters.
      return 0;
    }
  }
  // Max code point for Unicode is 0x0010FFFF.
  else if (srcChar <= kUnicodeMaxCodepoint) {
    return 4;
  } else {
    // Invalid UTF-32 character.
    return 0;
  }
}

// Write out the source character to <dstP>.

static inline void utf32_codepoint_to_utf8(uint8_t* dstP, char32_t srcChar,
                                           size_t bytes) {
  dstP += bytes;
  switch (bytes) { /* note: everything falls through. */
    case 4:
      *--dstP = (uint8_t)((srcChar | kByteMark) & kByteMask);
      srcChar >>= 6;
    case 3:
      *--dstP = (uint8_t)((srcChar | kByteMark) & kByteMask);
      srcChar >>= 6;
    case 2:
      *--dstP = (uint8_t)((srcChar | kByteMark) & kByteMask);
      srcChar >>= 6;
    case 1:
      *--dstP = (uint8_t)(srcChar | kFirstByteMark[bytes]);
  }
}

void utf16_to_utf8(const char16_t* src, size_t src_len, char* dst,
                   size_t dst_len) {
  if (src == NULL || src_len == 0 || dst == NULL) {
    return;
  }

  const char16_t* cur_utf16 = src;
  const char16_t* const end_utf16 = src + src_len;
  char* cur = dst;
  while (cur_utf16 < end_utf16) {
    char32_t utf32;
    // surrogate pairs
    if ((*cur_utf16 & 0xFC00) == 0xD800 && (cur_utf16 + 1) < end_utf16 &&
        (*(cur_utf16 + 1) & 0xFC00) == 0xDC00) {
      utf32 = (*cur_utf16++ - 0xD800) << 10;
      utf32 |= *cur_utf16++ - 0xDC00;
      utf32 += 0x10000;
    } else {
      utf32 = (char32_t)*cur_utf16++;
    }
    const size_t len = utf32_codepoint_utf8_length(utf32);
    LOG_ALWAYS_FATAL_IF(dst_len < len, "%zu < %zu", dst_len, len);
    utf32_codepoint_to_utf8((uint8_t*)cur, utf32, len);
    cur += len;
    dst_len -= len;
  }
  LOG_ALWAYS_FATAL_IF(dst_len < 1, "%zu < 1", dst_len);
  *cur = '\0';
}

ssize_t utf16_to_utf8_length(const char16_t* src, size_t src_len) {
  if (src == NULL || src_len == 0) {
    return -1;
  }

  size_t ret = 0;
  const char16_t* const end = src + src_len;
  while (src < end) {
    if ((*src & 0xFC00) == 0xD800 && (src + 1) < end &&
        (*(src + 1) & 0xFC00) == 0xDC00) {
      // surrogate pairs are always 4 bytes.
      ret += 4;
      src += 2;
    } else {
      ret += utf32_codepoint_utf8_length((char32_t)*src++);
    }
  }
  return ret;
}

// Separator used by resource paths. This is not platform dependent contrary
// to OS_PATH_SEPARATOR.
#define RES_PATH_SEPARATOR '/'

static SharedBuffer* gEmptyStringBuf = NULL;
static char* gEmptyString = NULL;

void initialize_string8();

static inline char* getEmptyString() {
  initialize_string8();
  gEmptyStringBuf->acquire();
  return gEmptyString;
}

void initialize_string8() {
  if (gEmptyStringBuf != NULL)
    return;

  SharedBuffer* buf = SharedBuffer::alloc(1);
  char* str = (char*)buf->data();
  *str = 0;
  gEmptyStringBuf = buf;
  gEmptyString = str;
}

void terminate_string8() {
  SharedBuffer::bufferFromData(gEmptyString)->release();
  gEmptyStringBuf = NULL;
  gEmptyString = NULL;
}

// ---------------------------------------------------------------------------

static char* allocFromUTF8(const char* in, size_t len) {
  if (len > 0) {
    if (len == SIZE_MAX) {
      return NULL;
    }
    SharedBuffer* buf = SharedBuffer::alloc(len + 1);
    HWCASSERT(buf, "Unable to allocate shared buffer");
    if (buf) {
      char* str = (char*)buf->data();
      memcpy(str, in, len);
      str[len] = 0;
      return str;
    }
    return NULL;
  }

  return getEmptyString();
}

static char* allocFromUTF16(const char16_t* in, size_t len) {
  if (len == 0)
    return getEmptyString();

  // Allow for closing '\0'
  const ssize_t resultStrLen = utf16_to_utf8_length(in, len) + 1;
  if (resultStrLen < 1) {
    return getEmptyString();
  }

  SharedBuffer* buf = SharedBuffer::alloc(resultStrLen);
  HWCASSERT(buf, "Unable to allocate shared buffer");
  if (!buf) {
    return getEmptyString();
  }

  char* resultStr = (char*)buf->data();
  utf16_to_utf8(in, len, resultStr, resultStrLen);
  return resultStr;
}

ssize_t utf32_to_utf8_length(const char32_t* src, size_t src_len) {
  if (src == NULL || src_len == 0) {
    return -1;
  }

  size_t ret = 0;
  const char32_t* end = src + src_len;
  while (src < end) {
    ret += utf32_codepoint_utf8_length(*src++);
  }
  return ret;
}

void utf32_to_utf8(const char32_t* src, size_t src_len, char* dst,
                   size_t dst_len) {
  if (src == NULL || src_len == 0 || dst == NULL) {
    return;
  }

  const char32_t* cur_utf32 = src;
  const char32_t* end_utf32 = src + src_len;
  char* cur = dst;
  while (cur_utf32 < end_utf32) {
    size_t len = utf32_codepoint_utf8_length(*cur_utf32);
    LOG_ALWAYS_FATAL_IF(dst_len < len, "%zu < %zu", dst_len, len);
    utf32_codepoint_to_utf8((uint8_t*)cur, *cur_utf32++, len);
    cur += len;
    dst_len -= len;
  }
  LOG_ALWAYS_FATAL_IF(dst_len < 1, "dst_len < 1: %zu < 1", dst_len);
  *cur = '\0';
}

static char* allocFromUTF32(const char32_t* in, size_t len) {
  if (len == 0) {
    return getEmptyString();
  }

  const ssize_t resultStrLen = utf32_to_utf8_length(in, len) + 1;
  if (resultStrLen < 1) {
    return getEmptyString();
  }

  SharedBuffer* buf = SharedBuffer::alloc(resultStrLen);
  HWCASSERT(buf, "Unable to allocate shared buffer");
  if (!buf) {
    return getEmptyString();
  }

  char* resultStr = (char*)buf->data();
  utf32_to_utf8(in, len, resultStr, resultStrLen);

  return resultStr;
}

size_t strlen16(const char16_t* s) {
  const char16_t* ss = s;
  while (*ss)
    ss++;
  return ss - s;
}

size_t strlen32(const char32_t* s) {
  const char32_t* ss = s;
  while (*ss)
    ss++;
  return ss - s;
}

// ---------------------------------------------------------------------------

String8::String8() : mString(getEmptyString()) {
}

String8::String8(StaticLinkage) : mString(0) {
  // this constructor is used when we can't rely on the static-initializers
  // having run. In this case we always allocate an empty string. It's less
  // efficient than using getEmptyString(), but we assume it's uncommon.

  char* data = static_cast<char*>(SharedBuffer::alloc(sizeof(char))->data());
  data[0] = 0;
  mString = data;
}

String8::String8(const String8& o) : mString(o.mString) {
  SharedBuffer::bufferFromData(mString)->acquire();
}

String8::String8(const char* o) : mString(allocFromUTF8(o, strlen(o))) {
  if (mString == NULL) {
    mString = getEmptyString();
  }
}

String8::String8(const char* o, size_t len) : mString(allocFromUTF8(o, len)) {
  if (mString == NULL) {
    mString = getEmptyString();
  }
}

String8::String8(const char16_t* o) : mString(allocFromUTF16(o, strlen16(o))) {
}

String8::String8(const char16_t* o, size_t len)
    : mString(allocFromUTF16(o, len)) {
}

String8::String8(const char32_t* o) : mString(allocFromUTF32(o, strlen32(o))) {
}

String8::String8(const char32_t* o, size_t len)
    : mString(allocFromUTF32(o, len)) {
}

String8::~String8() {
  SharedBuffer::bufferFromData(mString)->release();
}

size_t String8::length() const {
  return SharedBuffer::sizeFromData(mString) - 1;
}

String8 String8::format(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  String8 result(formatV(fmt, args));

  va_end(args);
  return result;
}

String8 String8::formatV(const char* fmt, va_list args) {
  String8 result;
  result.appendFormatV(fmt, args);
  return result;
}

void String8::clear() {
  SharedBuffer::bufferFromData(mString)->release();
  mString = getEmptyString();
}

void String8::setTo(const String8& other) {
  SharedBuffer::bufferFromData(other.mString)->acquire();
  SharedBuffer::bufferFromData(mString)->release();
  mString = other.mString;
}

status_t String8::setTo(const char* other) {
  const char* newString = allocFromUTF8(other, strlen(other));
  SharedBuffer::bufferFromData(mString)->release();
  mString = newString;
  if (mString)
    return NO_ERROR;

  mString = getEmptyString();
  return NO_MEMORY;
}

status_t String8::setTo(const char* other, size_t len) {
  const char* newString = allocFromUTF8(other, len);
  SharedBuffer::bufferFromData(mString)->release();
  mString = newString;
  if (mString)
    return NO_ERROR;

  mString = getEmptyString();
  return NO_MEMORY;
}

status_t String8::setTo(const char16_t* other, size_t len) {
  const char* newString = allocFromUTF16(other, len);
  SharedBuffer::bufferFromData(mString)->release();
  mString = newString;
  if (mString)
    return NO_ERROR;

  mString = getEmptyString();
  return NO_MEMORY;
}

status_t String8::setTo(const char32_t* other, size_t len) {
  const char* newString = allocFromUTF32(other, len);
  SharedBuffer::bufferFromData(mString)->release();
  mString = newString;
  if (mString)
    return NO_ERROR;

  mString = getEmptyString();
  return NO_MEMORY;
}

status_t String8::append(const String8& other) {
  const size_t otherLen = other.bytes();
  if (bytes() == 0) {
    setTo(other);
    return NO_ERROR;
  } else if (otherLen == 0) {
    return NO_ERROR;
  }

  return real_append(other.string(), otherLen);
}

status_t String8::append(const char* other) {
  return append(other, strlen(other));
}

status_t String8::append(const char* other, size_t otherLen) {
  if (bytes() == 0) {
    return setTo(other, otherLen);
  } else if (otherLen == 0) {
    return NO_ERROR;
  }

  return real_append(other, otherLen);
}

status_t String8::appendFormat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  status_t result = appendFormatV(fmt, args);

  va_end(args);
  return result;
}

status_t String8::appendFormatV(const char* fmt, va_list args) {
  int n, result = NO_ERROR;
  va_list tmp_args;

  /* args is undefined after vsnprintf.
   * So we need a copy here to avoid the
   * second vsnprintf access undefined args.
   */
  va_copy(tmp_args, args);
  n = vsnprintf(NULL, 0, fmt, tmp_args);
  va_end(tmp_args);

  if (n != 0) {
    size_t oldLength = length();
    char* buf = lockBuffer(oldLength + n);
    if (buf) {
      vsnprintf(buf + oldLength, n + 1, fmt, args);
    } else {
      result = NO_MEMORY;
    }
  }
  return result;
}

status_t String8::real_append(const char* other, size_t otherLen) {
  const size_t myLen = bytes();

  SharedBuffer* buf =
      SharedBuffer::bufferFromData(mString)->editResize(myLen + otherLen + 1);
  if (buf) {
    char* str = (char*)buf->data();
    mString = str;
    str += myLen;
    memcpy(str, other, otherLen);
    str[otherLen] = '\0';
    return NO_ERROR;
  }
  return NO_MEMORY;
}

char* String8::lockBuffer(size_t size) {
  SharedBuffer* buf =
      SharedBuffer::bufferFromData(mString)->editResize(size + 1);
  if (buf) {
    char* str = (char*)buf->data();
    mString = str;
    return str;
  }
  return NULL;
}

void String8::unlockBuffer() {
  unlockBuffer(strlen(mString));
}

status_t String8::unlockBuffer(size_t size) {
  if (size != this->size()) {
    SharedBuffer* buf =
        SharedBuffer::bufferFromData(mString)->editResize(size + 1);
    if (!buf) {
      return NO_MEMORY;
    }

    char* str = (char*)buf->data();
    str[size] = 0;
    mString = str;
  }

  return NO_ERROR;
}

ssize_t String8::find(const char* other, size_t start) const {
  size_t len = size();
  if (start >= len) {
    return -1;
  }
  const char* s = mString + start;
  const char* p = strstr(s, other);
  return p ? p - mString : -1;
}

bool String8::removeAll(const char* other) {
  ssize_t index = find(other);
  if (index < 0)
    return false;

  char* buf = lockBuffer(size());
  if (!buf)
    return false;  // out of memory

  size_t skip = strlen(other);
  size_t len = size();
  size_t tail = index;
  while (size_t(index) < len) {
    ssize_t next = find(other, index + skip);
    if (next < 0) {
      next = len;
    }

    memmove(buf + tail, buf + index + skip, next - index - skip);
    tail += next - index - skip;
    index = next;
  }
  unlockBuffer(tail);
  return true;
}

void String8::toLower() {
  toLower(0, size());
}

void String8::toLower(size_t start, size_t length) {
  const size_t len = size();
  if (start >= len) {
    return;
  }
  if (start + length > len) {
    length = len - start;
  }
  char* buf = lockBuffer(len);
  buf += start;
  while (length > 0) {
    *buf = tolower(*buf);
    buf++;
    length--;
  }
  unlockBuffer(len);
}

void String8::toUpper() {
  toUpper(0, size());
}

void String8::toUpper(size_t start, size_t length) {
  const size_t len = size();
  if (start >= len) {
    return;
  }
  if (start + length > len) {
    length = len - start;
  }
  char* buf = lockBuffer(len);
  buf += start;
  while (length > 0) {
    *buf = toupper(*buf);
    buf++;
    length--;
  }
  unlockBuffer(len);
}

static inline int32_t utf32_at_internal(const char* cur, size_t* num_read) {
  const char first_char = *cur;
  if ((first_char & 0x80) == 0) {  // ASCII
    *num_read = 1;
    return *cur;
  }
  cur++;
  char32_t mask, to_ignore_mask;
  size_t num_to_read = 0;
  char32_t utf32 = first_char;
  for (num_to_read = 1, mask = 0x40, to_ignore_mask = 0xFFFFFF80;
       (first_char & mask); num_to_read++, to_ignore_mask |= mask, mask >>= 1) {
    // 0x3F == 00111111
    utf32 = (utf32 << 6) + (*cur++ & 0x3F);
  }
  to_ignore_mask |= mask;
  utf32 &= ~(to_ignore_mask << (6 * (num_to_read - 1)));

  *num_read = num_to_read;
  return static_cast<int32_t>(utf32);
}

size_t utf8_to_utf32_length(const char* src, size_t src_len) {
  if (src == NULL || src_len == 0) {
    return 0;
  }
  size_t ret = 0;
  const char* cur;
  const char* end;
  size_t num_to_skip;
  for (cur = src, end = src + src_len, num_to_skip = 1; cur < end;
       cur += num_to_skip, ret++) {
    const char first_char = *cur;
    num_to_skip = 1;
    if ((first_char & 0x80) == 0) {  // ASCII
      continue;
    }
    int32_t mask;

    for (mask = 0x40; (first_char & mask); num_to_skip++, mask >>= 1) {
    }
  }
  return ret;
}

int32_t utf32_from_utf8_at(const char* src, size_t src_len, size_t index,
                           size_t* next_index) {
  if (index >= src_len) {
    return -1;
  }
  size_t dummy_index;
  if (next_index == NULL) {
    next_index = &dummy_index;
  }
  size_t num_read;
  int32_t ret = utf32_at_internal(src + index, &num_read);
  if (ret >= 0) {
    *next_index = index + num_read;
  }

  return ret;
}

void utf8_to_utf32(const char* src, size_t src_len, char32_t* dst) {
  if (src == NULL || src_len == 0 || dst == NULL) {
    return;
  }

  const char* cur = src;
  const char* const end = src + src_len;
  char32_t* cur_utf32 = dst;
  while (cur < end) {
    size_t num_read;
    *cur_utf32++ = static_cast<char32_t>(utf32_at_internal(cur, &num_read));
    cur += num_read;
  }
  *cur_utf32 = 0;
}

size_t String8::getUtf32Length() const {
  return utf8_to_utf32_length(mString, length());
}

int32_t String8::getUtf32At(size_t index, size_t* next_index) const {
  return utf32_from_utf8_at(mString, length(), index, next_index);
}

void String8::getUtf32(char32_t* dst) const {
  utf8_to_utf32(mString, length(), dst);
}

// ---------------------------------------------------------------------------
// Path functions

void String8::setPathName(const char* name) {
  setPathName(name, strlen(name));
}

void String8::setPathName(const char* name, size_t len) {
  char* buf = lockBuffer(len);

  memcpy(buf, name, len);

  // remove trailing path separator, if present
  if (len > 0 && buf[len - 1] == OS_PATH_SEPARATOR)
    len--;

  buf[len] = '\0';

  unlockBuffer(len);
}

String8 String8::getPathLeaf(void) const {
  const char* cp;
  const char* const buf = mString;

  cp = strrchr(buf, OS_PATH_SEPARATOR);
  if (cp == NULL)
    return String8(*this);
  else
    return String8(cp + 1);
}

String8 String8::getPathDir(void) const {
  const char* cp;
  const char* const str = mString;

  cp = strrchr(str, OS_PATH_SEPARATOR);
  if (cp == NULL)
    return String8("");
  else
    return String8(str, cp - str);
}

String8 String8::walkPath(String8* outRemains) const {
  const char* cp;
  const char* const str = mString;
  const char* buf = str;

  cp = strchr(buf, OS_PATH_SEPARATOR);
  if (cp == buf) {
    // don't include a leading '/'.
    buf = buf + 1;
    cp = strchr(buf, OS_PATH_SEPARATOR);
  }

  if (cp == NULL) {
    String8 res = buf != str ? String8(buf) : *this;
    if (outRemains)
      *outRemains = String8("");
    return res;
  }

  String8 res(buf, cp - buf);
  if (outRemains)
    *outRemains = String8(cp + 1);
  return res;
}

/*
 * Helper function for finding the start of an extension in a pathname.
 *
 * Returns a pointer inside mString, or NULL if no extension was found.
 */
char* String8::find_extension(void) const {
  const char* lastSlash;
  const char* lastDot;
  const char* const str = mString;

  // only look at the filename
  lastSlash = strrchr(str, OS_PATH_SEPARATOR);
  if (lastSlash == NULL)
    lastSlash = str;
  else
    lastSlash++;

  // find the last dot
  lastDot = strrchr(lastSlash, '.');
  if (lastDot == NULL)
    return NULL;

  // looks good, ship it
  return const_cast<char*>(lastDot);
}

String8 String8::getPathExtension(void) const {
  char* ext;

  ext = find_extension();
  if (ext != NULL)
    return String8(ext);
  else
    return String8("");
}

String8 String8::getBasePath(void) const {
  char* ext;
  const char* const str = mString;

  ext = find_extension();
  if (ext == NULL)
    return String8(*this);
  else
    return String8(str, ext - str);
}

String8& String8::appendPath(const char* name) {
  // TODO: The test below will fail for Win32 paths. Fix later or ignore.
  if (name[0] != OS_PATH_SEPARATOR) {
    if (*name == '\0') {
      // nothing to do
      return *this;
    }

    size_t len = length();
    if (len == 0) {
      // no existing filename, just use the new one
      setPathName(name);
      return *this;
    }

    // make room for oldPath + '/' + newPath
    int newlen = strlen(name);

    char* buf = lockBuffer(len + 1 + newlen);

    // insert a '/' if needed
    if (buf[len - 1] != OS_PATH_SEPARATOR)
      buf[len++] = OS_PATH_SEPARATOR;

    memcpy(buf + len, name, newlen + 1);
    len += newlen;

    unlockBuffer(len);

    return *this;
  } else {
    setPathName(name);
    return *this;
  }
}

String8& String8::convertToResPath() {
#if OS_PATH_SEPARATOR != RES_PATH_SEPARATOR
  size_t len = length();
  if (len > 0) {
    char* buf = lockBuffer(len);
    for (char* end = buf + len; buf < end; ++buf) {
      if (*buf == OS_PATH_SEPARATOR)
        *buf = RES_PATH_SEPARATOR;
    }
    unlockBuffer(len);
  }
#endif
  return *this;
}

};  // namespace hwcomposer
