/**
 * @file mavlink_manager.c
 */

#include <rcp/core/mavlink_manager.h>
#include <rc/math/quaternion.h>
#include <rc/mavlink_udp.h>
#include <rc/time.h>
#include <rcp/core/settings.h>
#include <rcp/core/state_estimator.h>
#include <stdio.h>

#define LOCALHOST_IP "127.0.0.1"
#define DEFAULT_SYS_ID 1

/**
 * Functions to only be used locally
 */
static void __callback_func_mocap(void);

static void __callback_func_mocap(void)
{
    int i;
    mavlink_att_pos_mocap_t data;

    if (rc_mav_get_att_pos_mocap(&data) < 0)
    {
        fprintf(stderr, "ERROR in mavlink manager, problem fetching att_pos_mocal packet\n");
        return;
    }

    // check if position is 0 0 0 which indicates mocap system is alive but
    // has lost visual contact on the object
    if (fabs(data.x) < 0.0001 && fabs(data.y) < 0.0001 && fabs(data.z) < 0.0001)
    {
        if (state_estimate.mocap_running == 1)
        {
            state_estimate.mocap_running = 0;
            if (settings.warnings_en)
            {
                fprintf(stderr, "WARNING, MOCAP LOST VISUAL\n");
            }
        }
        else
        {
            state_estimate.is_active = 0;
        }
        return;
    }

    // copy data
    for (i = 0; i < 4; i++) state_estimate.quat_mocap[i] = (double)data.q[i];
    // normalize quaternion because we don't trust the mocap system
    rc_quaternion_norm_array(state_estimate.quat_mocap);
    // calculate tait bryan angles too
    rc_quaternion_to_tb_array(state_estimate.quat_mocap, state_estimate.tb_mocap);
    // position
    state_estimate.pos_mocap[0] = (double)data.x;
    state_estimate.pos_mocap[1] = (double)data.y;
    state_estimate.pos_mocap[2] = (double)data.z;
    // mark timestamp
    state_estimate.mocap_timestamp_ns = rc_nanos_since_epoch();
    state_estimate.mocap_running = 1;

    return;
}

int mavlink_manager_init(void)
{
    // initialize the UDP port and listening thread with the rc_mav lib
    if (rc_mav_init(settings.my_sys_id, settings.dest_ip, settings.mav_port,
            RC_MAV_DEFAULT_CONNECTION_TIMEOUT_US) < 0)
    {
        fprintf(stderr, "ERROR: Mavlink manager failed to initilize\n");
        return -1;
    }
    // set the mocap callback to record position
    rc_mav_set_callback(MAVLINK_MSG_ID_ATT_POS_MOCAP, __callback_func_mocap);
    return 0;
}

int cleanup_mavlink_manager(void)
{
    return rc_mav_cleanup();
}