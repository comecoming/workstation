#include "station_docking_stop_service_callback.h"

station_docking_stop_service_callback::station_docking_stop_service_callback()
{
}

station_docking_stop_service_callback::~station_docking_stop_service_callback()
{
}

bool station_docking_stop_service_callback::callback(scrub_robot_msgs::WorkstationDockingStopSrv::Request &req, scrub_robot_msgs::WorkstationDockingStopSrv::Response &resp)
{
	m_station->thelog.printf("station_docking_stop_service_callback\n");
	printf("station_docking_stop_service_callback\n");
	/*req.docking_id
	req.device_id
	req.workstation_id*/

	m_station->docking_stop();

	resp.result_code = 0;
	resp.docking_id = req.docking_id;
		return true;


}


