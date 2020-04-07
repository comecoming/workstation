#include "station_supply_stop_service_callback.h"

station_supply_stop_service_callback::station_supply_stop_service_callback()
{
}

station_supply_stop_service_callback::~station_supply_stop_service_callback()
{
}

bool station_supply_stop_service_callback::callback(scrub_robot_msgs::WorkstationSupplyStopSrv::Request &req, scrub_robot_msgs::WorkstationSupplyStopSrv::Response &resp)
{
	m_station->thelog.printf("station_supply_stop_service_callback");

	/*req.supply_id
	req.device_id
	req.workstation_id*/

	int status;
	status = m_station->supply_stop();
	
	
	resp.result_code = status;
	resp.supply_id = req.supply_id;
	
    return true;
}


