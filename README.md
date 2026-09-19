# std/wol

Construct and send Wake-on-LAN magic packets over IPv4 broadcast.

```doof
import { wake } from "std/wol"

try! wake("aa:bb:cc:dd:ee:ff", "192.168.1.255")
```

`wake` returns a failure if its MAC address, broadcast address, or port is invalid, or if the operating system cannot send the datagram. A successful send only confirms local delivery to the network stack; the target still requires Wake-on-LAN support and an appropriate network path.
