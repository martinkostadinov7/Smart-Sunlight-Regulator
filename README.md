# Smart Sunlight Regulator
**Author:** Martin Kostadinov

## 1. Project Overview
The **Smart Sunlight Regulator** is an automated system designed to control window blinds. It offers both manual position adjustment using a rotary encoder and automated schedule-based control via a Real-Time Clock (RTC) module. A physical UI consisting of an OLED display and two buttons allows direct control without requiring a smartphone app.

## 2. Hardware Components & Wiring
* **Arduino Uno R3:** Central processing unit.
* **0.96" OLED Display (SSD1306):** Displays status and time (I2C: A4/A5).
* **DS1307 RTC Module:** Tracks real-time schedules (I2C: A4/A5).
* **KY-040 Rotary Encoder:** Relative rotation input for blind angle and time adjustment (CLK: D2, DT: D3).
* **SG90 Micro Servo:** Drives the blind mechanism (PWM: D5).
* **2x Push Buttons:** Button 1 (D12) sets opening time/saves; Button 2 (D13) sets closing time/cancels. External 10kΩ pull-down resistors are used.

| Component | Arduino Pin | Notes |
| :--- | :--- | :--- |
| **OLED & RTC** | A4 (SDA), A5 (SCL), 5V, GND | I2C Bus Communication |
| **Rotary Encoder** | D2 (CLK), D3 (DT), 5V, GND | Infinite relative rotation |
| **Servo Motor** | D5 (PWM), 5V, GND | Adjusts blind angle (0°–180°) |
| **Button 1 (Green)**| D12, 5V, 10kΩ to GND | Open Schedule / Save |
| **Button 2 (Red)**  | D13, 5V, 10kΩ to GND | Close Schedule / Cancel |

## 3. Key Software Logic

### Encoder Reading & Time Mapping
The rotary encoder eliminates position jumps when switching modes. The 24-hour day is split into 144 steps of 10-minute intervals:

```cpp
struct TimeOfDay {
  uint8_t hour;
  uint8_t minute;
};

TimeOfDay convertValueToTime(int value) {
  return TimeOfDay{(uint8_t)(value / 6), (uint8_t)((value % 6) * 10)};
}

int readEncoder() {
  int newClk = digitalRead(ENCODER_CLK);
  if (newClk != lastClk) {
    lastClk = newClk;
    int dtValue = digitalRead(ENCODER_DT);
    if (newClk == LOW && dtValue == HIGH) return 1;
    if (newClk == LOW && dtValue == LOW) return -1;
  }
  return 0;
}
