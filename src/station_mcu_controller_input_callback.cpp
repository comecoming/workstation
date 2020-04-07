#include "station_mcu_controller_input_callback.h"
#include "ros/ros.h"

enum cmd_message {
	FUN_OUTPUT,
	FUN_INPUT,
	FUN_PWM_DUTY,
	FUN_PWM_CYCLE,
	FUN_INPUT_R,
};

station_mcu_controller_input_callback::station_mcu_controller_input_callback()
{

}

station_mcu_controller_input_callback::~station_mcu_controller_input_callback()
{

}

void station_mcu_controller_input_callback::callback(const scrub_robot_msgs::McuController::ConstPtr& msg)
{
        //ROS_INFO("station_mcu_controller_input_callback %d %d %d", msg->cmd, msg->addr, msg->val);

		if (msg->cmd != FUN_INPUT_R);//TODO.

		m_station_mcu_controller->set_readback(msg->addr, msg->val);

}

void station_mcu_controller_input_callback::station_mcu_controller_input_callback_set_controller(station_mcu_controller *p_station_mcu_controller)
{
	m_station_mcu_controller = p_station_mcu_controller;
}


