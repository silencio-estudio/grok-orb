#pragma once
#include <stddef.h>
static inline int mbedtls_base64_encode(unsigned char *dst,size_t dlen,size_t *olen,const unsigned char *src,size_t slen){
    (void)dst;(void)dlen;(void)src;(void)slen;*olen=0;return 0;
}
