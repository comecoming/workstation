#include "station_docking_start_service_callback.h"

station_docking_start_service_callback::station_docking_start_service_callback()
{
}

station_docking_start_service_callback::~station_docking_start_service_callback()
{
}

bool station_docking_start_service_callback::callback(scrub_robot_msgs::WorkstationDockingStartSrv::Request &req, scrub_robot_msgs::WorkstationDockingStartSrv::Response &resp)
{

	m_station->thelog.printf("station_docking_start_service_callback");
	/*req.docking_id
	req.device_id
	req.workstation_id*/
	int status;
	int val;
	
	//machine position
	/*val = m_station->m_station_mcu_controller->getDin(10);
	ROS_INFO("station_docking_start_service_callback getDin:%d", val);
	if (val != 0) {
		resp.result_code = 0;
		resp.docking_id = req.docking_id;
		resp.docking_status = -1;//0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�-1����ʧ��
		
		return true;
	}*///TODO. io bug

	m_station->set_docking_id(req.device_id, req.workstation_id, req.docking_id);
	status = m_station->docking_start();

	//docking srv״̬��0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�-1����ʧ��
	//docking msg״̬:0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�3�Խ���ɣ�?-1����ʧ��

	
	resp.result_code = 0;
	resp.docking_id = req.docking_id;
	resp.docking_status = status;//0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�-1����ʧ��
	
    return true;
}





