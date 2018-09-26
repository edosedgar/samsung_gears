# Samsung Gears S3 mini project

The repo contains the project written in Tizen IDE. Native mode of operating
mode is used here. Smartwatch keeps track of heart rate and acceleration and
trasnmit JSON-formatted message to server in the same local network.
Smartwatch connects to network via Wi-Fi.

The structure of repo is the following:
1) GameGate - TizenIDE project.
Note: change the server address in inc/net.h (HOST_ADDR define)
2) analyze - Python script (for server)
Note: specify your network inteface on 65-66 lines.
