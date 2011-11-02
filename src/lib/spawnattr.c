/*******************************************************************************/
/* Permission is hereby granted, free of charge, to any person or organization */
/* obtaining a copy of the software and accompanying documentation covered by  */
/* this license (the "Software") to use, reproduce, display, distribute,       */
/* execute, and transmit the Software, and to prepare derivative works of the  */
/* Software, and to permit third-parties to whom the Software is furnished to  */
/* do so, all subject to the following:                                        */
/*                                                                             */
/* The copyright notices in the Software and this entire statement, including  */
/* the above license grant, this restriction and the following disclaimer,     */
/* must be included in all copies of the Software, in whole or in part, and    */
/* all derivative works of the Software, unless such copies or derivative      */
/* works are solely in the form of machine-executable object code generated by */
/* a source language processor.                                                */
/*                                                                             */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    */
/* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT   */
/* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE   */
/* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, */
/* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER */
/* DEALINGS IN THE SOFTWARE.                                                   */
/*******************************************************************************/

#include <lfp/spawn.h>
#include <lfp/stdlib.h>
#include <lfp/string.h>
#include <lfp/errno.h>
#include <lfp/unistd.h>
#include <lfp/signal.h>

#include <stdio.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/termios.h>

#include "spawn.h"

#define LFP_SPAWN_ALLATTRS ( LFP_SPAWN_SETSIGMASK    | \
                             LFP_SPAWN_SETSIGDEFAULT | \
                             LFP_SPAWN_SETPGROUP     | \
                             LFP_SPAWN_RESETIDS      | \
                             LFP_SPAWN_SETUID        | \
                             LFP_SPAWN_SETGID        | \
                             LFP_SPAWN_SETCWD        | \
                             LFP_SPAWN_SETSID        | \
                             LFP_SPAWN_SETCTTY       )

