# SmartRGBv2-Audio
Reads your computer audio, and streams color data to the SmartRGBv2 controller.

# Usage
`./main.exe [<server-address>] [<server-port>] [<packet-rate>] [<verbose>]`  

| Argument           | Default value   | Description                                             |
|:-------------------|:----------------|:--------------------------------------------------------|
| `<server-address>` | `192.168.0.100` | The SmartRGBvv2 controller's ip address.                |
| `<server-port>`    | `555`           | The SmartRGBvv2 controller's ip port.                   |
| `<packet-rate>`    | `50`            | The goal packet rate (in packets per second).           |
| `<verbose>`        | `false`         | Whether to print out average packet rates. (`true`/`t`) |
