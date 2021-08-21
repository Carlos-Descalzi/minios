#ifndef _POLL_H_
#define _POLL_H_


#define POLLIN  1

typedef uint16_t nfds_t;

struct pollfd {
   int   fd;         /* file descriptor */
   short events;     /* requested events */
   short revents;    /* returned events */
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif
