#include "station_supply_stop_service.h"

#include "scrub_robot_msgs/WorkstationSupplyStopSrv.h"

station_supply_stop_service::station_supply_stop_service(ros::NodeHandle *n):n(n)
{
}

station_supply_stop_service::~station_supply_stop_service()
{
}

int station_supply_stop_service::init(station *p_station)
{
    int ret = 0;
    try
    {
        service = n->advertiseService("workstation/srv/supply/stop",
                                &station_supply_stop_service_callback::callback, &cb);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("advertiseService "
                         << "workstation/srv/supply/stop"
                         << " failed! " << e.what());
        return -1;
    }

	cb.m_station = p_station;

    return ret;
}

