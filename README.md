# power-monitor
Power Monitor code for the Winter Project 2025 for ES1050 @ UWO
Coded by Ryan Jin for the ES1050 Winter Project "Power" for the scholastic term 2024-2025.
Utilizing the GxEPD2 library Jean-Marc Zingg (the G.O.A.T.)
This program is NOT a final product, nor is it to be assumed to be so.

Warnings and limitations will be discussed:
1. The voltage readings calculated by the code are taken from a voltage divider that does NOT denoise using capacitors. This means there is an linear function in terms of error and voltage. (i.e. margin of error increases when voltage increases, albeit not too much)

2. While this project does expect measurements for long periods of time, I have taken the liberty to treat this program as it is- a prototype test program. That is to say, the time keeping function is using the built in millis(), which WILL overflow back to zero in ~50 days after the code begins to run on the microcontroller. This means that the recommended run time for this program is only the amount of time we have run it during our showcase on April 1st, which I estimate to be ~1 hr. lol.

3. I have not figured out how to save data in a 2D array yet. You can actually see the method commented out, as well as the 2D array I inteded to use as the method of saving data commented out as well.

4. In terms of power consumption, the current build is already pretty energy efficient. However, it can be made even more efficient, by decreasing the CPU frequency to 80MHz, entering the ESP32's deep sleep function, and using interrupts rather than constantly checking for button inputs. Disabling unused functions, such as WiFi and Bluetooth (when not being used) and extra GPIO pins.

5. Probably could save a shit ton of storage by removing some of these since a few of these are definitely redundant.
