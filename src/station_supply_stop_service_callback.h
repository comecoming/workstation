#ifndef __STATION_SUPPLY_STOP_SERVICE_CALLBACK_H__
#define __STATION_SUPPLY_STOP_SERVICE_CALLBACK_H__

#include "scrub_robot_msgs/WorkstationSupplyStopSrv.h"
#include "station.h"

class station_supply_stop_service_callback {
public:
	station_supply_stop_service_callback();
	~station_supply_stop_service_callback();
	bool callback(scrub_robot_msgs::WorkstationSupplyStopSrv::Request &req, scrub_robot_msgs::WorkstationSupplyStopSrv::Response &resp);
	station *m_station;

private:
};


#endif
