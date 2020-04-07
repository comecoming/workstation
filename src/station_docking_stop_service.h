#ifndef __STATION_DOCKING_STOP_SERVICE_H__
#define __STATION_DOCKING_STOP_SERVICE_H__

#include "ros/ros.h"

#include "station_docking_stop_service_callback.h"
#include "station.h"

class station_docking_stop_service {
	public:
		station_docking_stop_service(ros::NodeHandle *n);
		~station_docking_stop_service();
		int init(station *p_station);
		
		station_docking_stop_service_callback cb;
	private:
		ros::NodeHandle *n;
		ros::ServiceServer service;
};

#endif

