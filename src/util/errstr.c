/*
 * Copyright (C) 2021   Steffen Nuessle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "errstr.h"
#include "macro.h"

const char *errstr(int code)
{
    static const char *names[] = {
        [0] = "OK",
        [EPERM] = "EPERM",
        [ENOENT] = "ENOENT",
        [ESRCH] = "ESRCH",
        [EINTR] = "EINTR",
        [EIO] = "EIO",
        [ENXIO] = "ENXIO",
        [E2BIG] = "E2BIG",
        [ENOEXEC] = "ENOEXEC",
        [EBADF] = "EBADF",
        [ECHILD] = "ECHILD",
        [EAGAIN] = "EAGAIN",
        [ENOMEM] = "ENOMEM",
        [EACCES] = "EACCES",
        [EFAULT] = "EFAULT",
        [ENOTBLK] = "ENOTBLK",
        [EBUSY] = "EBUSY",
        [EEXIST] = "EEXIST",
        [EXDEV] = "EXDEV",
        [ENODEV] = "ENODEV",
        [ENOTDIR] = "ENOTDIR",
        [EISDIR] = "EISDIR",
        [EINVAL] = "EINVAL",
        [ENFILE] = "ENFILE",
        [EMFILE] = "EMFILE",
        [ENOTTY] = "ENOTTY",
        [ETXTBSY] = "ETXTBSY",
        [EFBIG] = "EFBIG",
        [ENOSPC] = "ENOSPC",
        [ESPIPE] = "ESPIPE",
        [EROFS] = "EROFS",
        [EMLINK] = "EMLINK",
        [EPIPE] = "EPIPE",
        [EDOM] = "EDOM",
        [ERANGE] = "ERANGE",
        [EDEADLK] = "EDEADLK",
        [ENAMETOOLONG] = "ENAMETOOLONG",
        [ENOLCK] = "ENOLCK",
        [ENOSYS] = "ENOSYS",
        [ENOTEMPTY] = "ENOTEMPTY",
        [ELOOP] = "ELOOP",
        [ENOMSG] = "ENOMSG",
        [EIDRM] = "EIDRM",
        [ECHRNG] = "ECHRNG",
        [EL2NSYNC] = "EL2NSYNC",
        [EL3HLT] = "EL3HLT",
        [EL3RST] = "EL3RST",
        [ELNRNG] = "ELNRNG",
        [EUNATCH] = "EUNATCH",
        [ENOCSI] = "ENOCSI",
        [EL2HLT] = "EL2HLT",
        [EBADE] = "EBADE",
        [EBADR] = "EBADR",
        [EXFULL] = "EXFULL",
        [ENOANO] = "ENOANO",
        [EBADRQC] = "EBADRQC",
        [EBADSLT] = "EBADSLT",
        [EBFONT] = "EBFONT",
        [ENOSTR] = "ENOSTR",
        [ENODATA] = "ENODATA",
        [ETIME] = "ETIME",
        [ENOSR] = "ENOSR",
        [ENONET] = "ENONET",
        [ENOPKG] = "ENOPKG",
        [EREMOTE] = "EREMOTE",
        [ENOLINK] = "ENOLINK",
        [EADV] = "EADV",
        [ESRMNT] = "ESRMNT",
        [ECOMM] = "ECOMM",
        [EPROTO] = "EPROTO",
        [EMULTIHOP] = "EMULTIHOP",
        [EDOTDOT] = "EDOTDOT",
        [EBADMSG] = "EBADMSG",
        [EOVERFLOW] = "EOVERFLOW",
        [ENOTUNIQ] = "ENOTUNIQ",
        [EBADFD] = "EBADFD",
        [EREMCHG] = "EREMCHG",
        [ELIBACC] = "ELIBACC",
        [ELIBBAD] = "ELIBBAD",
        [ELIBSCN] = "ELIBSCN",
        [ELIBMAX] = "ELIBMAX",
        [ELIBEXEC] = "ELIBEXEC",
        [EILSEQ] = "EILSEQ",
        [ERESTART] = "ERESTART",
        [ESTRPIPE] = "ESTRPIPE",
        [EUSERS] = "EUSERS",
        [ENOTSOCK] = "ENOTSOCK",
        [EDESTADDRREQ] = "EDESTADDRREQ",
        [EMSGSIZE] = "EMSGSIZE",
        [EPROTOTYPE] = "EPROTOTYPE",
        [ENOPROTOOPT] = "ENOPROTOOPT",
        [EPROTONOSUPPORT] = "EPROTONOSUPPORT",
        [ESOCKTNOSUPPORT] = "ESOCKTNOSUPPORT",
        [EOPNOTSUPP] = "EOPNOTSUPP",
        [EPFNOSUPPORT] = "EPFNOSUPPORT",
        [EAFNOSUPPORT] = "EAFNOSUPPORT",
        [EADDRINUSE] = "EADDRINUSE",
        [EADDRNOTAVAIL] = "EADDRNOTAVAIL",
        [ENETDOWN] = "ENETDOWN",
        [ENETUNREACH] = "ENETUNREACH",
        [ENETRESET] = "ENETRESET",
        [ECONNABORTED] = "ECONNABORTED",
        [ECONNRESET] = "ECONNRESET",
        [ENOBUFS] = "ENOBUFS",
        [EISCONN] = "EISCONN",
        [ENOTCONN] = "ENOTCONN",
        [ESHUTDOWN] = "ESHUTDOWN",
        [ETOOMANYREFS] = "ETOOMANYREFS",
        [ETIMEDOUT] = "ETIMEDOUT",
        [ECONNREFUSED] = "ECONNREFUSED",
        [EHOSTDOWN] = "EHOSTDOWN",
        [EHOSTUNREACH] = "EHOSTUNREACH",
        [EALREADY] = "EALREADY",
        [EINPROGRESS] = "EINPROGRESS",
        [ESTALE] = "ESTALE",
        [EUCLEAN] = "EUCLEAN",
        [ENOTNAM] = "ENOTNAM",
        [ENAVAIL] = "ENAVAIL",
        [EISNAM] = "EISNAM",
        [EREMOTEIO] = "EREMOTEIO",
        [EDQUOT] = "EDQUOT",
        [ENOMEDIUM] = "ENOMEDIUM",
        [EMEDIUMTYPE] = "EMEDIUMTYPE",
        [ECANCELED] = "ECANCELED",
        [ENOKEY] = "ENOKEY",
        [EKEYEXPIRED] = "EKEYEXPIRED",
        [EKEYREVOKED] = "EKEYREVOKED",
        [EKEYREJECTED] = "EKEYREJECTED",
        [EOWNERDEAD] = "EOWNERDEAD",
        [ENOTRECOVERABLE] = "ENOTRECOVERABLE",
        [ERFKILL] = "ERFKILL",
        [EHWPOISON] = "EHWPOISON",
    };

    if (code == INT_MIN)
        return NULL;

    code = abs(code);

    if (code >= ARRAY_SIZE(names))
        return NULL;

    return names[code];
}

