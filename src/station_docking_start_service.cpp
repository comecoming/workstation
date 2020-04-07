#include "station_docking_start_service.h"

#include "scrub_robot_msgs/WorkstationDockingStartSrv.h"

station_docking_start_service::station_docking_start_service(ros::NodeHandle *n):n(n)
{
}

station_docking_start_service::~station_docking_start_service()
{
}

int station_docking_start_service::init(station *p_station)
{
    int ret = 0;
    try
    {
        service = n->advertiseService("workstation/srv/docking/start",
                                &station_docking_start_service_callback::callback, &cb);
    }
    catch (const ros::InvalidNameException &e)
    {
        ROS_ERROR_STREAM("advertiseService "
                         << "workstation/srv/docking/start"
                         << " failed! " << e.what());
        return -1;
    }

	cb.m_station = p_station;

    return ret;
}

