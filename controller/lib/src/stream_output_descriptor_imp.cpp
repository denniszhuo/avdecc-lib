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
 * stream_output_descriptor_imp.cpp
 *
 * STREAM OUTPUT descriptor implementation
 */

#include <vector>
#include "util_imp.h"
#include "enumeration.h"
#include "log_imp.h"
#include "adp.h"
#include "end_station_imp.h"
#include "system_tx_queue.h"
#include "acmp_controller_state_machine.h"
#include "aem_controller_state_machine.h"
#include "stream_output_descriptor_imp.h"

namespace avdecc_lib
{
    stream_output_descriptor_imp::stream_output_descriptor_imp(end_station_imp *end_station_obj, const uint8_t *frame, size_t pos, size_t frame_len) : descriptor_base_imp(end_station_obj)
    {
        stream_output_desc_read_returned = jdksavdecc_descriptor_stream_read(&stream_output_desc, frame, pos, frame_len);

        if(stream_output_desc_read_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "0x%llx, stream_output_desc_read error", end_station_obj->guid());
            assert(stream_output_desc_read_returned >= 0);
        }

        memset(&stream_output_flags, 0, sizeof(struct stream_output_desc_stream_flags));
        memset(&aem_cmd_set_stream_format_resp, 0, sizeof(struct jdksavdecc_aem_command_set_stream_format_response));
        memset(&aem_cmd_get_stream_format_resp, 0, sizeof(struct jdksavdecc_aem_command_get_stream_format_response));
        memset(&aem_cmd_set_stream_info_resp, 0, sizeof(struct jdksavdecc_aem_command_set_stream_info_response));
        memset(&aem_cmd_get_stream_info_resp, 0, sizeof(struct jdksavdecc_aem_command_get_stream_info_response));

        memset(&acmp_cmd_get_tx_state_resp, 0, sizeof(struct jdksavdecc_acmpdu));
        memset(&acmp_cmd_get_tx_connection_resp, 0, sizeof(struct jdksavdecc_acmpdu));

