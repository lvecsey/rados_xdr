
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <rpc/xdr.h>

#include <rados/librados.h>

typedef struct {

  rados_completion_t completion;

  rados_ioctx_t ioctx;

  char *oid;
  
} handle_pack;

int readit(char *handle, char *buf, int len) {

  printf("Reading %d bytes.\n", len);

  return len;

}

int writeit(char *handle, char *buf, int len) {

  handle_pack *hp;

  int retval;
  
  hp = (handle_pack*) handle;
  
  printf("Writing %d bytes.\n", len);

  retval = rados_aio_append(hp->ioctx, hp->oid, hp->completion, buf, len);
  if (retval == -1) {
    perror("rados_aio_append");
    return -1;
  }

  retval = rados_aio_wait_for_complete(hp->completion);
  if (retval == -1) {
    perror("rados_aio_wait_for_complete");
    return -1;
  }
  
  return len;

}



void cb_complete(rados_completion_t cb, void *arg) {

  printf("Rados cb_complete.\n");
  
}

void cb_safe(rados_completion_t cb, void *arg) {

  printf("Rados cb_safe.\n");
  
}

int main(int argc, char *argv[]) {

  XDR xdrs[2];

  rados_t cluster;
  
  unsigned long board[64];

  int retval;
  
  long int sqrno;

  handle_pack hp;

  hp.oid = "encoded_stream.xdr";
  
  {

    int major, minor, extra;
    rados_version(&major, &minor, &extra);
    printf("Rados version %d.%d.%d\n", major, minor, extra);
  }
      
  {

    retval = rados_create2(&cluster, "ceph", "client.silverspace", 0);
    if (retval == -1) {
      perror("rados_create2");
      return -1;
    }

    retval = rados_conf_read_file(cluster, "/etc/ceph/ceph_ipv4.conf");
    if (retval == -1) {
      perror("rados_conf_parse_argv");
      return -1;
    }

    printf("Connecting to rados cluster.\n");
    
    retval = rados_connect(cluster);
    if (retval == -1) {
      perror("rados_connect");
      return -1;
    }

    printf("Creating rados ioctx.\n");
    
    {

      char *poolname;

      poolname = "work";
    
      retval = rados_ioctx_create(cluster, poolname, &(hp.ioctx));
      if (retval == -1) {
	perror("rados_ioctx_create");
	return -1;
      }

      /*
      
      printf("Retrieving rados pool stats.\n");
      
      {
	struct rados_pool_stat_t stats;

	rados_ioctx_pool_stat(&(hp.ioctx), &stats);

	printf("pool: num_bytes %lu num_kb %lu num_objects %lu\n", stats.num_bytes, stats.num_kb, stats.num_objects);
	
      }

      */
	
      printf("Setting rados namespace.\n");

      {

	char *namespace;

	namespace = "chessdb";
	
	rados_ioctx_set_namespace(hp.ioctx, namespace);
	
      }

      printf("Setting up rados aio completion.\n");
      
      {
      
	retval = rados_aio_create_completion(NULL, cb_complete, cb_safe, &(hp.completion));
	if (retval == -1) {
	  perror("rados_aio_create_completion");
	  return -1;
	}

      }
      
      
    }
    
  }

  {

    struct rados_cluster_stat_t cstat;
    
    retval = rados_cluster_stat(cluster, &cstat);
    if (retval == -1) {
      perror("rados_cluster_stat");
      return -1;
    }

    printf("kb %lu kb_usd %lu kb_avail %lu num_objects %lu\n", cstat.kb, cstat.kb_used, cstat.kb_avail, cstat.num_objects);

  }
  
  for (sqrno = 0; sqrno < 64; sqrno++) {
    board[sqrno] = sqrno;
  }
  
  xdrrec_create(xdrs + 1, 0, 0, (char*) (&hp), readit, writeit);

  xdrs[1].x_op = XDR_ENCODE;

  xdrrec_skiprecord(xdrs + 1);  
  
  xdr_vector(xdrs + 1, (char*) board, 64, sizeof(unsigned long), (void*) xdr_u_long);

  xdrrec_endofrecord(xdrs + 1, 1);

  retval = rados_aio_flush(hp.ioctx);
  if (retval == -1) {
    perror("rados_aio_flush");
    return -1;
  }

  xdr_destroy(xdrs + 1);

  rados_ioctx_destroy(&(hp.ioctx));
  
  rados_shutdown(cluster);
  
  return 0;

}

