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

#include <lfp/sendfile.h>

#if defined(HAVE_SENDFILE)
# if defined(__linux__)
#  include <sys/sendfile.h>
# elif defined(__FreeBSD__)
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/uio.h>
# elif defined(__APPLE__)
// OSX doesn't expose sendfile if XOPEN_SOURCE is defined
int sendfile(int, int, off_t, off_t *, void *, int);
# endif
#endif

#include <stdlib.h>

DSO_PUBLIC ssize_t
lfp_sendfile(int out_fd, int in_fd, off_t offset, size_t nbytes)
{
#if defined(HAVE_SENDFILE)
# if defined(__linux__)
    off_t off = offset;
    return (ssize_t) sendfile(out_fd, in_fd, &off, nbytes);
# elif defined(__FreeBSD__)
    return (ssize_t) sendfile(in_fd, out_fd, offset, nbytes, NULL, NULL, SF_MNOWAIT);
# elif defined(__APPLE__)
    off_t len = nbytes;
    return (ssize_t) sendfile(in_fd, out_fd, offset, &len, NULL, 0);
# else
#  error "It appears that this OS has sendfile(), but LFP doesn't use it at the moment"
#  error "Please send an email to iolib-devel@common-lisp.net"
# endif
#else
    return ENOSYS;
#endif
}
