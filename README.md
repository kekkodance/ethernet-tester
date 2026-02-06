# ethernet-tester

This firmware turns an ESP8266 into a network cable tester, controlled via a web interface.

## Hardware Requirements

   1 ESP8266 Development Board (NodeMCU, Wemos D1 Mini, etc.).  
   2 RJ45 Breakout Boards or connectors.  
   8 LEDs.  

## Pinout

### Master Unit

Connect the ESP8266 pins to the RJ45 connector according to the T568B standard:

| ESP8266 Pin | RJ45 Pin |
| :--- | :--- |
| **D0** | Pin 1 | 
| **D1** | Pin 2 | 
| **D2** | Pin 3 | 
| **D3** | Pin 4 | 
| **D4** | Pin 5 | 
| **D5** | Pin 6 |
| **D6** | Pin 7 |
| **D7** | Pin 8 |

### Slave Unit

This is a "Remote" that plugs into the other end of the cable, **no microcontroller and no batteries required**. It uses LEDs to visualize the signal sent by the Master.
No resistors were used in my version, if you want to include them then do so (220ohm), but the ESP8266 normally can't supply enough current to blow up an LED.

| LED  | Anode (+) | Cathode (-) |
| :--- | :--- | :--- |
| **LED 1** | RJ45 Pin 1 | RJ45 Pin 2 |
| **LED 2** | RJ45 Pin 2 | RJ45 Pin 3 |
| **LED 3** | RJ45 Pin 3 | RJ45 Pin 4 | 
| **LED 4** | RJ45 Pin 4 | RJ45 Pin 5 |
| **LED 5** | RJ45 Pin 5 | RJ45 Pin 6 | 
| **LED 6** | RJ45 Pin 6 | RJ45 Pin 7 |
| **LED 7** | RJ45 Pin 7 | RJ45 Pin 8 |
| **LED 8** | RJ45 Pin 8 | RJ45 Pin 1 |

*Note: The 8th LED loops back from Pin 8 to Pin 1.*

<img width="404" height="618" alt="{3F6160D6-F3AC-4463-BD64-6E9D47A3395D}" src="https://github.com/user-attachments/assets/03332df9-9ce0-4807-94fe-9a28b2626df0" /> <img width="532" height="418" alt="{0E66BFF7-6549-4014-ADFD-9982A9E5630F}" src="https://github.com/user-attachments/assets/c501627d-054b-4f95-847f-4452f16780ed" />



## Installation and Usage

1.  **Flash:** Upload the provided `.ino` file to your ESP8266 using Arduino IDE.
2.  **Connect:** Power on the device. Connect to the network:
    *   **SSID:** `Tester Ethernet`
    *   **Password:** `ethernet`
3.  **Access:** The captive portal opens automatically. If it doesn't, navigate to `http://192.168.4.1`.
4.  **Test:** Use the dropdown menu to select Auto, Manual, or Continuity.

## Safety Warning

**Do not connect this device to a live switch, router, or PoE (Power over Ethernet) port.**  
The **48V** provided by PoE will **immediately** destroy the ESP8266. Use this device only on disconnected cables.
