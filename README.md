# SecureAES-OTA
SecureAES-OTA is a secure OTA firmware update system for ESP8266 that enables wireless firmware upgrades over HTTP using AES-128 encryption. Firmware integrity is verified using SHA-256 before safely flashing and rebooting into the updated firmware, ensuring reliable and secure updates for IoT devices.
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/4bf377a4-75c3-4921-a66d-1d65e4316d1c" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/062acbcb-3065-4bc4-9783-6312d7b95a07" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/b63e0451-2fc1-4066-824e-d8494817ef8b" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/14485e84-571e-40b2-aef3-994cdc417e65" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/3a834fa7-32e5-4b3d-8a03-26ad86e0bd5c" />
# SecureAES-OTA

SecureAES-OTA is a secure over-the-air (OTA) firmware update system for ESP8266 devices. It enables wireless firmware upgrades over HTTP using **AES-128-CBC** encrypted firmware binaries and verifies firmware integrity using **SHA-256** before safely flashing the update.

---

## 📌 Features

* **Wireless firmware updates** over HTTP
* **AES-128-CBC** encrypted firmware transmission
* **On-device firmware decryption** using BearSSL
* **SHA-256 integrity verification** before flashing
* **Safe flash update** using the ESP8266 Update API
* **Automatic reboot** after successful update
* **Safe Aborts**: Update cancels automatically on failure or checksum mismatch

---

## 🛠️ Technologies Used

### Device Side
* **Hardware**: ESP8266
* **Environment**: Arduino IDE / ESP8266 Arduino Core
* **Libraries**: BearSSL (AES + SHA-256), ESP8266 Update Library

### Server Side
* **Language**: Python
* **Framework**: Flask
* **Security**: PyCryptodome (AES encryption)

---

## 🧩 System Architecture



1.  **Connection**: ESP8266 connects to Wi-Fi.
2.  **Metadata Fetch**: Device fetches metadata from the `/meta` endpoint.
    * Contains: Firmware version, original firmware size, and SHA-256 checksum.
3.  **Update Check**: If a newer version is available:
    * The encrypted firmware is downloaded from `/firmware`.
    * Firmware is decrypted block-by-block using **AES-128-CBC**.
    * A **SHA-256 hash** is calculated on-the-fly during the flashing process.
4.  **Verification**: The calculated hash is verified against the server-provided value.
5.  **Finalization**: Firmware is finalized, and the device reboots into the new version.

---

## 🔐 Security Design

* **Confidentiality**: Firmware is AES-128-CBC encrypted on the server, ensuring the binary cannot be intercepted or reverse-engineered during transit.
* **Integrity**: A SHA-256 checksum ensures the firmware hasn't been corrupted or tampered with.
* **Safety**: The update process is atomic and will abort if:
    * Metadata is invalid.
    * Decryption fails.
    * Hash verification fails.
    * Flash write fails.
