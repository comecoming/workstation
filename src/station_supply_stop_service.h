#ifndef __STATION_SUPPLY_STOP_SERVICE_H__
#define __STATION_SUPPLY_STOP_SERVICE_H__

#include "ros/ros.h"

#include "station_supply_stop_service_callback.h"
#include "station.h"

class station_supply_stop_service {
public:
	station_supply_stop_service(ros::NodeHandle *n);
	~station_supply_stop_service();
	int init(station *p_station);
	
	station_supply_stop_service_callback cb;
private:
	ros::NodeHandle *n;
    ros::ServiceServer service;
};

#endif
