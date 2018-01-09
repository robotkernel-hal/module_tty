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

#ifndef _MODULE_TTY__TTY_H_
#define _MODULE_TTY__TTY_H_

#include "robotkernel/kernel.h"
#include "robotkernel/module_base.h"
#include "robotkernel/stream.h"
#include <string>
#include "yaml-cpp/yaml.h"

namespace module_tty {
#ifdef EMACS
}
#endif

class tty : 
    public std::enable_shared_from_this<tty>,
    public robotkernel::module_base,
    public robotkernel::stream
{
    private:
        //! decode baudrate to define
        /*!
         * \param baudrate input baudrate
         * \return baudrate define
         */
        int decode_baudrate(int baudrate);

    public:
        std::string     ifname;        //!< serial interface name
        unsigned        baudrate;      //!< baudrate to use
        unsigned        timeout_us;    //!< select timeout
        bool            hardware_flow_control;
        bool            no_baudrate;
        int             fd;            //!< fts file descriptor
        std::string     post_open;     //!< post open script
        bool            use_clocal;
        unsigned        n_stop_bits;

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

        //! cyclic process data read
        /*!
         * \param buf process data buffer
         * \param bufsize size of process data buffer
         * \return size of read bytes
         */
        size_t read(void* buf, size_t bufsize);

        //! cyclic process data write
        /*!
         * \param buf process data buffer
         * \param bufsize size of process data buffer
         * \return size of written bytes
         */
        size_t write(void* buf, size_t bufsize);
};

#ifdef EMACS
{
#endif
}; // namespace module_tty

#endif /* _MODULE_TTY__TTY_H_ */

