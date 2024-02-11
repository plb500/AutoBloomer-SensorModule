#ifndef _NETWORK_UTILS_H_
#define _NETWORK_UTILS_H_

#include "pico/types.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"


typedef struct DNSResolutionRequest_t {
    const char *mHost;
    ip_addr_t mResolvedAddress;
    err_t mStatus;
} DNSResolutionRequest;


void ip_address_to_string(char *dest, ip_addr_t *address);
bool is_network_connected();
void resolve_host_blocking(DNSResolutionRequest *request);
int connect_to_wifi(const char * const ssid, const char * const password, const char * const hostname);

void wifi_led_on();
void wifi_led_off();

#endif      // _NETWORK_UTILS_H_
