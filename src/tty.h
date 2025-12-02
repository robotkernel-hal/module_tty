//! robotkernel module for tty serial devices
/*!
 * author: Robert Burger <robert.burger@dlr.de>
 */

/*
 * This file is part of module_tty.
 *
 * module_tty is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * module_tty is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with module_tty; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MODULE_TTY__TTY_H
#define MODULE_TTY__TTY_H

#include "robotkernel/module_base.h"
#include "robotkernel/stream.h"
#include <string>
#include "yaml-cpp/yaml.h"

namespace module_tty {

class tty : 
    public virtual robotkernel::shared_base,
    public robotkernel::module_base,
    public robotkernel::serial_stream
{
    private:
        //! decode baudrate to define
        /*!
         * \param baudrate input baudrate
         * \return baudrate define
         */
        int decode_baudrate(const int baudrate) const;

        //! decodes character size to define which can be used with termios
        /*!
         * \param[in] char_size     Charater bit size.
         * \return     Corresponding bit size define.
         */
        unsigned decode_character_size(const int char_size) const;

    public:
        std::string     ifname;        //!< serial interface name
        unsigned        baudrate;      //!< baudrate to use
        unsigned        char_size;     //!< Character size.
        unsigned        stopbits;      //!< Number of stopbits.
        std::string     parity;        //!< Parity: 'off', 'even' or 'odd'
        unsigned        timeout_us;    //!< select timeout
        bool            hardware_flow_control;
        bool            no_baudrate;        
        int             fd;            //!< fts file descriptor
        std::string     post_open;     //!< post open script
        bool            use_clocal;
        bool            async_low_latency;
        bool            configure_rs485;

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

        //! Open tty port.
        /*!
         * \param[in] cflag_baudrate    Decoded defined baudrate.
         */
        void open_port(int cflag_baudrate);

        //! Close tty port.
        void close_port();

        //! Set stream device baudrate
        /*!
         * \param[in] baudrate  New baudrate to set.
         */
        void set_baudrate(int baudrate);

        //! Get stream device baudrate
        /*!
         * \return Actual baudrate.
         */
        int get_baudrate() const;

        //! Set serial port settings
        /*!
         * \param[in] char_size     Character bit size.
         * \param[in] par           Parity setting.
         * \param[in] stopbits      Number of Stopbits.
         */
        void set_port_settings(const character_size_t& char_size, 
                const parity_t& par, const stopbits_t& stopbits);

        //! Get serial port settings
        /*!
         * \param[out] char_size     Character bit size.
         * \param[out] par           Parity setting.
         * \param[out] stopbits      Number of Stopbits.
         */
        void get_port_settings(character_size_t& char_size, 
                parity_t& par, stopbits_t& stopbits) const;
};

}; // namespace module_tty

#endif /* MODULE_TTY__TTY_H */

