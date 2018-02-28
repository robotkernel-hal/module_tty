The module_tty works as a transparent layer for other modules. These can gain abstract access to a tty-hardware interface.

[[File:rk_module_tty_example.png|500px|frameless|center|baseline|upright=4]]

= configuration =

In the configuration the settings for the serial port have to be given. 

 - name: my_tty
   so_file: libmodule_tty.so
   config:
     ifname: /dev/ttyS0
     baudrate: 115200
     timeout_us: 10000
     post_open_script: some_thing_to_do.sh
   power_up: op

; ifname
: specifies the tty interface name to open
; baudrate
: Specifies the baudrate (e.g. 115000 [Baud])
; timeout
: Defined the read timeout on the tty. 
; post_open_script
: If some action has to be done after opening the tty device file (e.g. setting interrupt priorities or affinities) a script name may be given.

[[category:Robotkernel-5|Tty]]
