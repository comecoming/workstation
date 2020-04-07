#include "station_mcu_controller_input.h"

station_mcu_controller_input::station_mcu_controller_input()
{
}

station_mcu_controller_input::~station_mcu_controller_input()
{
}

int station_mcu_controller_input::init(ros::NodeHandle *n)
{


    try
    {
		sub = n->subscribe("/io_in", 1000, &station_mcu_controller_input_callback::callback, &cb);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("int advertise topic " << e.what() << " failed!");        
        return -1;
    }

    return 0;
}

void station_mcu_controller_input::set_callback_controller(station_mcu_controller *p_station_mcu_controller)
{
	cb.station_mcu_controller_input_callback_set_controller(p_station_mcu_controller);
}


