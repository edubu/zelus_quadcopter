/**
 * <input_manager.h>
 *
 * @brief   Functions to start and stop the input manager thread which is the translation
 * beween control inputs from DSM to the user_input struct which is read by the
 * setpoint manager. TODO: Allow other inputs such as mavlink
 *
 * @addtogroup InputManager
 * @{
 */

#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__

#include <feedback.h>  // only for arm_state_t  TODO: get rid of cross-polination
#include <flight_mode.h>

/**
 * @brief   determines how the dsm radio indicates an arm/disarm kill switch
 */
typedef enum dsm_kill_mode_t
{
    /**
     * A dedicated channel is used as a kill switch. Carefully set the
     * dsm_kill_ch and dsm_kill_pol channel and polarity settings.
     */
    DSM_KILL_DEDICATED_SWITCH,
    /**
     * Some radios, such as Spektrum DXe have an ARM/DISARM switch which
     * forces the throttle channel down below normal range to disarm. This
     * frees up a channel for other use and is the preffered method. When
     * using this mode, dsm_kill_ch and dsm_kill_pol are ignored.
     */
    DSM_KILL_NEGATIVE_THROTTLE
} dsm_kill_mode_t;

/**
 * @brief   Current user input to system
 *
 * Represents current command by the user. This is populated by the
 * input_manager thread which decides to read from mavlink or DSM depending on
 * what it is receiving.
 */
typedef struct user_input_t
{
    int initialized;                 ///< set to 1 after input_manager_init(void)
    flight_mode_t flight_mode;       ///< this is the user commanded flight_mode.
    flight_mode_t prev_flight_mode;  ///< this is the flight mode from the previous loop
    int input_active;                ///< nonzero indicates some user control is coming in
    int kill_switch;                 ///< For printing
    arm_state_t requested_arm_mode;  ///< set to ARMED after arming sequence is entered.

    // All sticks scaled from -1 to 1
    double thr_stick;    ///< positive forward
    double yaw_stick;    ///< positive to the right, CW yaw
    double roll_stick;   ///< positive to the right
    double pitch_stick;  ///< positive forward
} user_input_t;

extern user_input_t user_input;

/**
 * @brief   Starts an input manager thread.
 *
 * Watch for new DSM data and translate into local user mode. Used
 * in input_manager.c
 *
 * @return  0 on success, -1 on failure
 */
int input_manager_init(void);

/**
 * @brief   Waits for the input manager thread to exit
 *
 * This should only be called after the program flow state is set to
 * EXITING as that's the only thing that will cause the thread to
 * exit on its own safely. Used in input_manager.c
 *
 * @return  0 on clean exit, -1 if exit timed out.
 */
int input_manager_cleanup(void);

/**
 * @brief Applies a dead zone to an input stick in.
 *
 * in is supposed to range from -1 to 1
 * the dead zone is centered around 0. zone specifies the distance from 0 the
 * zone extends.
 *
 * @return  in value with applied deadzone
 */
double deadzone(double in, double zone);

/**
 * float apply_deadzone(float in, float zone)
 *
 * 
 **/

int input_manager_cleanup(void);

#endif /* __INPUT_MANAGER__ */

/**@}end group InputMaganger */