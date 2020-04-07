#include "station_mcu_controller.h"

enum cmd_message {
	FUN_OUTPUT,
	FUN_INPUT,
	FUN_PWM_DUTY,
	FUN_PWM_CYCLE,
	FUN_INPUT_R,
};

station_mcu_controller::station_mcu_controller()
{
}

station_mcu_controller::~station_mcu_controller()
{
}

int station_mcu_controller::init(ros::NodeHandle *n)
{

    try
    {
        pub = n->advertise<scrub_robot_msgs::McuController>("/io_out", 1000);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("int advertise topic " << e.what() << " failed!");        
        return -1;
    }


	//TODO. readback all

    return 0;
}

void station_mcu_controller::set_readback(int id, int val)
{
printf("%s %d %d\n", __func__, id, val);
	readback[id] = val;
}

int  station_mcu_controller::get_readback(int id)
{
	printf("%s %d %d\n", __func__, id, readback[id]);
	return readback[id];
}

int station_mcu_controller::getDin(int id)
{
    scrub_robot_msgs::McuController msg;
 
 	msg.cmd = FUN_INPUT;
	msg.addr = id;
	//msg.val = status;

    usleep(50000);
    pub.publish(msg);
    usleep(50000);


	//sleep(2);//TODO. totally unstable
	return get_readback(id);

}


void station_mcu_controller::setDout(int id, int status)
{
    scrub_robot_msgs::McuController msg;
 
 	msg.cmd = FUN_OUTPUT;
	msg.addr = id;
	msg.val = status;

    usleep(50000);
    pub.publish(msg);
    usleep(50000);

}

void station_mcu_controller::setPwm(int id, int duty, int cycle)
{
    scrub_robot_msgs::McuController msg;
	
 	msg.cmd = FUN_PWM_DUTY;
	msg.addr = id;
	msg.val = duty;

    usleep(50000);
    pub.publish(msg);
    usleep(50000);

	
 	msg.cmd = FUN_PWM_CYCLE;
	msg.addr = id;
	msg.val = cycle;

    usleep(50000);
    pub.publish(msg);
    usleep(50000);

}

