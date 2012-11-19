#ifndef RF212_LWIP_ETHERNETIF_H_
#define RF212_LWIP_ETHERNETIF_H_
#include "lwip/netif.h"
err_t ethernetif_init(struct netif *);
void  ethernetif_input(struct netif *);

struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int (*recv)(uint8_t len, uint8_t* data);
  int (*send)(uint8_t len, uint8_t* data);
};

#endif // XV6_LWIP_ETHERNETIF_H_

