#ifndef __STATION_MCU_CONTROLLER_INPUT_CALLBACK_H__
#define __STATION_MCU_CONTROLLER_INPUT_CALLBACK_H__

#include "scrub_robot_msgs/McuController.h"
#include "station_mcu_controller.h"


class station_mcu_controller_input_callback
{
public:
    station_mcu_controller_input_callback();
    ~station_mcu_controller_input_callback();
    void station_mcu_controller_input_callback_set_controller(station_mcu_controller *p_station_mcu_controller);


	void callback(const scrub_robot_msgs::McuController::ConstPtr& msg);

private:
	station_mcu_controller *m_station_mcu_controller;

};

#endif