        stream_flags_init();
    }

    stream_output_descriptor_imp::~stream_output_descriptor_imp() {}

    void stream_output_descriptor_imp::stream_flags_init()
    {
        stream_output_flags.clock_sync_source = stream_output_desc.stream_flags >> 0 & 0x01;;
        stream_output_flags.class_a = stream_output_desc.stream_flags >> 1 & 0x01;
        stream_output_flags.class_b = stream_output_desc.stream_flags >> 2 & 0x01;
        stream_output_flags.supports_encrypted = stream_output_desc.stream_flags >> 3 & 0x01;
        stream_output_flags.primary_backup_supported = stream_output_desc.stream_flags >> 4 & 0x01;
        stream_output_flags.primary_backup_valid = stream_output_desc.stream_flags >> 5 & 0x01;
        stream_output_flags.secondary_backup_supported = stream_output_desc.stream_flags >> 6 & 0x01;
        stream_output_flags.secondary_backup_valid = stream_output_desc.stream_flags >> 7 & 0x01;
        stream_output_flags.tertiary_backup_supported = stream_output_desc.stream_flags >> 8 & 0x01;
        stream_output_flags.tertiary_back_up_valid = stream_output_desc.stream_flags >> 9 & 0x01;
    }

    uint16_t STDCALL stream_output_descriptor_imp::descriptor_type() const
    {
        assert(stream_output_desc.descriptor_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT);
        return stream_output_desc.descriptor_type;
    }

    uint16_t STDCALL stream_output_descriptor_imp::descriptor_index() const
    {
        return stream_output_desc.descriptor_index;
    }

    uint8_t * STDCALL stream_output_descriptor_imp::object_name()
    {
        return stream_output_desc.object_name.value;
    }

    uint16_t STDCALL stream_output_descriptor_imp::localized_description()
    {
        return stream_output_desc.localized_description;
    }

    uint16_t STDCALL stream_output_descriptor_imp::clock_domain_index()
    {
        return stream_output_desc.clock_domain_index;
    }

    uint16_t stream_output_descriptor_imp::stream_flags()
    {
        return stream_output_desc.stream_flags;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_clock_sync_source()
    {
        return stream_output_flags.clock_sync_source;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_class_a()
    {
        return stream_output_flags.class_a;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_class_b()
    {
        return stream_output_flags.class_b;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_supports_encrypted()
    {
        return stream_output_flags.supports_encrypted;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_primary_backup_supported()
    {
        return stream_output_flags.primary_backup_supported;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_primary_backup_valid()
    {
        return stream_output_flags.primary_backup_valid;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_secondary_backup_supported()
    {
        return stream_output_flags.secondary_backup_supported;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_secondary_backup_valid()
    {
        return stream_output_flags.secondary_backup_valid;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_tertiary_backup_supported()
    {
        return stream_output_flags.tertiary_backup_supported;
    }

    bool STDCALL stream_output_descriptor_imp::stream_flags_tertiary_back_up_valid()
    {
        return stream_output_flags.tertiary_back_up_valid;
    }

    const char * stream_output_descriptor_imp::current_format()
    {
        uint64_t current_format = jdksavdecc_uint64_get(&stream_output_desc.current_format, 0);

        return utility->ieee1722_format_value_to_name(current_format);
    }

    uint16_t stream_output_descriptor_imp::formats_offset()
    {
        assert(stream_output_desc.formats_offset == 132);
        return stream_output_desc.formats_offset;
    }

    uint16_t STDCALL stream_output_descriptor_imp::number_of_formats()
    {
        assert(stream_output_desc.number_of_formats <= 47);
        return stream_output_desc.number_of_formats;
    }

    uint64_t STDCALL stream_output_descriptor_imp::backup_talker_entity_id_0()
    {
        return jdksavdecc_uint64_get(&stream_output_desc.backup_talker_entity_id_0, 0);
    }

    uint16_t STDCALL stream_output_descriptor_imp::backup_talker_unique_0()
    {
        return stream_output_desc.backup_talker_unique_0;
    }

    uint64_t STDCALL stream_output_descriptor_imp::backup_talker_entity_id_1()
    {
        return jdksavdecc_uint64_get(&stream_output_desc.backup_talker_entity_id_1, 0);
    }

    uint16_t STDCALL stream_output_descriptor_imp::backup_talker_unique_1()
    {
        return stream_output_desc.backup_talker_unique_1;
    }

    uint64_t STDCALL stream_output_descriptor_imp::backup_talker_entity_id_2()
    {
        return jdksavdecc_uint64_get(&stream_output_desc.backup_talker_entity_id_2, 0);
    }

    uint16_t STDCALL stream_output_descriptor_imp::backup_talker_unique_2()
    {
        return stream_output_desc.backup_talker_unique_2;
    }

    uint64_t STDCALL stream_output_descriptor_imp::backedup_talker_entity_id()
    {
        return jdksavdecc_uint64_get(&stream_output_desc.backedup_talker_entity_id, 0);
    }

    uint16_t STDCALL stream_output_descriptor_imp::backedup_talker_unique()
    {
        return stream_output_desc.backedup_talker_unique;
    }

    uint16_t STDCALL stream_output_descriptor_imp::avb_interface_index()
    {
        return stream_output_desc.avb_interface_index;
    }

    uint32_t STDCALL stream_output_descriptor_imp::buffer_length()
    {
        return stream_output_desc.buffer_length;
    }

    uint64_t STDCALL stream_output_descriptor_imp::set_stream_format_stream_format()
    {
        return jdksavdecc_uint64_get(&aem_cmd_set_stream_format_resp.stream_format, 0);
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_stream_format_stream_format()
    {
        return jdksavdecc_uint64_get(&aem_cmd_get_stream_format_resp.stream_format, 0);
    }

    uint32_t STDCALL stream_output_descriptor_imp::get_stream_info_flags()
    {
        return aem_cmd_get_stream_info_resp.aem_stream_info_flags;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_stream_info_stream_format()
    {
        return jdksavdecc_uint64_get(&aem_cmd_get_stream_info_resp.stream_format, 0);
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_stream_info_stream_id()
    {
        return jdksavdecc_uint64_get(&aem_cmd_get_stream_info_resp.stream_id, 0);
    }

    uint32_t STDCALL stream_output_descriptor_imp::get_stream_info_msrp_accumulated_latency()
    {
        return aem_cmd_get_stream_info_resp.msrp_accumulated_latency;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_stream_info_stream_dest_mac()
    {
        uint64_t stream_dest_mac;
        utility->convert_eui48_to_uint64(aem_cmd_get_stream_info_resp.stream_dest_mac.value, stream_dest_mac);

        return stream_dest_mac;
    }

    uint8_t STDCALL stream_output_descriptor_imp::get_stream_info_msrp_failure_code()
    {
        return aem_cmd_get_stream_info_resp.msrp_failure_code;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_stream_info_msrp_failure_bridge_id()
    {
        return jdksavdecc_uint64_get(&aem_cmd_get_stream_info_resp.msrp_failure_bridge_id, 0);
    }

    bool stream_output_descriptor_imp::is_clock_sync_source_set()
    {
        return stream_output_flags.clock_sync_source;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_tx_state_stream_id()
    {
        return jdksavdecc_uint64_get(&acmp_cmd_get_tx_state_resp.header.stream_id, 0);
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_tx_state_stream_dest_mac()
    {
        uint64_t stream_dest_mac;
        utility->convert_eui48_to_uint64(acmp_cmd_get_tx_state_resp.stream_dest_mac.value, stream_dest_mac);

        return stream_dest_mac;
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_state_connection_count()
    {
        return acmp_cmd_get_tx_state_resp.connection_count;
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_state_stream_vlan_id()
    {
        return acmp_cmd_get_tx_state_resp.stream_vlan_id;
    }
    
    uint64_t STDCALL stream_output_descriptor_imp::get_tx_connection_stream_id()
    {
        return jdksavdecc_uint64_get(&acmp_cmd_get_tx_connection_resp.header.stream_id, 0);
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_connection_talker_unique_id()
    {
        return acmp_cmd_get_tx_connection_resp.talker_unique_id;
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_connection_listener_unique_id()
    {
        return acmp_cmd_get_tx_connection_resp.listener_unique_id;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_tx_connection_stream_dest_mac()
    {
        uint64_t stream_dest_mac;
        utility->convert_eui48_to_uint64(acmp_cmd_get_tx_connection_resp.stream_dest_mac.value, stream_dest_mac);

        return stream_dest_mac;
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_connection_connection_count()
    {
        return acmp_cmd_get_tx_connection_resp.connection_count;
    }

    uint16_t STDCALL stream_output_descriptor_imp::get_tx_connection_stream_vlan_id()
    {
        return acmp_cmd_get_tx_connection_resp.stream_vlan_id;
    }

    uint64_t STDCALL stream_output_descriptor_imp::get_tx_connection_listener_entity_id()
    {
        return jdksavdecc_uint64_get(&acmp_cmd_get_tx_connection_resp.listener_entity_id, 0);
    }


    int STDCALL stream_output_descriptor_imp::send_set_stream_format_cmd(void *notification_id, uint64_t new_stream_format)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_set_stream_format aem_cmd_set_stream_format;
        int aem_cmd_set_stream_format_returned;
        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************** AECP Common Data *********************************************/
        aem_cmd_set_stream_format.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        // Fill aem_cmd_set_stream_format.sequence_id in AEM Controller State Machine
        aem_cmd_set_stream_format.command_type = JDKSAVDECC_AEM_COMMAND_GET_STREAM_FORMAT;

        /**************************************** AECP Message Specific Data *************************************/
        aem_cmd_set_stream_format.descriptor_type = descriptor_type();
        aem_cmd_set_stream_format.descriptor_index = descriptor_index();
        jdksavdecc_uint64_write(new_stream_format, &aem_cmd_set_stream_format.stream_format, 0, sizeof(uint64_t));

        /******************************** Fill frame payload with AECP data and send the frame ***************************/
        aem_controller_state_machine_ref->ether_frame_init(base_end_station_imp_ref->mac(), cmd_frame);
        aem_cmd_set_stream_format_returned = jdksavdecc_aem_command_set_stream_format_write(&aem_cmd_set_stream_format,
                                                                                            cmd_frame->payload,
                                                                                            ETHER_HDR_SIZE,
                                                                                            sizeof(cmd_frame->payload));

        if(aem_cmd_set_stream_format_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_set_stream_format_write error\n");
            assert(aem_cmd_set_stream_format_returned >= 0);
            return -1;
        }

        aem_controller_state_machine_ref->common_hdr_init(cmd_frame, base_end_station_imp_ref->guid());
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_set_stream_format_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        int aem_cmd_set_stream_format_resp_returned;
        uint32_t msg_type;
        bool u_field;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);

        aem_cmd_set_stream_format_resp_returned = jdksavdecc_aem_command_set_stream_format_response_read(&aem_cmd_set_stream_format_resp,
                                                                                                         frame,
                                                                                                         ETHER_HDR_SIZE,
                                                                                                         frame_len);

        if(aem_cmd_set_stream_format_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_set_stream_format_resp_read error\n");
            assert(aem_cmd_set_stream_format_resp_returned >= 0);
            return -1;
        }

        msg_type = aem_cmd_set_stream_format_resp.aem_header.aecpdu_header.header.message_type;
        status = aem_cmd_set_stream_format_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_set_stream_format_resp.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type

        aem_controller_state_machine_ref->update_inflight_for_rcvd_resp(notification_id, msg_type, u_field, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_get_stream_format_cmd(void *notification_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_get_stream_format aem_cmd_get_stream_format;
        int aem_cmd_get_stream_format_returned;
        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************** AECP Common Data *********************************************/
        aem_cmd_get_stream_format.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        // Fill aem_cmd_get_stream_format.sequence_id in AEM Controller State Machine
        aem_cmd_get_stream_format.command_type = JDKSAVDECC_AEM_COMMAND_GET_STREAM_FORMAT;

        /******************* AECP Message Specific Data ********************/
        aem_cmd_get_stream_format.descriptor_type = descriptor_type();
        aem_cmd_get_stream_format.descriptor_index = descriptor_index();

        /******************************** Fill frame payload with AECP data and send the frame ***************************/
        aem_controller_state_machine_ref->ether_frame_init(base_end_station_imp_ref->mac(), cmd_frame);
        aem_cmd_get_stream_format_returned = jdksavdecc_aem_command_get_stream_format_write(&aem_cmd_get_stream_format,
                                                                                            cmd_frame->payload,
                                                                                            ETHER_HDR_SIZE,
                                                                                            sizeof(cmd_frame->payload));

        if(aem_cmd_get_stream_format_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_get_stream_format_write error\n");
            assert(aem_cmd_get_stream_format_returned >= 0);
            return -1;
        }

        aem_controller_state_machine_ref->common_hdr_init(cmd_frame, base_end_station_imp_ref->guid());
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_get_stream_format_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        int aem_cmd_get_stream_format_resp_returned;
        uint32_t msg_type;
        bool u_field;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);

        aem_cmd_get_stream_format_resp_returned = jdksavdecc_aem_command_get_stream_format_response_read(&aem_cmd_get_stream_format_resp,
                                                                                                         frame,
                                                                                                         ETHER_HDR_SIZE,
                                                                                                         frame_len);

        if(aem_cmd_get_stream_format_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_get_stream_format_resp_read error\n");
            assert(aem_cmd_get_stream_format_resp_returned >= 0);
            return -1;
        }

        msg_type = aem_cmd_get_stream_format_resp.aem_header.aecpdu_header.header.message_type;
        status = aem_cmd_get_stream_format_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_get_stream_format_resp.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type

        aem_controller_state_machine_ref->update_inflight_for_rcvd_resp(notification_id, msg_type, u_field, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_set_stream_info_cmd(void *notification_id, void *new_stream_info_field)
    {
        log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "Need to implement SET_STREAM_INFO command.");

        return 0;
    }

    int stream_output_descriptor_imp::proc_set_stream_info_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "Need to implement SET_STREAM_INFO response.");

        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_get_stream_info_cmd(void *notification_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_get_stream_info aem_cmd_get_stream_info;
        int aem_cmd_get_stream_info_returned;
        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************** AECP Common Data *******************************************/
        aem_cmd_get_stream_info.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        // Fill aem_cmd_get_stream_info.sequence_id in AEM Controller State Machine
        aem_cmd_get_stream_info.command_type = JDKSAVDECC_AEM_COMMAND_GET_STREAM_INFO;

        /******************** AECP Message Specific Data *****************/
        aem_cmd_get_stream_info.descriptor_type = descriptor_type();
        aem_cmd_get_stream_info.descriptor_index = descriptor_index();

        /************************** Fill frame payload with AECP data and send the frame *****************************/
        aem_controller_state_machine_ref->ether_frame_init(base_end_station_imp_ref->mac(), cmd_frame);
        aem_cmd_get_stream_info_returned = jdksavdecc_aem_command_get_stream_info_write(&aem_cmd_get_stream_info,
                                                                                        cmd_frame->payload,
                                                                                        ETHER_HDR_SIZE,
                                                                                        sizeof(cmd_frame->payload));

        if(aem_cmd_get_stream_info_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_get_stream_info_write error\n");
            assert(aem_cmd_get_stream_info_returned >= 0);
            return -1;
        }

        aem_controller_state_machine_ref->common_hdr_init(cmd_frame, base_end_station_imp_ref->guid());
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_get_stream_info_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        int aem_cmd_get_stream_info_resp_returned;
        uint32_t msg_type;
        bool u_field;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);
        aem_cmd_get_stream_info_resp_returned = jdksavdecc_aem_command_get_stream_info_response_read(&aem_cmd_get_stream_info_resp,
                                                                                                     frame,
                                                                                                     ETHER_HDR_SIZE,
                                                                                                     frame_len);

        if(aem_cmd_get_stream_info_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_get_stream_info_resp_read error");
            assert(aem_cmd_get_stream_info_resp_returned >= 0);
            return -1;
        }

        msg_type = aem_cmd_get_stream_info_resp.aem_header.aecpdu_header.header.message_type;
        status = aem_cmd_get_stream_info_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_get_stream_info_resp.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type

        aem_controller_state_machine_ref->update_inflight_for_rcvd_resp(notification_id, msg_type, u_field, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_start_streaming_cmd(void *notification_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_start_streaming aem_cmd_start_streaming;
        int aem_cmd_start_streaming_returned;
        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************** AECP Common Data *******************************************/
        aem_cmd_start_streaming.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        // Fill aem_cmd_start_streaming.sequence_id in AEM Controller State Machine
        aem_cmd_start_streaming.command_type = JDKSAVDECC_AEM_COMMAND_START_STREAMING;

        /******************** AECP Message Specific Data *****************/
        aem_cmd_start_streaming.descriptor_type = descriptor_type();
        aem_cmd_start_streaming.descriptor_index = descriptor_index();

        /************************** Fill frame payload with AECP data and send the frame ***************************/
        aem_controller_state_machine_ref->ether_frame_init(base_end_station_imp_ref->mac(), cmd_frame);
        aem_cmd_start_streaming_returned = jdksavdecc_aem_command_start_streaming_write(&aem_cmd_start_streaming,
                                                                                        cmd_frame->payload,
                                                                                        ETHER_HDR_SIZE,
                                                                                        sizeof(cmd_frame->payload));

        if(aem_cmd_start_streaming_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_start_streaming_write error\n");
            assert(aem_cmd_start_streaming_returned >= 0);
            return -1;
        }

        aem_controller_state_machine_ref->common_hdr_init(cmd_frame, base_end_station_imp_ref->guid());
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_start_streaming_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_start_streaming_response aem_cmd_start_streaming_resp;
        int aem_cmd_start_streaming_resp_returned;
        uint32_t msg_type;
        bool u_field;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);
        aem_cmd_start_streaming_resp_returned = jdksavdecc_aem_command_start_streaming_response_read(&aem_cmd_start_streaming_resp,
                                                                                                     frame,
                                                                                                     ETHER_HDR_SIZE,
                                                                                                     frame_len);

        if(aem_cmd_start_streaming_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_start_streaming_resp_read error");
            assert(aem_cmd_start_streaming_resp_returned >= 0);
            return -1;
        }

        msg_type = aem_cmd_start_streaming_resp.aem_header.aecpdu_header.header.message_type;
        status = aem_cmd_start_streaming_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_start_streaming_resp.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type

        aem_controller_state_machine_ref->update_inflight_for_rcvd_resp(notification_id, msg_type, u_field, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_stop_streaming_cmd(void *notification_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_stop_streaming aem_cmd_stop_streaming;
        int aem_cmd_stop_streaming_returned;
        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************* AECP Common Data *******************************************/
        aem_cmd_stop_streaming.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        // Fill aem_cmd_stop_streaming.sequence_id in AEM Controller State Machine
        aem_cmd_stop_streaming.command_type = JDKSAVDECC_AEM_COMMAND_STOP_STREAMING;

        /******************** AECP Message Specific Data ****************/
        aem_cmd_stop_streaming.descriptor_type = descriptor_type();
        aem_cmd_stop_streaming.descriptor_index = descriptor_index();

        /************************** Fill frame payload with AECP data and send the frame *************************/
        aem_controller_state_machine_ref->ether_frame_init(base_end_station_imp_ref->mac(), cmd_frame);
        aem_cmd_stop_streaming_returned = jdksavdecc_aem_command_stop_streaming_write(&aem_cmd_stop_streaming,
                                                                                      cmd_frame->payload,
                                                                                      ETHER_HDR_SIZE,
                                                                                      sizeof(cmd_frame->payload));

        if(aem_cmd_stop_streaming_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_stop_streaming_write error\n");
            assert(aem_cmd_stop_streaming_returned >= 0);
            return -1;
        }

        aem_controller_state_machine_ref->common_hdr_init(cmd_frame, base_end_station_imp_ref->guid());
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_stop_streaming_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_aem_command_stop_streaming_response aem_cmd_stop_streaming_resp;
        int aem_cmd_stop_streaming_resp_returned;
        uint32_t msg_type;
        bool u_field;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);
        aem_cmd_stop_streaming_resp_returned = jdksavdecc_aem_command_stop_streaming_response_read(&aem_cmd_stop_streaming_resp,
                                                                                                   frame,
                                                                                                   ETHER_HDR_SIZE,
                                                                                                   frame_len);

        if(aem_cmd_stop_streaming_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "aem_cmd_stop_streaming_resp_read error");
            assert(aem_cmd_stop_streaming_resp_returned >= 0);
            return -1;
        }

        msg_type = aem_cmd_stop_streaming_resp.aem_header.aecpdu_header.header.message_type;
        status = aem_cmd_stop_streaming_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_stop_streaming_resp.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type

        aem_controller_state_machine_ref->update_inflight_for_rcvd_resp(notification_id, msg_type, u_field, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_get_tx_state_cmd(void *notification_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_acmpdu acmp_cmd_get_tx_state;
        int acmp_cmd_get_tx_state_returned;
        uint64_t talker_guid = base_end_station_imp_ref->get_entity_desc_by_index(0)->entity_id();

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /******************************************* ACMP Common Data ******************************************/
        acmp_cmd_get_tx_state.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        jdksavdecc_uint64_write(talker_guid, &acmp_cmd_get_tx_state.talker_entity_id, 0, sizeof(uint64_t));
        jdksavdecc_eui64_init(&acmp_cmd_get_tx_state.listener_entity_id);
        acmp_cmd_get_tx_state.talker_unique_id = descriptor_index();
        acmp_cmd_get_tx_state.listener_unique_id = 0;
        jdksavdecc_eui48_init(&acmp_cmd_get_tx_state.stream_dest_mac);
        acmp_cmd_get_tx_state.connection_count = 0;
        // Fill acmp_cmd_get_tx_state.sequence_id in AEM Controller State Machine
        acmp_cmd_get_tx_state.flags = 0;
        acmp_cmd_get_tx_state.stream_vlan_id = 0;

        /*************** Fill frame payload with AECP data and send the frame ****************/
        acmp_controller_state_machine_ref->ether_frame_init(cmd_frame);
        acmp_cmd_get_tx_state_returned = jdksavdecc_acmpdu_write(&acmp_cmd_get_tx_state,
                                                                 cmd_frame->payload,
                                                                 ETHER_HDR_SIZE,
                                                                 sizeof(cmd_frame->payload));

        if(acmp_cmd_get_tx_state_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "cmd_get_tx_state_write error\n");
            assert(acmp_cmd_get_tx_state_returned >= 0);
            return -1;
        }

        acmp_controller_state_machine_ref->common_hdr_init(JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_COMMAND, cmd_frame);
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_get_tx_state_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        int acmp_cmd_get_tx_state_resp_returned;
        uint32_t msg_type;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);
        acmp_cmd_get_tx_state_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_get_tx_state_resp,
                                                                     frame,
                                                                     ETHER_HDR_SIZE,
                                                                     frame_len);

        if(acmp_cmd_get_tx_state_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "acmp_cmd_get_tx_state_read error");
            assert(acmp_cmd_get_tx_state_resp_returned >= 0);
            return -1;
        }

        msg_type = acmp_cmd_get_tx_state_resp.header.message_type;
        status = acmp_cmd_get_tx_state_resp.header.status;

        acmp_controller_state_machine_ref->state_resp(notification_id, msg_type, cmd_frame);

        free(cmd_frame);
        return 0;
    }

    int STDCALL stream_output_descriptor_imp::send_get_tx_connection_cmd(void *notification_id, uint64_t listener_guid, uint16_t listener_unique_id)
    {
        struct jdksavdecc_frame *cmd_frame;
        struct jdksavdecc_acmpdu acmp_cmd_get_tx_connection;
        int acmp_cmd_get_tx_connection_returned;
        uint64_t talker_guid = base_end_station_imp_ref->get_entity_desc_by_index(0)->entity_id();

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));

        /********************************************* ACMP Common Data *********************************************/
        acmp_cmd_get_tx_connection.controller_entity_id = base_end_station_imp_ref->get_adp()->get_controller_guid();
        jdksavdecc_uint64_write(talker_guid, &acmp_cmd_get_tx_connection.talker_entity_id, 0, sizeof(uint64_t));
        jdksavdecc_uint64_write(listener_guid, &acmp_cmd_get_tx_connection.listener_entity_id, 0, sizeof(uint64_t));
        acmp_cmd_get_tx_connection.talker_unique_id = descriptor_index();
        acmp_cmd_get_tx_connection.listener_unique_id = listener_unique_id;
        jdksavdecc_eui48_init(&acmp_cmd_get_tx_connection.stream_dest_mac);
        acmp_cmd_get_tx_connection.connection_count = 0;
        // Fill acmp_cmd_get_tx_connection.sequence_id in AEM Controller State Machine
        acmp_cmd_get_tx_connection.flags = 0;
        acmp_cmd_get_tx_connection.stream_vlan_id = 0;

        /****************** Fill frame payload with AECP data and send the frame *****************/
        acmp_controller_state_machine_ref->ether_frame_init(cmd_frame);
        acmp_cmd_get_tx_connection_returned = jdksavdecc_acmpdu_write(&acmp_cmd_get_tx_connection,
                                                                      cmd_frame->payload,
                                                                      ETHER_HDR_SIZE,
                                                                      sizeof(cmd_frame->payload));

        if(acmp_cmd_get_tx_connection_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "cmd_get_tx_connection_write error\n");
            assert(acmp_cmd_get_tx_connection_returned >= 0);
            return -1;
        }

        acmp_controller_state_machine_ref->common_hdr_init(JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_COMMAND, cmd_frame);
        system_queue_tx(notification_id, CMD_WITH_NOTIFICATION, cmd_frame->payload, cmd_frame->length);

        free(cmd_frame);
        return 0;
    }

    int stream_output_descriptor_imp::proc_get_tx_connection_resp(void *&notification_id, const uint8_t *frame, uint16_t frame_len, int &status)
    {
        struct jdksavdecc_frame *cmd_frame;
        int acmp_cmd_get_tx_connection_resp_returned;
        uint32_t msg_type;

        cmd_frame = (struct jdksavdecc_frame *)malloc(sizeof(struct jdksavdecc_frame));
        memcpy(cmd_frame->payload, frame, frame_len);
        acmp_cmd_get_tx_connection_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_get_tx_connection_resp,
                                                                          frame,
                                                                          ETHER_HDR_SIZE,
                                                                          frame_len);

        if(acmp_cmd_get_tx_connection_resp_returned < 0)
        {
            log_imp_ref->post_log_msg(LOGGING_LEVEL_ERROR, "acmp_cmd_get_tx_connection_read error");
            assert(acmp_cmd_get_tx_connection_resp_returned >= 0);
            return -1;
        }

        msg_type = acmp_cmd_get_tx_connection_resp.header.message_type;
        status = acmp_cmd_get_tx_connection_resp.header.status;

        acmp_controller_state_machine_ref->state_resp(notification_id, msg_type, cmd_frame);

        free(cmd_frame);
        return 0;
    }
}
