#ifndef __STATION_MCU_CONTROLLER_INPUT_H__
#define __STATION_MCU_CONTROLLER_INPUT_H__

#include "ros/ros.h"
#include "station_mcu_controller_input.h"
#include "station_mcu_controller_input_callback.h"
#include "station_mcu_controller.h"


class station_mcu_controller_input
{

public:
	station_mcu_controller_input();
    ~station_mcu_controller_input();

    int init(ros::NodeHandle *n);
	void set_callback_controller(station_mcu_controller *p_station_mcu_controller);


private:
    ros::Subscriber sub;
	station_mcu_controller_input_callback cb;

};

#endif