DSO_PUBLIC int
lfp_spawnattr_init(lfp_spawnattr_t *attr)
{
    SYSCHECK(EINVAL, attr == NULL);
    memset(attr, 0, sizeof(lfp_spawnattr_t));
    sigemptyset(&attr->sigdefault);
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_destroy(lfp_spawnattr_t *attr)
{
    SYSCHECK(EINVAL, attr == NULL);
    if (attr->chdir_path) {
        free(attr->chdir_path);
        attr->chdir_path = NULL;
    }
    if (attr->pts_path) {
        free(attr->pts_path);
        attr->pts_path = NULL;
    }
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getflags(lfp_spawnattr_t *attr, uint32_t *flags)
{
    SYSCHECK(EINVAL, attr == NULL || flags == NULL);
    *flags = attr->flags;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setflags(lfp_spawnattr_t *attr, const uint32_t flags)
{
    SYSCHECK(EINVAL, attr == NULL || (flags & ~LFP_SPAWN_ALLATTRS) != 0);
    attr->flags = flags;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getsigmask(lfp_spawnattr_t *attr, sigset_t *sigmask)
{
    SYSCHECK(EINVAL, attr == NULL || sigmask == NULL);
    memcpy(sigmask, &attr->sigmask, sizeof(sigset_t));
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setsigmask(lfp_spawnattr_t *attr, const sigset_t *sigmask)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETSIGMASK;
    memcpy(&attr->sigmask, sigmask, sizeof(sigset_t));
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getsigdefault(lfp_spawnattr_t *attr, sigset_t *sigdefault)
{
    SYSCHECK(EINVAL, attr == NULL || sigdefault == NULL);
    memcpy(sigdefault, &attr->sigdefault, sizeof(sigset_t));
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setsigdefault(lfp_spawnattr_t *attr, const sigset_t *sigdefault)
{
    SYSCHECK(EINVAL, attr == NULL || sigdefault == NULL);
    attr->flags |= LFP_SPAWN_SETSIGDEFAULT;
    memcpy(&attr->sigdefault, sigdefault, sizeof(sigset_t));
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getpgroup(lfp_spawnattr_t *attr, pid_t *pgroup)
{
    SYSCHECK(EINVAL, attr == NULL || pgroup == NULL);
    *pgroup = attr->pgroup;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setpgroup(lfp_spawnattr_t *attr, const pid_t pgroup)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETPGROUP;
    attr->pgroup = pgroup;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setsid(lfp_spawnattr_t *attr)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETSID;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getctty(lfp_spawnattr_t *attr, char **path)
{
    SYSCHECK(EINVAL, attr == NULL || path == NULL);
    *path = strdup(attr->pts_path);
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setctty(lfp_spawnattr_t *attr, const char *path)
{
    SYSCHECK(EINVAL, attr == NULL || path == NULL);
    attr->flags |= LFP_SPAWN_SETCTTY;
    if (attr->pts_path) {
        free(attr->pts_path);
    }
    attr->pts_path = strdup(path);
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getcwd(lfp_spawnattr_t *attr, char **path)
{
    SYSCHECK(EINVAL, attr == NULL || path == NULL);
    *path = strdup(attr->chdir_path);
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setcwd(lfp_spawnattr_t *attr, const char *path)
{
    SYSCHECK(EINVAL, attr == NULL || path == NULL);
    attr->flags |= LFP_SPAWN_SETCWD;
    if (attr->chdir_path) {
        free(attr->chdir_path);
    }
    attr->chdir_path = strdup(path);
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getuid(lfp_spawnattr_t *attr, uid_t *uid)
{
    SYSCHECK(EINVAL, attr == NULL || uid == NULL);
    *uid = attr->uid;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setuid(lfp_spawnattr_t *attr, const uid_t uid)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETUID;
    attr->uid = uid;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_getgid(lfp_spawnattr_t *attr, gid_t *gid)
{
    SYSCHECK(EINVAL, attr == NULL || gid == NULL);
    *gid = attr->gid;
    return 0;
}

DSO_PUBLIC int
lfp_spawnattr_setgid(lfp_spawnattr_t *attr, const gid_t gid)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETGID;
    attr->gid = gid;
    return 0;
}



int lfp_spawn_apply_attributes(const lfp_spawnattr_t *attr)
{
    if (attr == NULL) return 0;

    SYSCHECK(EINVAL, (attr->flags & LFP_SPAWN_RESETIDS) && \
                     ((attr->flags & LFP_SPAWN_SETUID)  || \
                      (attr->flags & LFP_SPAWN_SETGID)));

    if (attr->flags & LFP_SPAWN_SETSIGMASK)
        if (sigprocmask(SIG_SETMASK, &attr->sigmask, NULL) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETSIGMASK:sigprocmask");
#endif
            goto error_return;
        }

    if (attr->flags & LFP_SPAWN_SETSIGDEFAULT) {
        struct sigaction sa = { .sa_flags   = 0,
                                .sa_handler = SIG_DFL };
        for (int i = 1; i <= NSIG; i++)
            if (sigismember(&attr->sigdefault, i))
                if (sigaction(i, &sa, NULL) < 0) {
#if !defined(NDEBUG)
                    perror("LFP_SPAWN_APPLY_ATTR:SETSIGDEFAULT:sigaction");
#endif
                    goto error_return;
                }
    }

    if (attr->flags & LFP_SPAWN_SETPGROUP)
        if (setpgid(0, attr->pgroup) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETPGROUP:setpgid");
#endif
            goto error_return;
        }

    if (attr->flags & LFP_SPAWN_SETSID)
        if (setsid() < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETSID:setsid");
#endif
            goto error_return;
        }

    if (attr->flags & LFP_SPAWN_SETCTTY) {
        int ttyfd = lfp_open(attr->pts_path, O_RDWR | O_NOCTTY);
        if (ttyfd  < 0) {
#if !defined(NDEBUG)
		perror("LFP_SPAWN_APPLY_ATTR:SETCTTY:lfp_open");
#endif
		goto error_return;
        } else {
	    if (ioctl(ttyfd, TIOCSCTTY) < 0) {
#if !defined(NDEBUG)
		perror("LFP_SPAWN_APPLY_ATTR:SETCTTY:ioctl");
#endif
		goto error_return;
	    }
	}
    }

    if (attr->flags & LFP_SPAWN_SETCWD)
        if (chdir(attr->chdir_path) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETCWD:chdir");
#endif
            goto error_return;
        }

    if (attr->flags & LFP_SPAWN_RESETIDS) {
        if (seteuid(getuid()) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:RESETIDS:seteuid");
#endif
            goto error_return;
        }
        if (setegid(getgid()) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:RESETIDS:setegid");
#endif
            goto error_return;
        }
    }

    if (attr->flags & LFP_SPAWN_SETUID)
        if (seteuid(attr->uid) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETUID:seteuid");
#endif
            goto error_return;
        }

    if (attr->flags & LFP_SPAWN_SETGID)
        if (setegid(attr->gid) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETGID:setegid");
#endif
            goto error_return;
        }

    return 0;
  error_return:
    return lfp_errno();
}
