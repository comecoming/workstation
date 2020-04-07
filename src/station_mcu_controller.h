#ifndef __STATION_MCU_CONTROLLER_H__
#define __STATION_MCU_CONTROLLER_H__

#include "ros/ros.h"
#include "scrub_robot_msgs/McuController.h"

class station_mcu_controller
{

public:
    station_mcu_controller();
    ~station_mcu_controller();

    int init(ros::NodeHandle *n);
	void setDout(int id, int status);
	void setPwm(int id, int duty, int cycle);
	int getDin(int id);
	
	void set_readback(int id, int val);
	int  get_readback(int id);
private:

	
    /* data */
    ros::Publisher pub;
	int readback[20];

	ros::NodeHandle *n;
};


#endif
