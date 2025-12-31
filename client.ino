#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Updater.h>
#include <bearssl/bearssl.h>

#define FW_VERSION "1.0.0"

// ================= WIFI CONFIG =================
const char* ssid     = "JioFiber-bF4kK";
const char* password = "kavi7597kavi";

// ================= SERVER CONFIG =================
const char* firmwareUrl = "http://192.168.29.167:5000/firmware";
const char* metaUrl     = "http://192.168.29.167:5000/meta";

// ================= AES CONFIG =================
static const uint8_t AES_KEY[16] = {
  0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
  0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36
};

static const uint8_t AES_IV[16] = {
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
  0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70
};

// ================= JSON HELPER =================
String getJsonValue(const String& json, const String& key) {
  int k = json.indexOf(key);
  if (k < 0) return "";
  int c = json.indexOf(':', k);
  int q1 = json.indexOf('"', c);
  int q2 = json.indexOf('"', q1 + 1);
  return json.substring(q1 + 1, q2);
}

// ================= OTA FUNCTION =================
bool secureOTA() {
  WiFiClient client;
  HTTPClient http;

  Serial.println("[OTA] Checking for update...");

  // ---------- STEP 1: FETCH METADATA ----------
  http.begin(client, metaUrl);
  if (http.GET() != HTTP_CODE_OK) {
    Serial.println("[OTA] Meta fetch failed");
    http.end();
    return false;
  }

  String meta = http.getString();
  http.end();

  String serverVersion = getJsonValue(meta, "version");
  size_t firmwareSize  = getJsonValue(meta, "size").toInt();
  String expectedSha   = getJsonValue(meta, "sha256");

  Serial.println("[OTA] Current FW : " FW_VERSION);
  Serial.println("[OTA] Server  FW : " + serverVersion);

  if (serverVersion == FW_VERSION) {
    Serial.println("[OTA] Firmware already up to date");
    return false;
  }

  if (firmwareSize == 0 || expectedSha.length() != 64) {
    Serial.println("[OTA] Invalid metadata");
    return false;
  }

  // ---------- STEP 2: BEGIN OTA ----------
  size_t maxSketchSpace =
    (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

  if (!Update.begin(maxSketchSpace)) {
    Serial.println("[OTA] Update begin failed");
    return false;
  }

  Serial.println("[OTA] Downloading firmware...");

  // ---------- STEP 3: INIT AES + SHA ----------
  br_aes_ct_cbcdec_keys aesCtx;
  br_aes_ct_cbcdec_init(&aesCtx, AES_KEY, 16);

  uint8_t iv[16];
  memcpy(iv, AES_IV, 16);

  br_sha256_context shaCtx;
  br_sha256_init(&shaCtx);

  // ---------- STEP 4: DOWNLOAD + DECRYPT ----------
  http.begin(client, firmwareUrl);
  if (http.GET() != HTTP_CODE_OK) {
    Serial.println("[OTA] Firmware download failed");
    Update.end(false);
    http.end();
    return false;
  }

  int remaining = http.getSize();
  WiFiClient* stream = http.getStreamPtr();

  uint8_t encBlock[16], decBlock[16], lastBlock[16];
  bool hasLast = false;

  while (remaining > 0) {
    if (stream->available() >= 16) {
      stream->readBytes(encBlock, 16);
      remaining -= 16;

      memcpy(lastBlock, encBlock, 16);
      hasLast = true;

      if (remaining > 0) {
        memcpy(decBlock, encBlock, 16);
        br_aes_ct_cbcdec_run(&aesCtx, iv, decBlock, 16);
        br_sha256_update(&shaCtx, decBlock, 16);
        Update.write(decBlock, 16);
      }
    } else {
      delay(5);
    }
  }

  http.end();

  // ---------- STEP 5: FINAL BLOCK ----------
  if (!hasLast) {
    Update.end(false);
    return false;
  }

  memcpy(decBlock, lastBlock, 16);
  br_aes_ct_cbcdec_run(&aesCtx, iv, decBlock, 16);

  uint8_t pad = decBlock[15];
  if (pad == 0 || pad > 16) {
    Serial.println("[OTA] Padding error");
    Update.end(false);
    return false;
  }

  int writeLen = 16 - pad;
  br_sha256_update(&shaCtx, decBlock, writeLen);
  Update.write(decBlock, writeLen);

  // ---------- STEP 6: VERIFY HASH ----------
  uint8_t hash[32];
  br_sha256_out(&shaCtx, hash);

  char localSha[65];
  for (int i = 0; i < 32; i++)
    sprintf(localSha + i * 2, "%02x", hash[i]);
  localSha[64] = 0;

  Serial.println("[OTA] Local  SHA256: " + String(localSha));
  Serial.println("[OTA] Server SHA256: " + expectedSha);

  if (expectedSha != String(localSha)) {
    Serial.println("[OTA] Checksum mismatch");
    Update.end(false);
    return false;
  }

  // ---------- STEP 7: FINALIZE ----------
  if (!Update.end(true)) {   // IMPORTANT FIX
    Serial.println("[OTA] Update finalize failed");
    return false;
  }

  Serial.println("[OTA] Update SUCCESSFUL!");
  Serial.println("[OTA] Rebooting...");
  delay(2000);
  ESP.restart();

  return true;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\nBooting...");
  Serial.println("Firmware Version: " FW_VERSION);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  secureOTA();
}

// ================= LOOP =================
void loop() {
}
