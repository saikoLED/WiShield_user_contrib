
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "WiServer.h"
#include "WiShield.h"

#ifdef UIP_DHCP
#ifdef ENABLE_DHCP_CLIENT

// Piggyback off of WiServer's verbose logging variable
extern boolean verbose;

// State of the configuration
typedef enum {
  DCS_CONFIGURING = 0,        // we're attempting to configure
  DCS_SUCCEEDED,              // it worked
  DCS_FAILED                  // it failed
} DHCP_CLIENT_STATE;

DHCP_CLIENT_STATE g_dhcpClientState;


bool dhcp_client_configure_network() {

#ifdef DEBUG
  if (verbose)
    Serial.println("Making DHCP query...");
#endif // DEBUG

  g_dhcpClientState = DCS_CONFIGURING;
  uip_dhcp_request();

  while (g_dhcpClientState == DCS_CONFIGURING) {
    WiFi.run();
  }

  return g_dhcpClientState == DCS_SUCCEEDED;
}


extern "C" {

void uip_dhcp_callback(const struct dhcp_state *s) {
  if (!s) {
    if (verbose)
      Serial.println("DHCP failed");

    g_dhcpClientState = DCS_FAILED;
    uip_dhcp_shutdown();
    return;
  }

  // Set the received IP addr data into the uIP stack
  uip_sethostaddr(s->ipaddr);
  uip_setdraddr(s->default_router);
  uip_setnetmask(s->netmask);

#ifdef DEBUG
  if (verbose) {
    // Print the received data - its quick and dirty but informative
    Serial.print("DHCP IP     : "); 
    Serial.print(uip_ipaddr1(s->ipaddr), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr2(s->ipaddr), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr3(s->ipaddr), DEC);
    Serial.print(".");
    Serial.println(uip_ipaddr4(s->ipaddr), DEC);

    Serial.print("DHCP GATEWAY: "); 
    Serial.print(uip_ipaddr1(s->default_router), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr2(s->default_router), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr3(s->default_router), DEC);
    Serial.print(".");
    Serial.println(uip_ipaddr4(s->default_router), DEC);

    Serial.print("DHCP NETMASK: "); 
    Serial.print(uip_ipaddr1(s->netmask), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr2(s->netmask), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr3(s->netmask), DEC);
    Serial.print(".");
    Serial.println(uip_ipaddr4(s->netmask), DEC);

    Serial.print("DHCP DNS    : "); 
    Serial.print(uip_ipaddr1(s->dnsaddr), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr2(s->dnsaddr), DEC);
    Serial.print(".");
    Serial.print(uip_ipaddr3(s->dnsaddr), DEC);
    Serial.print(".");
    Serial.println(uip_ipaddr4(s->dnsaddr), DEC);
  }
#endif // DEBUG

  // Shut down DHCP
  uip_dhcp_shutdown();

  g_dhcpClientState = DCS_SUCCEEDED;
}


} // extern "C"


#endif // ENABLE_DHCP_CLIENT
#endif // UIP_DHCP
