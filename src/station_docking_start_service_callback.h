#ifndef __STATION_DOCKING_START_SERVICE_CALLBACK_H__
#define __STATION_DOCKING_START_SERVICE_CALLBACK_H__

#include "scrub_robot_msgs/WorkstationDockingStartSrv.h"
#include "station.h"

class station_docking_start_service_callback {
public:
	station_docking_start_service_callback();
	~station_docking_start_service_callback();
	//TODO. private
	bool callback(scrub_robot_msgs::WorkstationDockingStartSrv::Request &req, scrub_robot_msgs::WorkstationDockingStartSrv::Response &resp);
	station *m_station;
private:
};


#endif
