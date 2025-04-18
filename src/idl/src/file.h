// Copyright(c) 2021 ZettaScale Technology and others
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License v. 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
// v. 1.0 which is available at
// http://www.eclipse.org/org/documents/edl-v10.php.
//
// SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause

#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <stddef.h>

#include "idl/export.h"
#include "idl/retcode.h"
#include "idl/file.h"

#if _MSC_VER
# include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

/**
 * @internal
 * @brief Resolve "." and ".." segments and remove consecutive slashes
 */
IDL_EXPORT ssize_t idl_untaint_path(char *path);

#endif /* FILE_H */
