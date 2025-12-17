#include <cstring>
#define MQTTClient_QOS2 1

#include "GUI.h"
#include "cy8ckit_028_tft.h"
#include "mbed.h"
#include "MFRC522.h"
#include <MQTTClientMbedOs.h>
#include <cstdint>

#include "thing_name.h"

#define RFID_TOPIC "rfid/card"

MFRC522 rfid(P8_0, P8_1, P8_2, P8_3, P8_4);

WiFiInterface *wifi;
DigitalOut statusLed(LED1);
DigitalOut cardDetectedLed(LED2);
DigitalOut lightsR(LED3);
DigitalOut lightsG(LED4);
DigitalOut lightsB(LED5);

#define LEDON 0
#define LEDOFF 1

void Display_Init(void) {
    GUI_Init();
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("RFID Reader System", 160, 0);
}

void Display_ShowStatus(const char* status) {
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("                                        ", 0, 40);
    GUI_DispStringAt(status, 0, 40);
}

void Display_ShowCard(const char* uid, const char* cardType) {
    char buffer[64];
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_GREEN);
    
    GUI_DispStringAt("                                        ", 0, 80);
    sprintf(buffer, "UID: %s", uid);
    GUI_DispStringAt(buffer, 0, 80);
    
    GUI_DispStringAt("                                        ", 0, 100);
    GUI_DispStringAt(cardType, 0, 100);
}

void Display_ShowMQTT(const char* status) {
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetColor(GUI_YELLOW);
    GUI_DispStringAt("                                        ", 0, 140);
    GUI_DispStringAt(status, 0, 140);
}

void Display_ShowCount(uint32_t count) {
    char buffer[32];
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_CYAN);
    sprintf(buffer, "Cards scanned: %lu", count);
    GUI_DispStringAt("                                        ", 0, 180);
    GUI_DispStringAt(buffer, 0, 180);
}

void printHex(uint8_t *buffer, uint8_t bufferSize) {
    for (uint8_t i = 0; i < bufferSize; i++) {
        printf("%02X", buffer[i]);
        if (i < bufferSize - 1) printf(":");
    }
}

void getUidString(uint8_t *buffer, uint8_t bufferSize, char *uidString) {
    uidString[0] = '\0';
    char temp[4];
    for (uint8_t i = 0; i < bufferSize; i++) {
        sprintf(temp, "%02X", buffer[i]);
        strcat(uidString, temp);
    }
}

const char *sec2str(nsapi_security_t sec) {
    switch (sec) {
        case NSAPI_SECURITY_NONE:     return "None";
        case NSAPI_SECURITY_WEP:      return "WEP";
        case NSAPI_SECURITY_WPA:      return "WPA";
        case NSAPI_SECURITY_WPA2:     return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2: return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:                      return "Unknown";
    }
}

int scan_demo(WiFiInterface *wifi) {
    WiFiAccessPoint *ap;
    printf("Scanning for WiFi networks...\n");
    
    int count = wifi->scan(NULL, 0);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }
    
    count = count < 15 ? count : 15;
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        delete[] ap;
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s RSSI: %hhd Ch: %hhd\n",
               ap[i].get_ssid(), sec2str(ap[i].get_security()),
               ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);
    
    delete[] ap;
    return count;
}

