//! robotkernel module for tty serial devices
/*!
 * author: Robert Burger
 *
 * $Id$
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

#if HAVE_TERMIOS_H == 1
#include <termios.h>
#endif

#ifdef __VXWORKS__
#include <ioLib.h>
#include <sioLib.h>
#include <strings.h>
#endif

MODULE_DEF(module_tty, module_tty::tty)

#define SWAP_BYTES(x) ((((x)&0xFF00) >> 8) | (((x)&0x00FF) << 8))

using namespace std;
using namespace robotkernel;
using namespace module_tty;

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
        default:
            log(module_error, "unknown baudrate! only know about 9600, "
                 "19200, 38400, 115200, 230400. assuming 115200\n");
            return B115200;
    }
#endif
}

//! construction
/*
 * \param name fts name
 * \param node YAML configuration node
 */
tty::tty(const char *name, const YAML::Node& node) 
    : module_base("module_tty", name) {
    fd             = -1;
    ifname         = get_as<std::string>(node, "ifname");
    baudrate       = get_as<unsigned>(node, "baudrate");
    timeout_us     = get_as<unsigned>(node, "timeout_us");

    const YAML::Node *value;
    if ((value = node.FindValue("post_open_script")))
        post_open = (*value).to<string>();
    else
        post_open = "";
}

//! destruction
tty::~tty() {
    // set to init, this will close serial device
    set_state(module_state_init);
}

size_t tty::read(void* buf, size_t bufsize) {
    if (state < module_state_safeop)
        // invalid state
        return 0;

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

                log(module_verbose, "select returned %s\n", strerror(errno));
                return 0;
            } else if (rc == 0) {
                log(module_warning, "reading from tty timed out\n");
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
    switch (state) {
        case module_state_init:
            if (fd > 0) {
                close(fd);
                fd = -1;
            }
            break;
        case module_state_preop: {
            log(module_info, "opening serial device %s ...\n", ifname.c_str());

            if (fd == -1) {
                fd = open(ifname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
                if (fd == -1)
                    throw str_exception("open %s: %s", ifname.c_str(), strerror(errno));
                 
                if (post_open != "") {
                    log(module_info, "executing post open script: %s\n", post_open.c_str());
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

                // Set baudrate.
                ret = cfsetispeed(&m_commState, br);
                if (ret == -1)
                    perror("cfsetispeed");
                ret = cfsetospeed(&m_commState, br);
                if (ret == -1)
                    perror("cfsetospeed");

                // Enable the receiver and set local mode
                m_commState.c_cflag |= (CLOCAL | CREAD);
                // Set character size to data bits and set no parity Mask the characte size bits
                m_commState.c_cflag &= ~(CSIZE|PARENB);
                m_commState.c_cflag |= CS8;             // Select 8 data bits
                m_commState.c_cflag &= ~CSTOPB;  // send 2 stop bits
                // Disable hardware flow control
#ifndef __QNX__
                m_commState.c_cflag &= ~CRTSCTS;
#endif
                m_commState.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
                // Disable software flow control
                m_commState.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);

                // Set the new options for the port
                ret = tcsetattr(fd,TCSANOW, &m_commState);
                if (ret == -1)
                    perror("tcsetattr:");

                ret = tcflush(fd, TCIOFLUSH);
                if (ret == -1)
                    perror("tcflush:");
#elif defined __VXWORKS__
                if (ioctl(fd, FIOBAUDRATE, br) == -1)
                    throw str_exception("FIONBAUDRATE: %s", 
                            strerror(errno));

                // configure interface to 8N2 configuration
                uint32_t hwopts = CLOCAL | CREAD | CS8;// | STOPB;
                if (ioctl(fd, SIO_HW_OPTS_SET, hwopts) == -1)
                    throw str_exception("SIO_HW_OPTS_SET: %s", 
                            strerror(errno));
#endif
            }
            break;
        }
        case module_state_safeop:
        case module_state_op:
        case module_state_boot:
            break;
        default:
            // invalid state
            return -1;
    }

    // assign new state
    this->state = state;

    return state;
}

//! send a request to module
/*!
  \param reqcode request code
  \param ptr pointer to request structure
  \return success or failure
  */
int tty::request(int reqcode, void* ptr) {
    int ret = 0;

    switch (reqcode) {
        case MOD_REQUEST_GET_MODULE_FEAT: {
            int *mod_feat = (int *)ptr;
            *mod_feat = MODULE_FEAT_READ | MODULE_FEAT_WRITE;
            break;
        }
        default:
            log(verbose, "not implemented request %d\n", 
                    reqcode);
            ret = -1;
            break;
    }

    return ret;
}
        
