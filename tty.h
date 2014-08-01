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

#ifndef __TTY_H__
#define __TTY_H__

#include "kernel.h"
#include "module_tty.h"
#include <string>
#include "yaml-cpp/yaml.h"

namespace module_tty {

class tty {
    public:
        std::string     _name;          //! module name
        std::string     _ifname;        //! serial interface name
        unsigned        _baudrate;      //! baudrate to use
        unsigned        _timeout_us;    //! select timeout 
        int             _fd;            //! fts file descriptor
        module_state_t  _state;         //! module state

        //! de-/construction
        /*
         * \param name fts name
         * \param node YAML configuration node
         */
        tty(const char *name, const YAML::Node& node);
        ~tty();

        //! set fts state
        /*!
         * \param state new fts state
         */
        int set_state(module_state_t state);

        //! read data from tty serial device
        /*!
         * \param data data to read
         * \param data_len length of data
         * \return size read/written
         */
        ssize_t read(char *data, size_t data_len);
        
        //! cyclic process data write
        /*!
          \param buf process data buffer
          \param bufsize size of process data buffer
          \return size of written bytes
          */
        ssize_t write(char *data, size_t data_len);
 
        //! send a request to module
        /*!
          \param reqcode request code
          \param ptr pointer to request structure
          \return success or failure
          */
        int request(int reqcode, void* ptr);
        
        //! log to kernel logging facility
        void log(robotkernel::loglevel lvl, const char *format, ...) {
            char buf[1024];

            // format argument list
            va_list args;
            va_start(args, format);
            vsnprintf(buf, 1024, format, args);
            klog(lvl, "[%s|%s] %s", MODNAME, _name.c_str(), buf);
        }
};

}; // namespace module_tty

#endif /* __TTY_H__ */

