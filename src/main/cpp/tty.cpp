//! robotkernel module for tty serial devices
/*!
 * author: Robert Burger <robert.burger@dlr.de>
 */

/*
 * This file is part of robotkernel.
 *
 * robotkernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * robotkernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with robotkernel.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tty.h"
#include "robotkernel/helpers.h"
#include "robotkernel/kernel.h"
#include "robotkernel/exceptions.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"

#if HAVE_TERMIOS_H == 1
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#endif

#ifdef __VXWORKS__
#include <ioLib.h>
#include <sioLib.h>
#include <strings.h>
#endif

MODULE_DEF(module_tty, module_tty::tty);

#define SWAP_BYTES(x) ((((x)&0xFF00) >> 8) | (((x)&0x00FF) << 8))

using namespace std;
using namespace robotkernel;
using namespace module_tty;
using namespace string_util;

int tty::decode_baudrate(const int baudrate) const {
#if defined __QNX__ || defined __VXWORKS__
    return baudrate;
#else
    switch (baudrate) {
        case 9600:      return B9600;
        case 19200:     return B19200;
        case 38400:     return B38400;
        case 115200:    return B115200;
        case 230400:    return B230400;
#ifdef  B460800
        case 460800:    return B460800;
#endif
#ifdef  B500000
        case 500000:    return B500000;
#endif
#ifdef  B576000
        case 576000:    return B576000;
#endif
#ifdef  B921600
        case 921600:    return B921600;
#endif
#ifdef  B1000000
        case 1000000:   return B1000000;
#endif
#ifdef  B1152000
        case 1152000:   return B1152000;
#endif
#ifdef  B1500000
        case 1500000:   return B1500000;
#endif
#ifdef  B2000000
        case 2000000:   return B2000000;
#endif
#ifdef  B2500000
        case 2500000:   return B2500000;
#endif
#ifdef  B3000000
        case 3000000:   return B3000000;
#endif
#ifdef  B3500000
        case 3500000:   return B3500000;
#endif
#ifdef  B4000000
        case 4000000:   return B4000000;
#endif
        default:        return -1;
    }
#endif // !defined QNX && !defined VXWORKS
}

//! decodes character size to define which can be used with termios
/*!
 * \param[in] char_size     Charater bit size.
 * \return     Corresponding bit size define.
 */
unsigned tty::decode_character_size(const int char_size) const {
    if (char_size == 5)      return CS5;
    else if (char_size == 6) return CS6;
    else if (char_size == 7) return CS7;
    
    return CS8;
}


//! construction
/*
 * \param name fts name
 * \param node YAML configuration node
 */
tty::tty(const char *name, const YAML::Node& node) :
    module_base("module_tty", name, node),
    serial_stream(name, "tty")
{
    fd                    = -1;
    ifname                = get_as<std::string>(node, "ifname");
    baudrate              = get_as<unsigned>(node, "baudrate", 0);
    char_size             = get_as<unsigned>(node, "character_size", 8);
    stopbits              = get_as<unsigned>(node, "stopbits", 1);
    parity                = get_as<string>  (node, "parity", "off");
    timeout_us            = get_as<unsigned>(node, "timeout_us");
    hardware_flow_control = get_as<bool>    (node, "hardware_flow_control", false);
    no_baudrate           = get_as<bool>    (node, "no_baudrate", false);
    use_clocal            = get_as<bool>    (node, "use_clocal", true);
    post_open             = get_as<string>  (node, "post_open_script", "");
    async_low_latency     = get_as<bool>    (node, "async_low_latency", true);
    configure_rs485       = get_as<bool>    (node, "configure_rs485", false);

    if(!no_baudrate && baudrate == 0)
        throw str_exception("invalid baudrate: %d", baudrate);

    set_state(module_state_init);
}

//! destruction
tty::~tty() {
    // set to init, this will close serial device
    set_state(module_state_init);
}

size_t tty::read(void* buf, size_t bufsize) {
    if (state < module_state_safeop) {
        log(warning, "invalid state for reading data\n");
        // invalid state
        return 0;
    }

    if (timeout_us > 0) {
        while (1) {
            fd_set readset;
            FD_ZERO(&readset);
            FD_SET(fd, &readset);
            timeval timeout = { 0, timeout_us };
            int rc = select(fd + 1, &readset, NULL, NULL, &timeout);
            if (rc == -1) {
                if (errno == EINTR)
                    continue;

                log(verbose, "select returned %s\n", strerror(errno));
                return -1;
            } else if (rc == 0) {
                log(warning, "reading from tty timed out\n");
                return -1;
            }

            break;
        }
    }

    return ::read(fd, buf, bufsize);
}

size_t tty::write(void* buf, size_t bufsize) {
    if (state < module_state_op)
        // invalid state
        return 0;

    return ::write(fd, buf, bufsize);
}

//! set fts state
/*!
 * \param state new fts state
 */
