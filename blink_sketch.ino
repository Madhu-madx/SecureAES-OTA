void setup() {
  // Most ESP8266 boards have a blue LED on GPIO 2 (LED_BUILTIN)
  pinMode(LED_BUILTIN, OUTPUT); 
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);  // LED ON (Most ESPs are active-low)
  delay(200);                      // Fast blink
  digitalWrite(LED_BUILTIN, HIGH); // LED OFF
  delay(200);
}