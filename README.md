# Smart-Sunlight-Regulator
### A system that regulates the sunlight exposure of a smart home by controlling the window blinds.

The Smart Sunlight Regulator is a system designed to automate the opening and closing the blinds of a persons home. The system provides manual adjustment by using a servo motor for controlling the blinds and a rotary encoder to precisely adjust the blinds angle manually. Additionally, by using a RTC module the system provides convenient automation by time - based schedules for opening and closing of the blinds. The system as a simple and easy to use physical interface, which includes a small OLED display, two buttons and a rotary encoder.


## Components used

- Arduino Uno R3.
-	Mini OLED display 0.96’’ 128x64 I2C
-	Rotary Encoder KY - 040
-	2x TS04 - 66 - 50 - BK - 260 - SMT Tactile Buttons
-	ZS - 042 DS3231 RTC module
-	TS90M Micro Servo
-	Breadboard
-	Jumper cables


## Software, Tools & Languages

-	Programming Language: C++ (Arduino Framework)
-	Development Environment: VS Code with PlatformIO IDE extension
-	Simulation Tool: Wokwi Simulator


## Libraries used

-	Adafruit_SSD1306  -  Display
-	Wire  -  I2C setup
-	Servo  -  Servo Motor
-	RTClib  -  Clock module
