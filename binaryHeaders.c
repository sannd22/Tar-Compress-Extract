#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>

uint32_t extract_special_int(char *where, int len) {
  /* returns integer on success and -1 on failure */
  int32_t val = -1;
  
  if((len >= sizeof(val)) && (where[0] & 0x80)) {
    val = *(int32_t*)(where+len-sizeof(val));
    /* convert to host byte order */
    val = ntohl(val); 
  }
  
  return val;
}

int insert_special_int(char *where, size_t size, int32_t val) {
  /* returns 0 on success and nonzero otherwise */
  int err = 0;

  if(val < 0 || (size < sizeof(val))) {
    /* checks if negative or if len is too small */
    err++;
  }
  else {
    /* clear buffer */
    memset(where, 0, size);
    /* place the integer */
    *(int32_t*)(where+size-sizeof(val)) = htonl(val);
    /* set high-order bit */
    *where |= 0x80;
  }
  return err;
}
