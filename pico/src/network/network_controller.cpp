#include "network_controller.h"

#include "lwip/dns.h"
#include "pico/cyw43_arch.h"

// DNS resolution callbacl
void on_resolution_completed(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    NetworkController::DNSRequest *r = (NetworkController::DNSRequest *) callback_arg;

    if(ipaddr) {
        r->mResolvedAddress.addr = ipaddr->addr;
        r->mStatus = ERR_OK;
    } else {
        // Something went wrong - either a failed resolution or a timeout, we don't really know which
        r->mStatus = ERR_ARG;
    }
}

bool NetworkController::isConnected() {
    return (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP);
}

void NetworkController::resolveHost(DNSRequest &request) {
    cyw43_arch_lwip_begin();
    request.mStatus = dns_gethostbyname(request.mHost, &request.mResolvedAddress, on_resolution_completed, &request);
    cyw43_arch_lwip_end();

    if(request.mStatus == ERR_ARG) {
        return;
    }

    if(request.mStatus == ERR_OK) {
        // IP was resolved internally (from cache) and stored in request->mResolvedAddress, nothing else required
        return;
    }

    while((request.mStatus != ERR_OK) && (request.mStatus != ERR_ARG)) {
        sleep_ms(1);
    }
}

int NetworkController::connectToWiFi(const char * const ssid, const char * const password, const char * const hostname) {
    uint32_t country = CYW43_COUNTRY_CANADA;
    uint32_t auth = CYW43_AUTH_WPA2_MIXED_PSK;
    return init_wifi(country, ssid, password, auth, hostname);
}

void NetworkController::ledOn() {
    if(cyw43_arch_async_context()) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
}

void NetworkController::ledOff() {
    if(cyw43_arch_async_context()) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }
}

void NetworkController::ipAddressToString(char *dest, ip_addr_t *address) {
    if(!dest || !address) {
        return;
    }

    sprintf(
        dest,
        "%d.%d.%d.%d",
        (address->addr & 0x000000FF),
        ((address->addr & 0x0000FF00) >> 8),
        ((address->addr & 0x00FF0000) >> 16),
        ((address->addr & 0xFF000000) >> 24)
    );
}

int NetworkController::init_wifi(
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
