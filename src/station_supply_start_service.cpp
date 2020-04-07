#include "station_supply_start_service.h"

#include "scrub_robot_msgs/WorkstationSupplyStartSrv.h"

station_supply_start_service::station_supply_start_service(ros::NodeHandle *n):n(n)
{
}

station_supply_start_service::~station_supply_start_service()
{
}

int station_supply_start_service::init(station *p_station)
{
    int ret = 0;
    try
    {
        service = n->advertiseService("workstation/srv/supply/start",
                                &station_supply_start_service_callback::callback, &cb);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("advertiseService "
                         << "workstation/srv/supply/start"
                         << " failed! " << e.what());
        return -1;
    }

	cb.m_station = p_station;

    return ret;
}

