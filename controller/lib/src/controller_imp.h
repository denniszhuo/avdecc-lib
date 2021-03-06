/*
 * Licensed under the MIT License (MIT)
 *
 * Copyright (c) 2013 AudioScience Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * controller_imp.h
 *
 * Controller implementation class
 */

#pragma once
#ifndef _AVDECC_CONTROLLER_LIB_CONTROLLER_IMP_H_
#define _AVDECC_CONTROLLER_LIB_CONTROLLER_IMP_H_

#include "controller.h"

namespace avdecc_lib
{
    class controller_imp : public virtual controller
    {
    private:
        std::vector<end_station_imp *> end_station_vec; // Store a list of End Station objects

    public:
        /**
         * A constructor for controller_imp used for constructing an object with notification, and post_log_msg callback functions.
         */
        controller_imp(void (*notification_callback) (void *, int32_t, uint64_t, uint16_t, uint16_t, uint16_t, uint32_t, void *),
                       void (*log_callback) (void *, int32_t, const char *, int32_t));

        virtual ~controller_imp();

        /**
         * Call destructor for Controller used for destroying objects
         */
        void STDCALL destroy();

        /**
         * Get the current build release version.
         */
        const char * STDCALL get_version() const;

        /**
         * Get the total number of End Stations connected
         */
        uint32_t STDCALL get_end_station_count();

        /**
         * Get the corresponding End Station by index.
         */
        end_station * STDCALL get_end_station_by_index(uint32_t end_station_index);

        /**
         * Check if the corresponding End Station with the GUID exist.
         */
        bool is_end_station_found_by_guid(uint64_t entity_guid, uint32_t &end_station_index);

        /**
         * Get the corresponding CONFIGURATION descriptor by index.
         */
        configuration_descriptor * STDCALL get_config_desc_by_index(uint32_t end_station_index, uint16_t entity_index, uint16_t config_index);

        /**
         * Get the corresponding CONFIGURATION descriptor by GUID.
         */
        configuration_descriptor * STDCALL get_config_desc_by_guid(uint64_t entity_guid, uint16_t entity_index, uint16_t config_index);

        /**
         * Check if the command with the corresponding notification id is in the inflight list.
         */
        bool is_inflight_cmd_with_notification_id(void *notification_id);

        /**
         * Update the base log level for messages to be logged by the post_log_msg callback.
         */
        void STDCALL set_logging_level(int32_t new_log_level);

        /**
         * Get the missed notification event count.
         */
        uint32_t STDCALL missed_notification_count();

        /**
         * Get the missed log event count.
         */
        uint32_t STDCALL missed_log_count();

        /**
         * Check for End Station connection, command packet, and response packet timeouts.
         */
        void time_tick_event();

        /**
         * Find an end station that matches the entity and controller IDs
         */
        int find_in_end_station(struct jdksavdecc_eui64 &entity_guid, const uint8_t *frame);

        /**
         * Lookup and process packet received.
         */
        void rx_packet_event(void *&notification_id, bool &is_notification_id_valid, const uint8_t *frame, uint16_t frame_len, int &status);

        /**
         * Send queued packet to the AEM Controller State Machine.
         */
        void tx_packet_event(void *notification_id, uint32_t notification_flag, uint8_t *frame, uint16_t frame_len);

        /**
         * Send a CONTROLLER_AVAILABLE command to verify that the AVDECC Controller is still there.
         */
        int STDCALL send_controller_avail_cmd(void *notification_id, uint32_t end_station_index);

        /**
         * Process a CONTROLLER_AVAILABLE response for the CONTROLLER_AVAILABLE command.
         */
        int proc_controller_avail_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status);
    };

    extern controller_imp *controller_imp_ref;
}

#endif
