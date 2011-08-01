
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "WiServer.h"
#include "WiShield.h"

#ifdef UIP_DNS
#ifdef ENABLE_DNS_CLIENT

#define MAX_CONCURRENT_QUERIES  10

// Piggyback off of WiServer's verbose logging variable
extern boolean verbose;

typedef struct {
  const char *name;
  dns_client_resolve_callback callback;
  void *data;

} dns_client_query;


dns_client_query dns_client_queries[MAX_CONCURRENT_QUERIES];

void dns_client_init(u16_t *server) {
  uip_dns_conf(server);
}


void dns_client_resolve(char *name, dns_client_resolve_callback callback, void *data) {

  unsigned int i;

  if (!name)
    return;

  for (i = 0; i < MAX_CONCURRENT_QUERIES; i++) {
    if (!dns_client_queries[i].name)
      break;
  }

  if (i == MAX_CONCURRENT_QUERIES) {
    // too many pending queries
    callback(name, NULL, data);
    return;
  }

#ifdef DEBUG
  if (verbose) {
    Serial.print("will resolve: ");
    Serial.print(name);
    Serial.print(" in position: ");
    Serial.println(i);
  }
#endif // DEBUG

  dns_client_queries[i].name = name;
  dns_client_queries[i].callback = callback;
  dns_client_queries[i].data = data;

  uip_dns_query(name);
}

void dns_client_ip_resolved(unsigned int index, u16_t *ipaddr) {

#ifdef DEBUG
  if (verbose) {
    Serial.print("Resolved ");
    Serial.println(dns_client_queries[index].name);
    uint8 dnsAddr[] = {0,0,0,0};
    dnsAddr[0] = uip_ipaddr1(ipaddr);
    dnsAddr[1] = uip_ipaddr2(ipaddr);
    dnsAddr[2] = uip_ipaddr3(ipaddr);
    dnsAddr[3] = uip_ipaddr4(ipaddr);
    Serial.print(dnsAddr[0], DEC);
    Serial.print(".");
    Serial.print(dnsAddr[1], DEC);
    Serial.print(".");
    Serial.print(dnsAddr[2], DEC);
    Serial.print(".");
    Serial.println(dnsAddr[3], DEC);
  }
#endif // DEBUG

  dns_client_queries[index].callback(dns_client_queries[index].name, ipaddr, dns_client_queries[index].data);
  dns_client_queries[index].name = NULL;
  dns_client_queries[index].callback = NULL;
}

extern "C" {


void uip_dns_callback(char *name, u16_t *ipaddr) {

  unsigned int i;
  for (i = 0; i < MAX_CONCURRENT_QUERIES; i++) {
    if (strcmp(name, dns_client_queries[i].name) == 0) {
      dns_client_ip_resolved(i, ipaddr);
      break;
    }
  }

  if (i == MAX_CONCURRENT_QUERIES) {
    if (verbose) {
      Serial.print("got an unexpected DNS response: ");
      Serial.println(name);
    }
  }

  for (i = 0; i < MAX_CONCURRENT_QUERIES; i++) {
    if (dns_client_queries[i].name != NULL)
      break;
  }
  
  // shut down the dns channel if no more pending queries
  if (i == MAX_CONCURRENT_QUERIES) {
    if (verbose)
      Serial.println("No outstanding DNS queries, shutting down");
    uip_dns_shutdown();
  }

}


} // extern "C"



#endif // ENABLE_DNS_CLIENT
#endif // UIP_DNS
