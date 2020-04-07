#ifndef __STATION_SUPPLY_START_SERVICE_CALLBACK_H__
#define __STATION_SUPPLY_START_SERVICE_CALLBACK_H__

#include "scrub_robot_msgs/WorkstationSupplyStartSrv.h"
#include "station.h"

class station_supply_start_service_callback {
public:
	station_supply_start_service_callback();
	~station_supply_start_service_callback();
	
	bool callback(scrub_robot_msgs::WorkstationSupplyStartSrv::Request &req, scrub_robot_msgs::WorkstationSupplyStartSrv::Response &resp);
	station *m_station;
private:
};


#endif
