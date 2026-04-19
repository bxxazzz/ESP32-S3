// typedef enum {
//     WIFI_EVENT_WIFI_READY = 0,           /**< Wi-Fi ready */
//     WIFI_EVENT_SCAN_DONE,                /**< Finished scanning AP */
//     WIFI_EVENT_STA_START,                /**< Station start */
//     WIFI_EVENT_STA_STOP,                 /**< Station stop */
//     WIFI_EVENT_STA_CONNECTED,            /**< Station connected to AP */
//     WIFI_EVENT_STA_DISCONNECTED,         /**< Station disconnected from AP */
//     WIFI_EVENT_STA_AUTHMODE_CHANGE,      /**< The auth mode of AP connected by device's station changed */

//     WIFI_EVENT_STA_WPS_ER_SUCCESS,       /**< Station WPS succeeds in enrollee mode */
//     WIFI_EVENT_STA_WPS_ER_FAILED,        /**< Station WPS fails in enrollee mode */
//     WIFI_EVENT_STA_WPS_ER_TIMEOUT,       /**< Station WPS timeout in enrollee mode */
//     WIFI_EVENT_STA_WPS_ER_PIN,           /**< Station WPS pin code in enrollee mode */
//     WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP,   /**< Station WPS overlap in enrollee mode */

//     WIFI_EVENT_AP_START,                 /**< Soft-AP start */
//     WIFI_EVENT_AP_STOP,                  /**< Soft-AP stop */
//     WIFI_EVENT_AP_STACONNECTED,          /**< A station connected to Soft-AP */
//     WIFI_EVENT_AP_STADISCONNECTED,       /**< A station disconnected from Soft-AP */
//     WIFI_EVENT_AP_PROBEREQRECVED,        /**< Receive probe request packet in soft-AP interface */

//     WIFI_EVENT_FTM_REPORT,               /**< Receive report of FTM procedure */

//     /* Add next events after this only */
//     WIFI_EVENT_STA_BSS_RSSI_LOW,         /**< AP's RSSI crossed configured threshold */
//     WIFI_EVENT_ACTION_TX_STATUS,         /**< Status indication of Action Tx operation */
//     WIFI_EVENT_ROC_DONE,                 /**< Remain-on-Channel operation complete */

//     WIFI_EVENT_STA_BEACON_TIMEOUT,       /**< Station beacon timeout */

//     WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START,   /**< Connectionless module wake interval start */
//     /* Add next events after this only */

//     WIFI_EVENT_AP_WPS_RG_SUCCESS,       /**< Soft-AP wps succeeds in registrar mode */
//     WIFI_EVENT_AP_WPS_RG_FAILED,        /**< Soft-AP wps fails in registrar mode */
//     WIFI_EVENT_AP_WPS_RG_TIMEOUT,       /**< Soft-AP wps timeout in registrar mode */
//     WIFI_EVENT_AP_WPS_RG_PIN,           /**< Soft-AP wps pin code in registrar mode */
//     WIFI_EVENT_AP_WPS_RG_PBC_OVERLAP,   /**< Soft-AP wps overlap in registrar mode */

//     WIFI_EVENT_ITWT_SETUP,              /**< iTWT setup */
//     WIFI_EVENT_ITWT_TEARDOWN,           /**< iTWT teardown */
//     WIFI_EVENT_ITWT_PROBE,              /**< iTWT probe */
//     WIFI_EVENT_ITWT_SUSPEND,            /**< iTWT suspend */
//     WIFI_EVENT_TWT_WAKEUP,              /**< TWT wakeup */
//     WIFI_EVENT_BTWT_SETUP,              /**< bTWT setup */
//     WIFI_EVENT_BTWT_TEARDOWN,           /**< bTWT teardown*/

//     WIFI_EVENT_NAN_STARTED,              /**< NAN Discovery has started */
//     WIFI_EVENT_NAN_STOPPED,              /**< NAN Discovery has stopped */
//     WIFI_EVENT_NAN_SVC_MATCH,            /**< NAN Service Discovery match found */
//     WIFI_EVENT_NAN_REPLIED,              /**< Replied to a NAN peer with Service Discovery match */
//     WIFI_EVENT_NAN_RECEIVE,              /**< Received a Follow-up message */
//     WIFI_EVENT_NDP_INDICATION,           /**< Received NDP Request from a NAN Peer */
//     WIFI_EVENT_NDP_CONFIRM,              /**< NDP Confirm Indication */
//     WIFI_EVENT_NDP_TERMINATED,           /**< NAN Datapath terminated indication */
//     WIFI_EVENT_HOME_CHANNEL_CHANGE,      /**< Wi-Fi home channel change，doesn't occur when scanning */

//     WIFI_EVENT_STA_NEIGHBOR_REP,         /**< Received Neighbor Report response */

//     WIFI_EVENT_MAX,                      /**< Invalid Wi-Fi event ID */
// } wifi_event_t;

// ESP-IDF에서 WIFI 사용을 위한 필수 라이브러리.
#include "WIFI.h"
#include "esp_wifi.h"           // WIFI 핵심 API.
#include "esp_event.h"          // 특정 이벤트 발생 시 실행될 콜백함수.
#include "nvs_flash.h"          // ESP32는 WIFI 설정 정보를 내부 NVS(비휘발성 저장소) Flash에 저장함.
#include "esp_log.h"            // ESP32 로그 출력용. (printf보다 기능 많음)

// WIFI - SETTING.
#define WIFI_SSID   "iPhone"
#define WIFI_PASS   "12345678"

static const char *TAG = "WIFI";

uint8_t WIFI_STATUS;

// 이벤트 핸들러: Wi-Fi 연결 상태에 따라 호출됨.
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if     (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)          esp_wifi_connect();           // WIFI 연결.
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
         // 끊기면 재연결 시도.
        esp_wifi_connect();                                                                                    
        WIFI_STATUS = WIFI_DISCONNECTED;
    } 
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        //ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        //ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));

        WIFI_STATUS = WIFI_CONNECTED;
    }
}

void WIFI_INIT() 
{
    // 1. NVS 초기화.
    esp_err_t result = nvs_flash_init();
    if(result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }

    ESP_ERROR_CHECK(result);

    // 2. 네트워크 인터페이스 초기화.
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 3. Wi-Fi 초기화 설정.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. 이벤트 핸들러 등록.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // 5. SSID & PW 설정.
    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    // 6. Wi-Fi 시작.
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WIFI INIT OK...");
}









