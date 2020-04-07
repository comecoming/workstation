#ifndef __STATION_DOCKING_START_SERVICE_H__
#define __STATION_DOCKING_START_SERVICE_H__

#include "ros/ros.h"
#include "station.h"
#include "station_docking_start_service_callback.h"

class station_docking_start_service {
public:
	station_docking_start_service(ros::NodeHandle *n);
	~station_docking_start_service();
	int init(station *p_station);
	
	station_docking_start_service_callback cb;
private:
	ros::NodeHandle *n;
    ros::ServiceServer service;
	
};

#endif