int tty::set_state(module_state_t state) {
    kernel& k = *kernel::get_instance();

    // get transition
    uint32_t transition = GEN_STATE(this->state, state);

    switch (transition) {
        case op_2_safeop:
        case op_2_preop:
        case op_2_init:
            // ====> stop sending commands
            if (    (transition == op_2_safeop))
                break;
        case safeop_2_preop:
        case safeop_2_init:
            // ====> stop receiving measurements
            if (    (transition == op_2_preop) ||
                    (transition == safeop_2_preop))
                break;
        case preop_2_init:
            // ====> deinit devices
            
            // remove stream device
            k.remove_device(static_pointer_cast<stream>(shared_from_this()));

            close_port();
        case init_2_init:
            // ====> do nothing
            break;

        case init_2_op:
        case init_2_safeop:
        case init_2_preop: {
            // ====> initial devices            
            log(info, "opening serial device %s ...\n", ifname.c_str());

            // set baudrate will also open port
            set_baudrate(baudrate);

            // add stream device
            k.add_device(static_pointer_cast<stream>(shared_from_this()));

            if (    (transition == init_2_preop))
                break;
        }
        case preop_2_op:
        case preop_2_safeop:
            // ====> start receiving measurements
            if (    (transition == init_2_safeop) ||
                    (transition == preop_2_safeop))
                break;
        case safeop_2_op:
            // ====> start sending commands
            break;
        case op_2_op:
        case safeop_2_safeop:
        case preop_2_preop:
            // ====> do nothing
            break;

        default:
            break;
    }

    return (this->state = state);
}

//! Open tty port.
/*!
 * \param[in] cflag_baudrate    Decoded defined baudrate.
 */
void tty::open_port(int cflag_baudrate) {
    struct termios newtio;

    fd = open(ifname.c_str(), O_RDWR | O_NOCTTY/*|O_NONBLOCK */| O_SYNC);
    if (fd < 0)
        throw str_exception("Error opening serial port %s!\n", ifname.c_str());

    bzero(&newtio, sizeof(newtio)); // clear struct for new port settings

    newtio.c_cflag      = cflag_baudrate | decode_character_size(char_size) | CREAD;
    
    if (use_clocal)     
        newtio.c_cflag |= CLOCAL;
    
    if (stopbits == 2)
        newtio.c_cflag |= CSTOPB;
    
    if (parity == "odd")
        newtio.c_cflag |= PARENB | PARODD;
    else if (parity == "even")
        newtio.c_cflag |= PARENB;

    newtio.c_iflag      = IGNPAR;
    newtio.c_oflag      = 0;
    newtio.c_lflag      = 0;
    newtio.c_cc[VTIME]  = 1; // 1 decisecond;
    newtio.c_cc[VMIN]   = 1; // 1 character;

    // clean the buffer and activate the settings for the port
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
        
    if (async_low_latency) {
        struct serial_struct ss;
        if (ioctl(fd, TIOCGSERIAL, &ss) != 0)
            throw str_exception("TIOCGSERIAL failed!\n");
    
        log(verbose, "setting low latency timer\n");
        ss.flags |= ASYNC_LOW_LATENCY;
        
        if (ioctl(fd, TIOCSSERIAL, &ss) < 0)
            throw str_exception("TIOCSSERIAL failed!\n");
    }
                
    if (configure_rs485) {
        struct serial_rs485 data;
        if (ioctl(fd, TIOCGRS485, &data)) {
            throw errno_exception_tb("TIOCGRS485 failed");
        }

        data.flags |= SER_RS485_ENABLED
            | SER_RS485_RTS_ON_SEND;

        if (ioctl(fd, TIOCSRS485, &data)) {
            throw errno_exception_tb("TIOCSRS485 failed");
        }
    }
}

//! Close tty port.
void tty::close_port() {
    if (fd == -1)
        return;

    close(fd);
    fd = -1;
}

//! Set stream device baudrate
/*!
 * \param[in] baudrate  New baudrate to set.
 */
void tty::set_baudrate(int baudrate) {
    this->baudrate = baudrate;
    int tmp_baudrate = decode_baudrate(baudrate);

    close_port();

    if (tmp_baudrate <= 0) {  // custom baudrate
        open_port(B38400);

        // try to set a custom divisor
        struct serial_struct ss;
        if (ioctl(fd, TIOCGSERIAL, &ss) != 0)
            throw str_exception("TIOCGSERIAL failed!\n");

        ss.flags = (ss.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
        ss.custom_divisor = (ss.baud_base + (baudrate / 2)) / baudrate;
        int closest_br = ss.baud_base / ss.custom_divisor;

        if (closest_br < baudrate * 98 / 100 || closest_br > baudrate * 102 / 100)
            throw str_exception("Cannot set speed to %d, closest is %d \n", baudrate, closest_br);

        if (ioctl(fd, TIOCSSERIAL, &ss) < 0)
            throw str_exception("TIOCSSERIAL failed!\n");
    } else {
        open_port(tmp_baudrate);
    }
}

//! Get stream device baudrate
/*!
 * \return Actual baudrate.
 */
int tty::get_baudrate() const {
    return baudrate;
}

//! Set serial port settings
/*!
 * \param[in] char_size     Character bit size.
 * \param[in] par           Parity setting.
 * \param[in] stopbits      Number of Stopbits.
 */
void tty::set_port_settings(const character_size_t& char_size, 
        const parity_t& par, const stopbits_t& stopbits) {
    this->char_size = (int)char_size;
    this->stopbits  = (int)stopbits;
    this->parity    = par == parity_off ? "off" : par == parity_even ? "even" : "odd";

    set_baudrate(baudrate);
}

//! Get serial port settings
/*!
 * \param[out] char_size     Character bit size.
 * \param[out] par           Parity setting.
 * \param[out] stopbits      Number of Stopbits.
 */
void tty::get_port_settings(character_size_t& char_size, 
        parity_t& par, stopbits_t& stopbits) const {
}

