#include "station_supply_start_service_callback.h"

station_supply_start_service_callback::station_supply_start_service_callback()
{
}

station_supply_start_service_callback::~station_supply_start_service_callback()
{
}

bool station_supply_start_service_callback::callback(scrub_robot_msgs::WorkstationSupplyStartSrv::Request &req, scrub_robot_msgs::WorkstationSupplyStartSrv::Response &resp)
{
	m_station->thelog.printf("station_supply_start_service_callback");

	/*req.supply_id
	req.device_id
	req.workstation_id
	req.add_water
	req.discharge_water
	req.charge*/
	int status;

	m_station->set_supply_id(req.device_id, req.workstation_id, req.supply_id);
	status = m_station->supply_start(req.add_water, req.discharge_water, req.charge);

	resp.result_code = 0;
	resp.supply_id = req.supply_id;
	resp.supply_status = status;
	
    return true;
}


