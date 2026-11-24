# Reception Tools for Windows

## Local dashboard
Open KALIMAN-DASHBOARD.html in Chrome or Edge. Connect the ATS with a USB data cable, wait for startup to finish, and close Schematik's serial monitor. Click the USB connect button and select the ATS port. If opening the port restarts the radio, wait six seconds before reconnecting. Disconnect the dashboard before using Deploy.

## Audio for CW and FT8
Connect the ATS headphone jack to a line input on the PC or a USB audio interface. The ATS USB control connection does not carry audio. Start with low volume and avoid clipping. Do not use an amplified speaker output or a powered microphone input.

## Show decoded text on the dashboard
1. Keep KALIMAN-DASHBOARD.html, KALIMAN-BRIDGE.py and both RECEPTION-TOOLS Markdown files together. Run `py -3 KALIMAN-BRIDGE.py` in PowerShell from that folder. Open http://127.0.0.1:8765 in Chrome or Edge.
2. Select the line input in the decoder. USB controls the ATS; the waterfall is optional.
3. FT8: In WSJT-X, choose the line input, select FT8, set Rig to None, and leave transmit control unconfigured. In Reporting, set UDP Server to 127.0.0.1 and port to 2237. Enable Monitor and synchronize the Windows clock. On the ATS, try 14.074 MHz, USB, SYNC off and BFO initially at zero. The dashboard shows new off-air reception, not WAV playback.
4. CW: Open fldigi in CW mode. On the ATS use USB or LSB with SYNC off and adjust the BFO for a clear tone. Center the decoder on the tone in fldigi. Use `--xmlrpc-server-address 127.0.0.1 --xmlrpc-server-port 7362` when starting fldigi. Do not run another reader of `rx.get_data` at the same time. Do not expose these ports to the Internet.

PSK Reporter and Reverse Beacon Network show third-party reports. POTA shows reported activations and HamQSL provides solar context. None confirms reception on your ATS; check local RSSI/SNR and decoded text.
