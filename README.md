# module_tty

[![Build and Publish Debian Package](https://github.com/robotkernel-hal/module_tty/actions/workflows/build-deb.yaml/badge.svg)](https://github.com/robotkernel-hal/module_tty/actions/workflows/build-deb.yaml)
[![License: LGPL-V3](https://img.shields.io/badge/license-LGPL--V3-green.svg)](LICENSE)
[![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)](#)
[![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=fff)](#)
[![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?logo=ubuntu&logoColor=white)](#)

**Robotkernel handler module for serial (TTY) port integration**

This module enables robotkernel HAL to interact with standard serial (TTY) devices on UNIX-like systems. It provides a handler interface that integrates seamlessly with robotkernel's module processing model.

---

## 🧩 Configuration

Configure the module within your robotkernel handler setup:

`main.rkc` 
```yaml
name: my_tty_handler
so_file: libmodule_tty.so
config: !include tty_0
```

This is a complete template for module_tty configuration:

`tty_0.rkc`
```yaml
# Configuration file for module_tty.
#
# vim: ft=yaml

#########################################################
# tty settings

# Serial interface device name
ifname: /dev/ttyUSB0

# Baudrate to be used.
baudrate: 115200

# Number of data bits.
#character_size: 8

# Number of stop bits.
#stopbits: 1

# Parity control.
# Values can be "off", "even" or "odd"
#parity: off

# Read timeout in microseconds.
timeout_us: 0

# Enable/Disable hardware flow controler.
#hardware_flow_control: false

# Disable setting baudrate from module.
#no_baudrate: false

# Ignore modem control lines.
#use_clocal: true

# Execute/Disable script after opening tty device.
#post_open_script: ""

# Enalbe/Disable asynchronous low latency.
#async_low_latency: true

# Configure interface in RS485 mode
#configure_rs485: false

#########################################################
# logging settings

# Standard robotkernel module local loglevel.
#loglevel: verbose

```

| Parameter               | Description                                      |
|-------------------------|--------------------------------------------------|
| `ifname`                | Path to the TTY device (e.g. `/dev/ttyUSB0`)     |
| `baudrate`              | Communication speed (e.g. `9600`, `115200`)      |
| `character_size`        | Number of bits per character (e.g. `7`, `8`)     |
| `stopbits`              | Number of stop bits (`1` or `2`)                 |
| `parity`                | `'none'`, `'even'`, or `'odd'`                   |
| `timeout_us`            | Read timeout in [us].                            |
| `hardware_flow_control` | `'none'`, `'rtscts'`, or `'xonxoff'`             |
| `no_baudrate`           | Do not set any baudrate on device.               |
| `use_clocal`            | Ignore modem control lines                       |
| `post_open_script`      | Shellscript to run after device was opened.      |
| `async_low_latency`     | Asynchronous low latency.                        |
| `configure_rs485`       | Configure device for RS485 mode.                 |

---

## ✨ Features

- Abstracts POSIX TTY interfaces (e.g. `/dev/ttyS*`, `/dev/ttyUSB*`)
- Asynchronous I/O via robotkernel data streams.
- Configurable baud rate, parity, stop bits, flow control, etc.
- Modular design for custom read/write message handling

---

## ⚙️ Runtime Behavior

- Hooks into the HAL execution loop
- Uses blocking or non-blocking I/O for real-time compatibility
- Emits incoming data as messages to downstream modules
- Retrieves queued messages from other modules and sends them out

---

## 🔧 Example Output

```text
TODO
```

---

## 🖼️ Architecture Diagram

```text
+------------------+        +------------------+
|  Robotkernel HAL | <----> |  module_tty.so   |
+------------------+        +--------+---------+
                                      |
                                      v
                              +---------------+
                              | /dev/ttyUSB0  |
                              +---------------+
```

---

## 📦 Build & Installation

Please make sure that the prerequisites are installed. These are:

    robotkernel

Then you should be able to build module_tty with:

```bash
git clone https://github.com/robotkernel-hal/module_tty.git
cd module_tty
./bootstrap.sh
autoreconf -i -f
mkdir build && cd build
../configure
make
sudo make install
```

---

## 🧪 Testing

```bash
TODO
```

Use `socat` or virtual PTYs to simulate devices if physical serial ports are unavailable:

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

---

## 🤝 Contributing

Contributions welcome! Please ensure:

- No compiler warnings
- Code follows robotkernel coding rules
- Tests included for new features

---

## 📄 License

Licensed under the **LGPL-V3 License**. See the [LICENSE](LICENSE) file.

---

**Robotkernel HAL Project** – powering real-time robotics infrastructure with modular, modern C++

