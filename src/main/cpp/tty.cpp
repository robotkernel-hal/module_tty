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

int tty::decode_baudrate(int baudrate) {
#if defined __QNX__ || defined __VXWORKS__
    return baudrate;
#else
    switch (baudrate) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
#ifdef  B460800
            return B460800;
#else
	    log(error, "Boudrate 460800 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 500000:
#ifdef  B500000
            return B500000;
#else
	    log(error, "Boudrate 460800 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 576000:
#ifdef  B576000
            return B576000;
#else
	    log(error, "Boudrate 576000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 921600:
#ifdef  B921600
            return B921600;
#else
	    log(error, "Boudrate 921600 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 1000000:
#ifdef  B1000000
            return B1000000;
#else
	    log(error, "Boudrate 1000000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 1152000:
#ifdef  B1152000
            return B1152000;
#else
	    log(error, "Boudrate 1152000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 1500000:
#ifdef  B1500000
            return B1500000;
#else
	    log(error, "Boudrate 1500000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 2000000:
#ifdef  B2000000
            return B2000000;
#else
	    log(error, "Boudrate 2000000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 2500000:
#ifdef  B2500000
            return B2500000;
#else
	    log(error, "Boudrate 2500000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 3000000:
#ifdef  B3000000
            return B3000000;
#else
	    log(error, "Boudrate 3000000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 3500000:
#ifdef  B3500000
            return B3500000;
#else
	    log(error, "Boudrate 3500000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        case 4000000:
#ifdef  B4000000
            return B4000000;
#else
	    log(error, "Boudrate 4000000 not available on this system. Assuming 115200\n");
	    return B115200;
#endif
        default:
            log(error, "unknown baudrate! Assuming 115200\n");
            return B115200;
    }
#endif
}

//! construction
/*
 * \param name fts name
 * \param node YAML configuration node
 */
tty::tty(const char *name, const YAML::Node& node) :
    module_base("module_tty", name, node),
    stream(name, "tty")
{
    fd                    = -1;
    ifname                = get_as<std::string>(node, "ifname");
    baudrate              = get_as<unsigned>(node, "baudrate", 0);
    timeout_us            = get_as<unsigned>(node, "timeout_us");
    hardware_flow_control = get_as<bool>(node, "hardware_flow_control", false);
    no_baudrate           = get_as<bool>(node, "no_baudrate", false);
    use_clocal            = get_as<bool>(node, "use_clocal", true);
    post_open             = get_as<string>(node, "post_open_script", "");

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
                return 0;
            } else if (rc == 0) {
                log(warning, "reading from tty timed out\n");
                return 0;
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

            close(fd);
            fd = -1;
        case init_2_init:
            // ====> do nothing
            break;

        case init_2_op:
        case init_2_safeop:
        case init_2_preop:
            // ====> initial devices            
            log(info, "opening serial device %s ...\n", ifname.c_str());

            if (fd == -1) {
                fd = open(ifname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
                if (fd == -1)
                    throw str_exception("open %s: %s", ifname.c_str(), strerror(errno));
                 
                if (post_open != "") {
                    log(info, "executing post open script: %s\n", post_open.c_str());
                    system(post_open.c_str());
                }

                // decode baudrate, depends on platform
                int br = decode_baudrate(baudrate);

#if HAVE_TERMIOS_H == 1
                termios m_commState;

                /* Start configuring of port for non-canonical transfer mode */
                // Get current options for the port
                tcgetattr(fd, &m_commState);

                int ret;

                if(!no_baudrate) {
                    // Set baudrate.
                    ret = cfsetispeed(&m_commState, br);
                    if (ret == -1)
                        perror("cfsetispeed");
                    ret = cfsetospeed(&m_commState, br);
                    if (ret == -1)
                        perror("cfsetospeed");
                }

                // Enable the receiver and set local mode
                m_commState.c_cflag |= (CLOCAL | CREAD);
                // Set character size to data bits and set no parity Mask the characte size bits
                m_commState.c_cflag &= ~(CSIZE|PARENB);
                m_commState.c_cflag |= CS8;             // Select 8 data bits
                m_commState.c_cflag &= ~CSTOPB;  // send 1 stop bits
                // Disable hardware flow control
#ifndef __QNX__
                if(hardware_flow_control) {
                    log(info, "enabling hardware flow control\n");
                    m_commState.c_cflag |= CRTSCTS;
                } else
                    m_commState.c_cflag &= ~CRTSCTS;
#endif
                m_commState.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
                // Disable software flow control
                m_commState.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);

                /*
                   m_commState.c_cc[VMIN] = 1;
                   m_commState.c_cc[VTIME] = (unsigned int)(timeout_us / 1e5);
                   */

                // Set the new options for the port
                ret = tcsetattr(fd,TCSANOW, &m_commState);
                if (ret == -1)
                    perror("tcsetattr:");

                ret = tcflush(fd, TCIOFLUSH);
                if (ret == -1)
                    perror("tcflush:");
#elif defined __VXWORKS__
                log(info, "setting baudrate to %d\n", br);
                if (ioctl(fd, FIOBAUDRATE, br) == -1)
                    throw str_exception("FIONBAUDRATE: %s", 
                            strerror(errno));

                // configure interface to 8N1 configuration
                uint32_t hwopts = CREAD | CS8;// | STOPB;
                if (use_clocal)
                    hwopts |= CLOCAL;
                if (ioctl(fd, SIO_HW_OPTS_SET, hwopts) == -1)
                    throw str_exception("SIO_HW_OPTS_SET: %s", 
                            strerror(errno));
#endif
            }

            // add stream device
            k.add_device(static_pointer_cast<stream>(shared_from_this()));

            if (    (transition == init_2_preop))
                break;
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
 
