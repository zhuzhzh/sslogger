#ifndef SSLN_SSLOGGER_MACROS_H_
#define SSLN_SSLOGGER_MACROS_H_

/**
 * This file provides simplified logging macros using the default logger.
 * It is recommended to use these macros instead of the raw Quill macros
 * for better consistency and easier maintenance.
 */

// Disable Quill's non-prefixed macros
#define QUILL_DISABLE_NON_PREFIXED_MACROS

#include "quill/LogMacros.h"
#include "sslogger.h"

// Basic logging macros using default logger
#define SSLN_TRACE_L3(fmt, ...)  QUILL_LOG_TRACE_L3(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_TRACE_L2(fmt, ...)  QUILL_LOG_TRACE_L2(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_TRACE_L1(fmt, ...)  QUILL_LOG_TRACE_L1(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_DEBUG(fmt, ...)     QUILL_LOG_DEBUG(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_INFO(fmt, ...)      QUILL_LOG_INFO(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_WARNING(fmt, ...)   QUILL_LOG_WARNING(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_ERROR(fmt, ...)     QUILL_LOG_ERROR(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_CRITICAL(fmt, ...)  QUILL_LOG_CRITICAL(::ssln::default_logger, fmt, ##__VA_ARGS__)

#define SSLN_LOG_TRACE_L3(logger, fmt, ...)  QUILL_LOG_TRACE_L3(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_TRACE_L2(logger, fmt, ...)  QUILL_LOG_TRACE_L2(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_TRACE_L1(logger, fmt, ...)  QUILL_LOG_TRACE_L1(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_DEBUG(logger, fmt, ...)  QUILL_LOG_DEBUG(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_INFO(logger, fmt, ...)  QUILL_LOG_INFO(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_WARNING(logger, fmt, ...)  QUILL_LOG_WARNING(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_ERROR(logger, fmt, ...)  QUILL_LOG_ERROR(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_CRITICAL(logger, fmt, ...)  QUILL_LOG_CRITICAL(logger, fmt, ##__VA_ARGS__)

// Optional: provide shorter aliases if SSLN_SHORT_MACROS is defined
#ifdef SSLN_SHORT_MACROS
#define TRACE3(fmt, ...)  SSLN_TRACE_L3(fmt, ##__VA_ARGS__)
#define TRACE2(fmt, ...)  SSLN_TRACE_L2(fmt, ##__VA_ARGS__)
#define TRACE1(fmt, ...)  SSLN_TRACE_L1(fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)   SSLN_DEBUG(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)    SSLN_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)    SSLN_WARNING(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)   SSLN_ERROR(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...)   SSLN_CRITICAL(fmt, ##__VA_ARGS__)
#endif

#endif  // SSLN_SSLOGGER_MACROS_H_ 