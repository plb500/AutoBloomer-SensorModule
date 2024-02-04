#include "network_utils.h"

#include "lwip/dns.h"
#include "pico/cyw43_arch.h"

bool is_network_connected() {
    return (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP);
}

void on_resolution_completed(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    DNSResolutionRequest *r = (DNSResolutionRequest *) callback_arg;

    if(ipaddr) {
        r->mResolvedAddress.addr = ipaddr->addr;
        r->mStatus = ERR_OK;
    } else {
        // Something went wrong - either a failed resolution or a timeout, we don't really know which
        r->mStatus = ERR_ARG;
    }
}

void resolve_host_blocking(DNSResolutionRequest *request) {
    if(!request) {
        return;
    }

    cyw43_arch_lwip_begin();
    request->mStatus = dns_gethostbyname(request->mHost, &request->mResolvedAddress, on_resolution_completed, request);
    cyw43_arch_lwip_end();

    if(request->mStatus == ERR_ARG) {
        return;
    }

    if(request->mStatus == ERR_OK) {
        // IP was resolved internally (from cache) and stored in request->mResolvedAddress, nothing else required
        return;
    }

    while((request->mStatus != ERR_OK) && (request->mStatus != ERR_ARG)) {
        sleep_ms(1);
    }
}

int init_wifi(
    uint32_t country, 
    const char *ssid, 
    const char *password,
    uint32_t authType, 
    const char *hostname
) {
    // Clean up any previous connections
    if(cyw43_arch_async_context()) {
        cyw43_arch_disable_sta_mode();
        cyw43_arch_deinit();
    }

    if(cyw43_arch_init_with_country(country)) {
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if(hostname != NULL){
        netif_set_hostname(netif_default, hostname);
    }

    return cyw43_arch_wifi_connect_timeout_ms(ssid, password, authType, 20000);
}

int connect_to_wifi(const char * const ssid, const char * const password, const char * const hostname) {
    uint32_t country = CYW43_COUNTRY_CANADA;
    uint32_t auth = CYW43_AUTH_WPA2_MIXED_PSK;
    return init_wifi(country, ssid, password, auth, hostname);
}

void wifi_led_on() {
    if(cyw43_arch_async_context()) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
}

void wifi_led_off() {
    if(cyw43_arch_async_context()) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }
}
