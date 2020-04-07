#include "station_docking_stop_service.h"

#include "scrub_robot_msgs/WorkstationDockingStopSrv.h"

station_docking_stop_service::station_docking_stop_service(ros::NodeHandle *n):n(n)
{
}

station_docking_stop_service::~station_docking_stop_service()
{
}

int station_docking_stop_service::init(station *p_station)
{
    int ret = 0;
    try
    {
        service = n->advertiseService("workstation/srv/docking/stop",
                                &station_docking_stop_service_callback::callback, &cb);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("advertiseService "
                         << "workstation/srv/docking/stop"
                         << " failed! " << e.what());
        return -1;
    }

	cb.m_station = p_station;

    return ret;
}

