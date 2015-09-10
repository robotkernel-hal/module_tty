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

#include "robotkernel/kernel.h"
#include "robotkernel/module_base.h"
#include <string>
#include "yaml-cpp/yaml.h"

namespace module_tty {

class tty : public robotkernel::module_base {
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
 
        //! send a request to module
        /*!
          \param reqcode request code
          \param ptr pointer to request structure
          \return success or failure
          */
        int request(int reqcode, void* ptr);
};

}; // namespace module_tty

#endif /* __TTY_H__ */

