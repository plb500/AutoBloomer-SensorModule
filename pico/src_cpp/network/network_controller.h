#ifndef _NETWORK_CONTROLLER_H_
#define _NETWORK_CONTROLLER_H_

#include "lwip/ip_addr.h"
#include "lwip/err.h"

class NetworkController {
    public:
        struct DNSRequest {
            const char *mHost;
            ip_addr_t mResolvedAddress;
            err_t mStatus;
        };

        bool isConnected();
        void resolveHost(DNSRequest &request);
        int connectToWiFi(const char * const ssid, const char * const password, const char * const hostname);

        void ledOn();
        void ledOff();

        static void ipAddressToString(char *dest, ip_addr_t *address);

    private:
        int init_wifi(
            uint32_t country, 
            const char *ssid, 
            const char *password,
            uint32_t authType, 
            const char *hostname
        );
};

#endif
