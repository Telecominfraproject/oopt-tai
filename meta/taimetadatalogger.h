/**
 * @file    taimetadatalogger.h
 *
 * @brief   This module defines TAI Metadata Logger
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 *
 * @remark  Licensed under the Apache License, Version 2.0 (the "License"); you
 *          may not use this file except in compliance with the License. You may
 *          obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 * @remark  THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *          CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *          LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *          FOR A PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 * @remark  See the Apache Version 2.0 License for specific language governing
 *          permissions and limitations under the License.
 *
 * @remark  Microsoft would like to thank the following companies for their
 *          review and assistance with these files: Intel Corporation, Mellanox
 *          Technologies Ltd, Dell Products, L.P., Facebook, Inc., Marvell
 *          International Ltd.
 */

#ifndef __TAIMETADATALOGGER_H_
#define __TAIMETADATALOGGER_H_

/**
 * @defgroup TAIMETADATALOGGER TAI - Metadata Logger Definitions
 *
 * @{
 */

/**
 * @brief Log level function definition.
 *
 * User can specify his own function that will be called when message log level
 * will be greater or equal to #tai_metadata_log_level.
 *
 * @param[in] log_level Log level
 * @param[in] file Source file
 * @param[in] line Line number in file
 * @param[in] function Function name
 * @param[in] format Format of logging
 * @param[in] ... Variable parameters
 */
typedef void (*tai_metadata_log_fn)(
        _In_ tai_log_level_t log_level,
        _In_ const char *file,
        _In_ int line,
        _In_ const char *function,
        _In_ const char *format,
        _In_ ...);

/**
 * @brief User specified log function.
 *
 * TODO: add a set function to update this?
 */
extern volatile tai_metadata_log_fn tai_metadata_log;

/**
 * @brief Log level for TAI metadata macros.
 *
 * Log level can be changed by user at any time.
 *
 * TODO: add a set function to update this?
 */
extern volatile tai_log_level_t tai_metadata_log_level;

/**
 * @brief Helper log macro definition
 *
 * If logger function is NULL, stderr is used to print messages. Also, fprintf
 * function will validate parameters at compilation time.
 */
#define TAI_META_LOG(loglevel,format,...)\
    if (loglevel >= tai_metadata_log_level)\
{\
    if (tai_metadata_log == NULL) /* or syslog? */ \
        fprintf(stderr, "%s:%d %s: " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    else\
        tai_metadata_log(loglevel, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);\
}

/*
 * Helper macros.
 */

#define TAI_META_LOG_ENTER()                TAI_META_LOG(TAI_LOG_LEVEL_DEBUG, ":> enter");
#define TAI_META_LOG_DEBUG(format,...)      TAI_META_LOG(TAI_LOG_LEVEL_DEBUG, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_INFO(format,...)       TAI_META_LOG(TAI_LOG_LEVEL_INFO, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_NOTICE(format,...)     TAI_META_LOG(TAI_LOG_LEVEL_NOTICE, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_WARN(format,...)       TAI_META_LOG(TAI_LOG_LEVEL_WARN, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_ERROR(format,...)      TAI_META_LOG(TAI_LOG_LEVEL_ERROR, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_CRITICAL(format,...)   TAI_META_LOG(TAI_LOG_LEVEL_CRITICAL, ":- " format, ##__VA_ARGS__)
#define TAI_META_LOG_EXIT()                 TAI_META_LOG(TAI_LOG_LEVEL_DEBUG, ":< exit");

/**
 * @}
 */
#endif /** __TAIMETADATALOGGER_H_ */
