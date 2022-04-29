
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <rpc/xdr.h>

#include <errno.h>

#define emit_trouble(x) printf("%s",x);

int writeit(char *handle, char *buf, int len) {

  printf("Writing %d bytes.\n", len);

  return len;
  
}

int readit(char *handle, char *buf, int len) {

  int *pfd;

  ssize_t bytes_read;
  
  printf("Reading %d bytes.\n", len);

  pfd = (int *) handle;

  bytes_read = read(pfd[0], buf, len);
  
  return bytes_read;
  
}

int main(int argc, char *argv[]) {

  XDR xdrs[2];

  unsigned long board[64];

  int retval;
  
  long int sqrno;

  int fd;

  char *xdr_fn;

  for (sqrno = 0; sqrno < 64; sqrno++) {
    board[sqrno] = 0;
  }

  xdr_fn = argv[1];

  printf("Opening XDR filename %s\n", xdr_fn);
  
  fd = open(xdr_fn, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return -1;
  }

  printf("Decoding XDR stream.\n");
  
  xdrrec_create(xdrs + 0, 0, 0, (char*) (&fd), readit, writeit);

  xdrs[0].x_op = XDR_DECODE;  

  retval = xdrrec_skiprecord(xdrs + 0);
  if (retval != 1) {
    emit_trouble("xdrrec_skiprecord");
    return -1;
  }

  printf("Reading vector of XDR records.\n");
  
  retval = xdr_vector(xdrs + 0, (char*) board, 64, sizeof(unsigned long), (void*) xdr_u_long);
  if (retval != 1) {
    emit_trouble("xdr_vector");
    return -1;
  }

  printf("Values ");
  
  for (sqrno = 0; sqrno < 64; sqrno++) {
    printf("%lu ", board[sqrno]);
  }

  putchar('\n');
  
  xdr_destroy(xdrs + 0);

  printf("Closing XDR filename.\n");
  
  retval = close(fd);
  if (retval == -1) {
    perror("fclose");
    return -1;
  }

  return 0;

}