int main() {
    printf("\n=== MFRC522 RFID Reader with Display ===\n\n");
    
    Display_Init();
    Display_ShowStatus("Initializing...");
    
    lightsR = LEDOFF;
    lightsG = LEDOFF;
    lightsB = LEDOFF;
    
    rfid.PCD_Init();
    printf("RFID Reader initialized\n");
    Display_ShowStatus("RFID initialized");
    
    uint8_t version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
    printf("MFRC522 Firmware Version: 0x%02X\n", version);
    
    if (version == 0x00 || version == 0xFF) {
        printf("WARNING: Communication failure - check wiring!\n");
        Display_ShowStatus("RFID ERROR - Check wiring!");
        lightsR = LEDON;
    }
    
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        Display_ShowStatus("No WiFi interface!");
        return -1;
    }
    
    Display_ShowStatus("Scanning WiFi...");
    int count = scan_demo(wifi);
    
    char statusBuffer[64];
    sprintf(statusBuffer, "Connecting to %s...", MBED_CONF_APP_WIFI_SSID);
    Display_ShowStatus(statusBuffer);
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    
    if (ret != 0) {
        printf("Connection error: %d\n", ret);
        Display_ShowStatus("WiFi connection failed!");
        lightsR = LEDON;
    } else {
        printf("WiFi connected successfully!\n");
        printf("MAC: %s\n", wifi->get_mac_address());
        printf("IP: %s\n", wifi->get_ip_address());
        Display_ShowStatus("WiFi connected!");
        lightsG = LEDON;
    }
    
    TCPSocket socket;
    MQTTClient client(&socket);
    bool mqttConnected = false;
    
    if (ret == 0) {
        socket.open(wifi);
        Display_ShowMQTT("Connecting to MQTT...");
        int rc = socket.connect(MQTT_BROKER, 1883);
        
        if (rc == 0) {
            printf("Socket connected to MQTT broker %s\n", MQTT_BROKER);
            
            MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
            data.clientID.cstring = (char *)THING_NAME;
            data.keepAliveInterval = 20;
            data.cleansession = 1;
            
            rc = client.connect(data);
            if (rc == 0) {
                printf("MQTT client connected as %s\n", THING_NAME);
                mqttConnected = true;
                Display_ShowMQTT("MQTT connected!");
                
                MQTT::Message message;
                memset(&message, 0, sizeof(message));
                char buffer[128];
                sprintf(buffer, "RFID Reader %s online", THING_NAME);
                message.qos = MQTT::QOS0;
                message.retained = false;
                message.dup = false;
                message.payload = (void *)buffer;
                message.payloadlen = strlen(buffer) + 1;
                client.publish(ANNOUNCE_TOPIC, message);
            } else {
                printf("MQTT connection failed: %d\n", rc);
                Display_ShowMQTT("MQTT failed!");
            }
        } else {
            printf("Socket connection failed: %d\n", rc);
            Display_ShowMQTT("Socket failed!");
        }
    }
    
    wait_us(1000000);
    
    GUI_Clear();
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("RFID Reader Ready", 160, 0);
    GUI_SetTextAlign(GUI_TA_LEFT);
    Display_ShowStatus("Waiting for card...");
    Display_ShowCount(0);
    
    printf("\n=== Ready to scan RFID cards ===\n");
    printf("Place a card near the reader...\n\n");
    
    char lastUid[32] = "";
    uint32_t cardCount = 0;
    
    while (1) {
        statusLed = !statusLed;
        
        if (!rfid.PICC_IsNewCardPresent()) {
            wait_us(100000);
            continue;
        }
        
        if (!rfid.PICC_ReadCardSerial()) {
            wait_us(100000);
            continue;
        }
        
        cardDetectedLed = 1;
        lightsB = LEDON;
        cardCount++;
        
        char uidString[32];
        getUidString(rfid.uid.uidByte, rfid.uid.size, uidString);
        
        if (strcmp(uidString, lastUid) != 0) {
            printf("\n--- Card #%lu Detected ---\n", cardCount);
            printf("UID: ");
            printHex(rfid.uid.uidByte, rfid.uid.size);
            printf("\n");
            printf("UID String: %s\n", uidString);
            printf("UID Size: %d bytes\n", rfid.uid.size);
            printf("SAK: 0x%02X\n", rfid.uid.sak);
            
            uint8_t piccType = MFRC522::PICC_GetType(rfid.uid.sak);
            const char* typeName = MFRC522::PICC_GetTypeName(piccType);
            printf("Card Type: %s\n", typeName);
            
            Display_ShowCard(uidString, typeName);
            Display_ShowCount(cardCount);
            Display_ShowStatus("Card detected!");
            
            strcpy(lastUid, uidString);
            
            if (mqttConnected) {
                MQTT::Message message;
                memset(&message, 0, sizeof(message));
                char payload[128];
                sprintf(payload, "{\"uid\":\"%s\",\"type\":\"%s\",\"count\":%lu}", 
                        uidString, typeName, cardCount);
                
                message.qos = MQTT::QOS0;
                message.retained = false;
                message.dup = false;
                message.payload = (void *)payload;
                message.payloadlen = strlen(payload) + 1;
                
                int rc = client.publish(RFID_TOPIC, message);
                if (rc == 0) {
                    printf("Published to MQTT: %s\n", payload);
                    Display_ShowMQTT("Sent to MQTT");
                } else {
                    printf("MQTT publish failed: %d\n", rc);
                    Display_ShowMQTT("MQTT send failed");
                }
            }
        }
        
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        
        wait_us(500000);
        cardDetectedLed = 0;
        lightsB = LEDOFF;
        Display_ShowStatus("Waiting for card...");
        
        if (mqttConnected) {
            client.yield(100);
        }
    }
    
    if (mqttConnected) {
        socket.close();
    }
    if (ret == 0) {
        wifi->disconnect();
    }
    
    printf("\nDone\n");
    return 0;
}
