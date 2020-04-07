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


int main(int argc, char **argv)
{

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
		ros::spinOnce();
		loop_rate.sleep();
	}

}

