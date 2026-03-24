/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef PROBE_CONFIG_H_
#define PROBE_CONFIG_H_

#ifdef PROBE_DEBUG_ENABLE
#define PROBE_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#define PROBE_INFO(...)  FURI_LOG_I(TAG, __VA_ARGS__)
#else
#define PROBE_DEBUG(...)
#define PROBE_INFO(...)
#endif

#include "board_probe_config.h"

#define PROTO_DAP_V1 1
#define PROTO_DAP_V2 2

// Interface config
#ifndef PROBE_DEBUG_PROTOCOL
#define PROBE_DEBUG_PROTOCOL PROTO_DAP_V2
#endif

#endif
