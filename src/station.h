#ifndef __STATION_H__
#define __STATION_H__

#include <mutex>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "thelog.h"
#include "ros/ros.h"
#include "station_mcu_controller.h"

enum sta_cmd {
	STA_CONN = 0x1001,
	STA_CONN_R,
	STA_CONN_START,
	STA_CONN_START_R,
	STA_CONN_FIN,
	STA_CONN_FIN_R,

	STA_CHARGE,
	STA_CHARGE_R,
	STA_CHARGE_START,
	STA_CHARGE_START_R,
	STA_CHARGE_STOP,
	STA_CHARGE_STOP_R,
	STA_CHARGE_SUBMIT,
	STA_CHARGE_SUBMIT_R,

	STA_WATER,
	STA_WATER_R,
	STA_WATER_START,
	STA_WATER_START_R,
	STA_WATER_STOP,
	STA_WATER_STOP_R,
	STA_WATER_SUBMIT,
	STA_WATER_SUBMIT_R,

	STA_DIRTY,
	STA_DIRTY_R,
	STA_DIRTY_START,
	STA_DIRTY_START_R,
	STA_DIRTY_STOP,
	STA_DIRTY_STOP_R,
	STA_DIRTY_SUBMIT,
	STA_DIRTY_SUBMIT_R,

	STA_DETACH,
	STA_DETACH_R,
	STA_DISCONN,
	STA_DISCONN_R,

	STA_HEARTBEAT,
	STA_HEARTBEAT_R,

	STA_RESET = 0x2001,

};


enum docking_status {
	DOCKING_STATUS_FAILED = -1,
	DOCKING_STATUS_ALLOW,
	DOCKING_STATUS_CONNECTING,
	DOCKING_STATUS_FORBIDDEN,
	DOCKING_STATUS_COMPLETED,

};

enum docking_thread_status {
	DOCKING_THREAD_RUNNING,
	DOCKING_THREAD_STOP,

};



enum heartbeat_status {
	HEARTBEAT_RUNNING,
	HEARTBEAT_STOPING,
	HEARTBEAT_STOPED,
};




enum read_frame_status {
	READFRAME_RUNNING,
	READFRAME_STOPING,
	READFRAME_STOPED,
};


enum link_status {
	LINK_NORMAL,
	LINK_CONNECTING,
	LINK_DISCONNECTED,
	LINK_FAILED,
};

	



enum supply_status {
	SUPPLY_STATUS_FAIL = -1,
	SUPPLY_STATUS_STOP,
	SUPPLY_STATUS_RUN,
	SUPPLY_STATUS_FIN,

};

enum supply_thread_status {
	SUPPLY_THREAD_RUNNING,
	SUPPLY_THREAD_STOP,

};

enum supply_mission {
	MISSION_CHARGE,
	MISSION_WATER,
	MISSION_DIRTY,
};


class station {
	
public:
	station(ros::NodeHandle *n);
	~station();
	int init(const char* ip, unsigned short port);
	void set_ip(const char* ip, unsigned short port);
	void set_mcu_controller(station_mcu_controller *p_station_mcu_controller);

	int station_read(int fd, void *buf, int count);
	int station_write(int fd, const void *buf, int count);
	int send_frame(unsigned short cmd, char *data, unsigned short len);
	int send_frame(unsigned short cmd);
	struct station_frame read_frame();
	unsigned short get_frame_cmd(struct station_frame sf);
	int fetch_frame(int cmd);
	int reconnect();





	int read_frame_get_status();
	void read_frame_set_status(read_frame_status status);
	void read_frame_start();
	void read_frame_stop();


	




	int heartbeat_get_status();
	void heartbeat_set_status(heartbeat_status status);
	void heartbeat_start();
	void heartbeat_stop();




	int link_get_status();
		void link_set_status(link_status status);



	
	int docking_stop();
	int docking_start();
	void docking_set_and_pub_status(docking_status status);
	void docking_pub_status();
	void docking_set_status(docking_status status);
	int docking_get_status();
	void docking_stop_thread_set_status(docking_thread_status status);
	int docking_stop_thread_get_status();
	void docking_start_thread_set_status(docking_thread_status status);
	int docking_start_thread_get_status();
	void set_docking_id(std::string p_device_id, std::string p_workstation_id, std::string p_docking_id);






	int supply_start(int water, int dirty, int charge);
	int supply_stop();
	void supply_mission_start(int mission);
	void supply_mission_stop(int mission);
	int supply_get_thread_status(int mission);
	void supply_set_thread_status(int mission, supply_thread_status status);
	int supply_get_status(int mission);
	void supply_set_status(int mission, supply_status status);
	void supply_pub_status();
	void supply_set_and_pub_status(int mission, supply_status status);
	void set_supply_id(std::string p_device_id, std::string p_workstation_id, std::string p_supply_id);




	
//private:
	int station_sock;
	struct sockaddr_in sock_sta;
	socklen_t len;

	std::string supply_id;
	std::string docking_id;
	std::string device_id;
	std::string workstation_id;
	std::string map_id;
	static unsigned short seq;

	ros::NodeHandle *n;

	std::mutex mutex_fetch_frame;

//private:
	link_status m_link_status;

	read_frame_status m_read_frame_status;
	heartbeat_status m_heartbeat_status;

	supply_status m_supply_status[10];
	supply_thread_status m_supply_thread_status[10];

	
	docking_status m_docking_status;
	docking_thread_status m_docking_start_thread_status;
	docking_thread_status m_docking_stop_thread_status;
	
	ros::Publisher docking_status_pub;
	ros::Publisher supply_status_pub;

	station_mcu_controller *m_station_mcu_controller;


	theLog thelog;
};

struct station_frame {
	unsigned short len;
	unsigned short seq;
	unsigned short cmd;
	char data[10];

};


#endif
