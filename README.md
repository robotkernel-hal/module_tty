The **module_tty** works as a transparent layer for other modules. It gains abstract access to a tty-hardware interface.

## Example config file

This example config file can be used as a template for own configurations.

```
# Configuration file for module_tty.
#
# vim: ft=yaml

#########################################################
# tty settings

# Serial interface device name
#ifname: /dev/ttyUSB0

# Baudrate to be used.
#baudrate: 0

# Number of data bits.
#character_size: 8

# Number of stop bits.
#stopbits: 1

# Parity control.
# Values can be "off", "even" or "odd"
#parity: off

# Read timeout in microseconds.
#timeout_us: 0

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

#########################################################
# logging settings

# Standard robotkernel module local loglevel.
#loglevel: verbose

```
