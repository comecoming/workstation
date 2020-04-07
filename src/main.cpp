#include <stdio.h>
#include <unistd.h>

#include "station.h"
#include "station_docking_start_service.h"
#include "station_docking_stop_service.h"
#include "station_supply_start_service.h"
#include "station_supply_stop_service.h"

#include "station_mcu_controller.h"
#include "station_mcu_controller_input.h"

unsigned short station::seq = 0x1234;
#define STA_PORT 8899
#define STA_IP "10.10.100.254"


int test()
{
/*printf("%s %d\n", __func__, __LINE__);
	station s;
	s.init("10.10.100.254", 8899);
	printf("%s %d\n", __func__, __LINE__);

	s.send_frame(STA_CONN, NULL, 0);s.read_frame();
	printf("%s %d\n", __func__, __LINE__);
	s.send_frame(STA_CONN_START, NULL, 0);s.read_frame();
	s.send_frame(STA_CONN_FIN, NULL, 0);s.read_frame();
	printf("%s %d\n", __func__, __LINE__);

sleep(1);
	s.send_frame(STA_CHARGE, char * data, unsigned short len);
	s.send_frame(STA_CHARGE_START, char * data, unsigned short len);
	s.send_frame(STA_CHARGE_SUBMIT, char * data, unsigned short len);
	s.send_frame(STA_CHARGE_STOP, char * data, unsigned short len);

	s.send_frame(STA_DETACH, NULL, 0);s.read_frame();
	s.send_frame(STA_DISCONN, NULL, 0);s.read_frame();*/
	}

int main(int argc, char **argv)
{

	//test();
	ros::init(argc, argv, "workstation");
	ros::NodeHandle n;
	ros::Rate loop_rate(1);

	station_mcu_controller g_station_mcu_controller;
	g_station_mcu_controller.init(&n);

	station_mcu_controller_input g_station_mcu_controller_input;
	g_station_mcu_controller_input.init(&n);
	g_station_mcu_controller_input.set_callback_controller(&g_station_mcu_controller);

	
	
	station g_station(&n);
	g_station.set_ip(STA_IP, STA_PORT);
	g_station.set_mcu_controller(&g_station_mcu_controller);

	station_docking_start_service g_station_docking_start_service(&n);
	g_station_docking_start_service.init(&g_station);

	station_docking_stop_service g_station_docking_stop_service(&n);
	g_station_docking_stop_service.init(&g_station);

	station_supply_start_service g_station_supply_start_service(&n);
	g_station_supply_start_service.init(&g_station);
	
	station_supply_stop_service g_station_supply_stop_service(&n);
	g_station_supply_stop_service.init(&g_station);


	
	while (ros::ok()) {
		//TODO. heartbeat here
		ros::spinOnce();
		loop_rate.sleep();
	}

}

