/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_PLATFORM_FILE_MONITOR_IMPL_HPP
#define ADOBE_PLATFORM_FILE_MONITOR_IMPL_HPP

/****************************************************************************************************/

#include <sys/event.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct file_monitor_platform_data_t
{
    file_monitor_platform_data_t();

    file_monitor_platform_data_t(const file_monitor_path_type& path, const file_monitor_callback_t& proc);

    file_monitor_platform_data_t(const file_monitor_platform_data_t& rhs);

    ~file_monitor_platform_data_t();

    file_monitor_platform_data_t& operator = (const file_monitor_platform_data_t& rhs);

    void set_path(const file_monitor_path_type& path);

    void connect();

    void disconnect();

    file_monitor_path_type  path_m;
    file_monitor_callback_t proc_m;
    std::time_t             last_write_m;
    int                     fd_m;
    struct kevent           evt_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
