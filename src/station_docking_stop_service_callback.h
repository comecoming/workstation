#ifndef __SUPPLEMENT_SERVICE_STOP_CALLBACK_H__
#define __SUPPLEMENT_SERVICE_STOP_CALLBACK_H__

#include "scrub_robot_msgs/WorkstationDockingStopSrv.h"
#include "station.h"

class station_docking_stop_service_callback {
public:
	station_docking_stop_service_callback();
	~station_docking_stop_service_callback();
	station *m_station;
	bool callback(scrub_robot_msgs::WorkstationDockingStopSrv::Request &req, scrub_robot_msgs::WorkstationDockingStopSrv::Response &resp);
	private:
};


#endif
