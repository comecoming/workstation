#include <thread>

//reconnect
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include "station.h"
#include "scrub_robot_msgs/WorkstationDockingStatus.h"
#include "scrub_robot_msgs/WorkstationSupplyStatus.h"

#define OFFSET_LEN 2
#define OFFSET_SEQ 4
#define OFFSET_SID 6
#define OFFSET_DID 22
#define OFFSET_CMD 38
#define OFFSET_DAT 40

static void heartbeat_thread(station *m_station);

static void docking_start_thread(station *p_station);
static void docking_stop_thread(station *p_station);

static void read_frame_thread(station *p_station);


//TODO. diff:mission, timeout, frame cmd
static void supply_water_thread(int mission, int timeout, station *p_station);
static void supply_dirty_thread(int mission, int timeout, station *p_station);
static void supply_charge_thread(int mission, int timeout, station *p_station);
//static void supply_mission_thread(int mission, int timeout, station *p_station);


station::station(ros::NodeHandle *n):n(n)
{
	seq = 0;
	
	docking_stop_thread_set_status(DOCKING_THREAD_STOP);
	docking_start_thread_set_status(DOCKING_THREAD_STOP);
	docking_set_status(DOCKING_STATUS_FAILED);//TODO. should be DOCKING_STATUS_STOP

	supply_stop();

	//publish docking/supply status
	docking_status_pub = n->advertise<scrub_robot_msgs::WorkstationDockingStatus>("workstation/msg/docking/status", 1000);
	supply_status_pub = n->advertise<scrub_robot_msgs::WorkstationSupplyStatus>("workstation/msg/supply/status", 1000);

}

station::~station()
{

	if (station_sock > 0)
		close(station_sock);
}



int station::init(const char* ip, unsigned short port)
{


}

void station::set_ip(const char* ip, unsigned short port)
{
	
	sock_sta.sin_family = AF_INET;
	sock_sta.sin_port = htons(port);
	sock_sta.sin_addr = {.s_addr = inet_addr(ip)};
	len = sizeof(sock_sta);

}

void station::set_mcu_controller(station_mcu_controller *p_station_mcu_controller)
{
	m_station_mcu_controller = p_station_mcu_controller;

}

int station::station_write(int fd, const void *buf, int count)
{
	int ret = 0;
	int retry;
	ros::Rate loop_rate(1);

	retry = 3;
	while (ros::ok()) {
		if (retry-- <= 0)
			break;
	
		ret = write(fd, buf, count);

		if (ret >= 0) {
			break;
		} else {
			loop_rate.sleep();
			continue;
		}

	}

	return ret;
}

int station::send_frame(unsigned short cmd, char *data, unsigned short len)
{//TODO. retry mechanism
	unsigned char buf[1024] = {0};
	unsigned char crc = 0;
	int i;
	int ret;

	memset(buf, 0, 1024);
	buf[0] = 0x3b;
	buf[1] = 0xb3;
	*(unsigned short *)(buf+OFFSET_LEN) = len+37;//*(unsigned short *)(buf+OFFSET_LEN) = htons(len+37);
	*(unsigned short *)(buf+OFFSET_SEQ) = seq;//*(unsigned short *)(buf+OFFSET_SEQ) = htons(seq);
	*(unsigned short *)(buf+OFFSET_CMD) = cmd;//*(unsigned short *)(buf+OFFSET_CMD) = htons(cmd);
	if (device_id.size() < 16);//TODO.
	memcpy(buf+OFFSET_SID, device_id.c_str(), 16);
	memcpy(buf+OFFSET_DID, workstation_id.c_str(), 16);
	//device_id.copy((char *)&buf[OFFSET_SID], 0, 16);
	//workstation_id.copy((char *)&buf[OFFSET_DID], 0, 16);
	
	if (data)
		memcpy(buf+OFFSET_DAT, data, len);
	else
		len = 0;

	for (i = 0; i < len+37-1; i++) {
		crc ^= buf[i+4];
	}
	buf[OFFSET_DAT+len] = crc;

	ret = station_write(station_sock, buf, len+4+37);
	
	/*printf("write %d bytes: [", ret);
	for (i = 0; i < len+4+37; i++) {
		printf("%02x ", buf[i]);

	}
	printf("]\n\n");*/
	
	return ret;

}

int station::send_frame(unsigned short cmd)
{
	return send_frame(cmd, NULL, 0);
}

int station::station_read(int fd, void *buf, int count)
{
	int ret = 0;
	int retry;
	struct timeval tm;
	tm.tv_sec = 1;
	tm.tv_usec = 0;//100*1000;
	fd_set set_read;
	FD_ZERO(&set_read);
	FD_SET(fd, &set_read);

	retry = 3;
	while (ros::ok()) {
		if (retry-- <= 0)
			break;
	
		if (0 >= select(fd + 1, &set_read, NULL, NULL, &tm)) {
			ret = 0;
			continue;
		}
		
		if (FD_ISSET(fd, &set_read)) {
			ret = read(fd, buf, count);
		}
		if (ret >= 0)
			break;

	}

	return ret;

}


struct station_frame station::read_frame()
{//TODO. retry mechanism
	unsigned char head1, head2;
	unsigned short len;
	int offset;
	int ret;
	unsigned char buf[100];
	struct station_frame sf;

	ros::Rate loop_rate(100);
	
	int nothead = 0;
	while (ros::ok()) {
		if (nothead > 50) {
			memset(&sf, 0, sizeof(sf));
			return sf;
		}
		
		station_read(station_sock, &head1, 1);//printf("read head1:%x\n", head1);
		if (head1 != 0x3b) {
			printf("read %x except head1\n", head1);
			nothead++;
			loop_rate.sleep();
			continue;
		}
		
		station_read(station_sock, &head2, 1);//printf("read head2:%x\n", head2);
		if (head2 != 0xb3) {
			printf("read %x except head1\n", head2);
			nothead++;
			loop_rate.sleep();
			continue;
		}

		station_read(station_sock, &len, 2);

		offset = 0;
		while (offset < len) {
			ret = station_read(station_sock, buf+offset, len);
			if (ret <= 0) break;//TODO. handle lamp
			offset += ret;
		}

		sf.len = len-37;
		sf.seq = *(unsigned short *)(buf+OFFSET_SEQ-OFFSET_SEQ);
		sf.cmd = *(unsigned short *)(buf+OFFSET_CMD-OFFSET_SEQ);
		if (sf.len > 0)
			memcpy(sf.data, buf+OFFSET_DAT-OFFSET_SEQ, sf.len);


		
		/*int i;
		printf("read:%d  [", len);
		for (i = 0; i < len; i++) {
			printf("%02x ", buf[i]);
		}
		printf("]\n");


		
		printf("cmd:%04x seq:%04x %d bytes:", 
			sf.cmd, sf.seq, sf.len);
		for (i = 0; i < sf.len; i++) {
			printf("%02x ", buf[i+OFFSET_DAT-OFFSET_SEQ]);
		}
		printf("\n\n");*/

		break;
		
	}
	return sf;
}

unsigned short station::get_frame_cmd(struct station_frame sf)
{
	return sf.cmd;

}

int station::fetch_frame(int cmd)
{//TODO. reconnect here
thelog.printf("fetching frame %x\n", cmd);
	mutex_fetch_frame.lock();
	send_frame(cmd);
	/*if (cmd+1 != get_frame_cmd(read_frame())) {
		mutex_fetch_frame.unlock();
		return -1;
	}*/
	
	mutex_fetch_frame.unlock();
	return 0;

}



int station::read_frame_get_status()
{
	return m_read_frame_status;
}

void station::read_frame_set_status(read_frame_status status)
{
	m_read_frame_status = status;
}

void station::read_frame_start()
{
	read_frame_set_status(READFRAME_RUNNING);

	std::thread pid_read_frame(read_frame_thread, this);
	pid_read_frame.detach();
}

void station::read_frame_stop()
{
	read_frame_set_status(READFRAME_STOPING);
}


static void read_frame_thread(station *p_station)
{
	unsigned short cmd;
	
	while (ros::ok()) {
		
		if (p_station->read_frame_get_status() == READFRAME_STOPING)
			break;
		
		cmd = p_station->get_frame_cmd(p_station->read_frame());
		if (cmd == 0) continue;
		p_station->thelog.printf("get_frame_cmd %x\n", cmd);

		switch (cmd) {
		case STA_CHARGE_STOP:
		case STA_WATER_STOP:
		case STA_DIRTY_STOP:
			p_station->supply_stop();
			break;
		default:
			break;
		}
	}
	
	p_station->read_frame_set_status(READFRAME_STOPED);
}


int station:: reconnect()
{//TODO. retry mechanism, put giant lock on this  code
	int flag;	
	int retry;
	int len;
	int error;
	struct timeval tm;
/*
	set_supply_status(MISSION_LINK, LINK_CONNECTING);
	pub_supply_status();
	
	mutex_fetch_frame.lock();
	retry = 3;
	while (retry--) {


		if (station_sock > 0) {
			close(station_sock);
		}
		
		station_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (station_sock < 0) {
			perror("reconnect socket");
			sleep(1);
			continue;
		}
		
		flag = fcntl(station_sock, F_GETFD, 0);
		flag |= O_NONBLOCK;
		fcntl(station_sock, F_SETFD, flag);
		if (connect(station_sock, (struct sockaddr *)&sock_sta, len) < 0 && errno != EINPROGRESS) {
			perror("reconnect connect");
			sleep(1);
			continue;
		}

		tm.tv_sec = 1;
		tm.tv_usec = 0;
		fd_set w;
		FD_ZERO(&w);
		FD_SET(station_sock, &w);
		if (0 < select(station_sock + 1, &w, NULL, NULL, &tm)) {
			if (0 == FD_ISSET(station_sock, &w)) {
				sleep(1);
				continue;
			}
		} else {
			//already block
			continue;
		}

		getsockopt(station_sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
		if (error != 0) {
			sleep(1);
			continue;
		}

	}

	if (retry = 0) {
		mutex_fetch_frame.unlock();
		set_supply_status(MISSION_LINK, LINK_FAILED);
		pub_supply_status();
		return -1;
	}

	
	flag = fcntl(station_sock, F_GETFD, 0);
	flag &= ~O_NONBLOCK;
	fcntl(station_sock, F_SETFD, flag);
	mutex_fetch_frame.unlock();
	
	set_supply_status(MISSION_LINK, LINK_NORMAL);
	pub_supply_status();*/

	return 0;

}


int station::link_get_status()
{
	return m_link_status;
}

void station::link_set_status(link_status status)
{
	m_link_status = status;
}















int station::heartbeat_get_status()
{
	return m_heartbeat_status;
}

void station::heartbeat_set_status(heartbeat_status status)
{
	m_heartbeat_status = status;
}

void station::heartbeat_start()
{
	heartbeat_set_status(HEARTBEAT_RUNNING);

	std::thread pid_heartbeat(heartbeat_thread, this);
	pid_heartbeat.detach();
}

void station::heartbeat_stop()
{
	heartbeat_set_status(HEARTBEAT_STOPING);
}

static void heartbeat_thread(station *p_station)
{
//link_status 0����������1���������У�2���������Ͽ���-1����ʧ��
	ros::Rate loop_rate(1);
	int failtime = 0;
	
	while (ros::ok()) {
		
		if (p_station->heartbeat_get_status() == HEARTBEAT_STOPING)
			break;
	
		if (p_station->fetch_frame(STA_HEARTBEAT) < 0) {
			printf("lost one heartbeat......\n");
			failtime++;
		} else {
			failtime = 0;
		}

		if (failtime > 3) {
			printf("reconnect proceduce.....\n");
			/*if (0 > p_station->reconnect()) {
				p_station->docking_stop();
				p_station->supply_stop();
				break;
			}*/
		}

		/*workstation::workstation_docking_status docking_status_msg;
		workstation::workstation_supply_status supply_status_msg;
		
		docking_status_msg.docking_id;//TODO. stuff id
		docking_status_msg.device_id;
		docking_status_msg.workstation_id;
		docking_status_msg.docking_status = m_station->docking_status;

		
		supply_status_msg.supply_id;//TODO. stuff id and status
		supply_status_msg.device_id;
		supply_status_msg.workstation_id;
		supply_status_msg.link_status;
		supply_status_msg.add_water_status;
		supply_status_msg.discharge_water_status;
		supply_status_msg.charge_status = m_station->supply_status;

		
		m_station->docking_status_pub.publish(docking_status_msg);
		m_station->supply_status_pub.publish(supply_status_msg);*/

		/*int i = 60;
		while (i--) {
			if (p_station->heartbeat_get_status() == HEARTBEAT_STOPING)
				break;
			loop_rate.sleep();
		}*/


		loop_rate.sleep();
	}

	p_station->heartbeat_set_status(HEARTBEAT_STOPED);
	
}















void station::set_docking_id(std::string p_device_id, std::string p_workstation_id, std::string p_docking_id)
{
	device_id = p_device_id;
	workstation_id = p_workstation_id;
	docking_id = p_docking_id;
}

int station::docking_start_thread_get_status()
{
	return m_docking_start_thread_status;
}

void station::docking_start_thread_set_status(docking_thread_status status)
{
	m_docking_start_thread_status = status;
}

int station::docking_stop_thread_get_status()
{
	return m_docking_stop_thread_status;
}

void station::docking_stop_thread_set_status(docking_thread_status status)
{
	m_docking_stop_thread_status = status;
}

int station::docking_get_status()
{
	return m_docking_status;
}

void station::docking_set_status(docking_status status)
{
	m_docking_status = status;
}

void station::docking_pub_status()
{
	scrub_robot_msgs::WorkstationDockingStatus docking_status_msg;
	
	docking_status_msg.docking_id = docking_id;
	docking_status_msg.device_id = device_id;
	docking_status_msg.workstation_id = workstation_id;
	docking_status_msg.docking_status = m_docking_status;

	docking_status_pub.publish(docking_status_msg);

}

void station::docking_set_and_pub_status(docking_status status)
{
	docking_set_status(status);
	docking_pub_status();
}

static void docking_start_thread(station *p_station)
{
	p_station->thelog.create("/home/bzl/catkin_ws/");
	//p_station->thelog.create("/opt/ros/catkin_ws/");
	p_station->thelog.printf("%s start\n", __func__);
	
	int try_stop_heartbeat = 5;

	
	ros::Rate loop_rate(1);
	int docking_pos;
	int nav_timeout;
	nav_timeout = 100;


	
	p_station->docking_set_and_pub_status(DOCKING_STATUS_CONNECTING);

	//TODO. functionize the socket
	p_station->station_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (p_station->station_sock < 0) {
		perror("socket");
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FAILED);
	
	}

	//TODO. retry mach at start, connect timeout
	if (0 > connect(p_station->station_sock, (struct sockaddr *)&p_station->sock_sta, p_station->len)) {
		perror("connect");
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FAILED);
		goto err2;
	}

	p_station->read_frame_start();
	
	//TODO. debug only
	p_station->send_frame(STA_RESET);//p_station->read_frame();

	
	if (p_station->fetch_frame(STA_CONN) < 0) {
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FORBIDDEN);
		goto err2;
	}
	p_station->docking_set_and_pub_status(DOCKING_STATUS_ALLOW);

	p_station->heartbeat_start();
	
	if (p_station->fetch_frame(STA_CONN_START) < 0) {
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FAILED);
		goto err3;
	}




	/*navigating......*/
	/*while (ros::ok()) {
		if (nav_timeout-- <= 0)
			break;
		
		//TODO. get status msg from nav and ...
		docking_pos = p_station->m_station_mcu_controller->getDin(10);
		if (docking_pos == 0)
			break;


		printf("%s running, docking_pos:%d nav_timeout:%d\n", __func__, docking_pos, nav_timeout);

		loop_rate.sleep();
	}
	if (nav_timeout <= 0) {
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FAILED);
		goto err3;
	}*/





	


	if (p_station->fetch_frame(STA_CONN_FIN) < 0) {
		p_station->docking_set_and_pub_status(DOCKING_STATUS_FAILED);
		goto err3;
	}
	p_station->docking_set_and_pub_status(DOCKING_STATUS_COMPLETED);

	p_station->docking_start_thread_set_status(DOCKING_THREAD_STOP);
	
	p_station->thelog.printf("%s end\n", __func__);
	return;

	err3:
		p_station->heartbeat_stop();
		while (try_stop_heartbeat--) {
			if (p_station->heartbeat_get_status() == HEARTBEAT_STOPED)
				break;
			sleep(1);
		}
	err2:
		if (p_station->station_sock > 0) {
			close(p_station->station_sock);
			p_station->station_sock = -1;
		}
	err:
		p_station->docking_start_thread_set_status(DOCKING_THREAD_STOP);
		return;

}


int station::docking_start()
{
	//docking srv״̬��0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�-1����ʧ��
	//docking msg״̬:0�����Խӣ�1�������ӹ���վ��2����վæ�������Խӣ�3�Խ���ɣ�?-1����ʧ��
	
	if (DOCKING_THREAD_STOP != docking_start_thread_get_status()) {
		return docking_get_status();
	}

	docking_start_thread_set_status(DOCKING_THREAD_RUNNING);
	std::thread pid_docking_start(docking_start_thread, this);
	pid_docking_start.detach();

	sleep(1);
	return docking_get_status();


}

static void docking_stop_thread(station *p_station)
{
	int try_stop_heartbeat = 5;
		ros::Rate loop_rate(1);

	p_station->thelog.printf("%s start\n", __func__);
	while (1) {
		if (DOCKING_THREAD_STOP == p_station->docking_start_thread_get_status())
			break;
		p_station->thelog.printf("docking_start_thread not finish yet\n");
		sleep(1);
	}

	//TODO. wait supply stop
	while (ros::ok()) {
		p_station->thelog.printf("%s stoping\n");
		if (
			p_station->supply_get_thread_status(MISSION_CHARGE) == SUPPLY_THREAD_STOP &&
			p_station->supply_get_thread_status(MISSION_CHARGE) == SUPPLY_THREAD_STOP &&
			p_station->supply_get_thread_status(MISSION_CHARGE) == SUPPLY_THREAD_STOP
		) break;
		loop_rate.sleep();
	}
	
	p_station->heartbeat_stop();
	while (try_stop_heartbeat--) {
		p_station->thelog.printf("docking stop thread 4\n");
		if (p_station->heartbeat_get_status() == HEARTBEAT_STOPED)
			break;
		sleep(1);
	}

	p_station->thelog.printf("docking stop thread heartbeat stop\n");
	if (p_station->fetch_frame(STA_DISCONN) < 0) {

	p_station->thelog.printf("%s fetch STA_DISCONN fail, do nothing\n", __func__);
		//TODO. retry
	
	}

	if (p_station->fetch_frame(STA_DETACH) < 0) {

	p_station->thelog.printf("%s fetch STA_DETACH fail, do nothing\n", __func__);
		//TODO. retry
	
	}
	p_station->thelog.printf("docking stop thread send STA_DETACH\n");

	
	p_station->read_frame_stop();

	if (p_station->station_sock > 0) {
		close(p_station->station_sock);
		p_station->station_sock = -1;
	}

	p_station->docking_stop_thread_set_status(DOCKING_THREAD_STOP);
	p_station->thelog.printf("%s end\n", __func__);
	printf("%s end\n", __func__);

	p_station->thelog.close();
}

int station::docking_stop()
{
	if (DOCKING_THREAD_STOP != docking_stop_thread_get_status()) {
		thelog.printf("docking stop under going\n");
		return -1;
	}

	docking_stop_thread_set_status(DOCKING_THREAD_RUNNING);
	std::thread pid_docking_stop(docking_stop_thread, this);
	pid_docking_stop.detach();

	return 0;
}

















void station::set_supply_id(std::string p_device_id, std::string p_workstation_id, std::string p_supply_id)
{
	device_id = p_device_id;
	workstation_id = p_workstation_id;
	supply_id = p_supply_id;
}

void station::supply_pub_status()
{
	scrub_robot_msgs::WorkstationSupplyStatus supply_status_msg;

	supply_status_msg.supply_id = supply_id;
	supply_status_msg.device_id = device_id;
	supply_status_msg.workstation_id = workstation_id;
	//supply_status_msg.link_status = supply_status[MISSION_LINK];
	supply_status_msg.add_water_status = supply_get_status(MISSION_WATER);
	supply_status_msg.discharge_water_status = supply_get_status(MISSION_DIRTY);
	supply_status_msg.charge_status = supply_get_status(MISSION_CHARGE);
	
	
	supply_status_pub.publish(supply_status_msg);

}

void station::supply_set_and_pub_status(int mission, supply_status status)
{
	switch (mission) {

	case MISSION_CHARGE:
		thelog.printf("%s mission:MISSION_CHARGE  status:%d\n", __func__, status);
		break;
	case MISSION_WATER:
		thelog.printf("%s mission:MISSION_WATER  status:%d\n", __func__, status);
		break;
	case MISSION_DIRTY:
		thelog.printf("%s mission:MISSION_DIRTY  status:%d\n", __func__, status);
		break;
	default:
		break;
	}


	supply_set_status(mission, status);
	supply_pub_status();
}

void station::supply_set_thread_status(int mission, supply_thread_status status)
{
	m_supply_thread_status[mission] = status;
}

int station::supply_get_thread_status(int mission)
{
	return m_supply_thread_status[mission];
}

void station::supply_set_status(int mission, supply_status status)
{
	m_supply_status[mission] = status;
}

int station::supply_get_status(int mission)
{
	return m_supply_status[mission];
}

void station::supply_mission_stop(int mission)
{
	supply_set_thread_status(MISSION_WATER, SUPPLY_THREAD_STOP);
}

void station::supply_mission_start(int mission)
{
	thelog.printf("supply %d start\n", mission);
	
	if (SUPPLY_THREAD_STOP != supply_get_thread_status(mission)) {
		thelog.printf("water already start\n");
		return;
	}

	supply_set_thread_status(mission, SUPPLY_THREAD_RUNNING);

	/*switch (mission) {
	case MISSION_WATER:
		std::thread pid_supply_water(supply_water_thread, this);
		pid_supply_water.detach();
		break;
	case MISSION_DIRTY:
		std::thread pid_supply_water(supply_dirty_thread, this);
		pid_supply_water.detach();
		break;
	case MISSION_CHARGE:
		std::thread pid_supply_water(supply_charge_thread, this);
		pid_supply_water.detach();
		break;
	default:
		break;
	}*/


}



#define SUPPLY_TIMEOUT 100
#define SUPPLY_WATER_TIME 100
#define SUPPLY_DIRTY_TIME 100
#define SUPPLY_CHARGE_TIME 100

#define STA_BASE_CMD STA_CHARGE
#define STA_BASE_CMD_R STA_CHARGE_R
#define STA_BASE_CMD_START STA_CHARGE_START
#define STA_BASE_CMD_START_R STA_CHARGE_START_R
#define STA_BASE_CMD_STOP STA_CHARGE_STOP
#define STA_BASE_CMD_STOP_R STA_CHARGE_STOP_R
#define STA_BASE_CMD_SUBMIT STA_CHARGE_SUBMIT
#define STA_BASE_CMD_SUBMIT_R STA_CHARGE_SUBMIT_R

/*
static void supply_mission_thread(int mission, int timeout, station *p_station)
{
	printf("%s start %d \n", __func__, mission);
	
	ros::Rate loop_rate(1);
	
	if (p_station->fetch_frame(STA_BASE_CMD+mission*8) < 0) {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		//p_station.supply_set_and_pub_status(MISSION_WATER, SUPPLY_WATER_RUN);
	}
	
	
	if (p_station->fetch_frame(STA_BASE_CMD_START+mission*8) < 0) {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_RUN);
	}


	while (ros::ok()) {
		if (timeout-- <= 0)
			break;
		
		if (p_station->supply_get_thread_status(mission) == SUPPLY_THREAD_STOP)
			break;

		printf("mission %d\n running\n", mission);
		loop_rate.sleep();
	}

	
	if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {


		sleep(1);
		//TODO. retry
	
	}

	if (timeout <= 0) {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);
	} else {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_STOP);
	}
	p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);

}
*/

static void supply_water_thread(int mission, int timeout, station *p_station)
{
	p_station->thelog.printf("%s start\n", __func__);
	
	ros::Rate loop_rate(1);
	int water_pos;

	
	water_pos = p_station->m_station_mcu_controller->getDin(5);
	sleep(2);
	water_pos = p_station->m_station_mcu_controller->getDin(5);
	if (water_pos == 0) {
		p_station->thelog.printf("%s water already full, exit\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		return;

	}
	
	if (p_station->fetch_frame(STA_BASE_CMD+mission*8) < 0) {
		p_station->thelog.printf("%s fetch frame STA_BASE_CMD fail, return\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		//p_station.supply_set_and_pub_status(MISSION_WATER, SUPPLY_WATER_RUN);
	}

	sleep(15);//wait water pipe get in
	
	if (p_station->fetch_frame(STA_BASE_CMD_START+mission*8) < 0) {
		p_station->thelog.printf("%s wait water pipe in fail\n");
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_RUN);
	}

	while (ros::ok()) {
		p_station->thelog.printf("%s running, water_pos:%d timeout:%d\n", __func__, water_pos, timeout);

		if (timeout-- <= 0)
			break;
		
		water_pos = p_station->m_station_mcu_controller->getDin(5);
		if (water_pos == 0) {
			p_station->thelog.printf("%s water is full, break\n", __func__);
			break;

		}
		
		if (p_station->supply_get_thread_status(mission) == SUPPLY_THREAD_STOP) {
			p_station->thelog.printf("%s SUPPLY_THREAD_STOP, return\n", __func__);

			if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
				p_station->thelog.printf("%s fetch STA_BASE_CMD_STOP fail, do nothing\n", __func__);

				sleep(1);
				//TODO. retry
			
			}
				p_station->thelog.printf("%s wait 30s pipe get in\n", __func__);
				sleep(30);//wait water pipe get in

				p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
			return;
			
		}

		loop_rate.sleep();
	}

	
	if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
			p_station->thelog.printf("%s fetch STA_BASE_CMD_STOP fail, do nothing\n", __func__);

		sleep(1);
		//TODO. retry
	
	}
			p_station->thelog.printf("%s wait 30s pipe get in\n", __func__);
	sleep(30);//wait water pipe get in

		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);

	p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);

	
	p_station->thelog.printf("%s end\n", __func__);

}

static void supply_dirty_thread(int mission, int timeout, station *p_station)
{
	p_station->thelog.printf("%s start\n", __func__);
	
	ros::Rate loop_rate(1);
	int dirty_pos;

	dirty_pos = p_station->m_station_mcu_controller->getDin(8);
	sleep(2);
	dirty_pos = p_station->m_station_mcu_controller->getDin(8);
	if (dirty_pos == 0) {
		p_station->thelog.printf("%s dirty already empty, exit\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		return;

	}
	
	if (p_station->fetch_frame(STA_BASE_CMD+mission*8) < 0) {
		p_station->thelog.printf("%s fetch STA_BASE_CMD fail, return\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		//p_station.supply_set_and_pub_status(MISSION_WATER, SUPPLY_WATER_RUN);
	}
	
	
	if (p_station->fetch_frame(STA_BASE_CMD_START+mission*8) < 0) {
		p_station->thelog.printf("%s fetch STA_BASE_CMD_START fail, return\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_RUN);
	}

	p_station->m_station_mcu_controller->setDout(13,1);//valve
	p_station->m_station_mcu_controller->setPwm(4, 100, 5);//boom
	
	while (ros::ok()) {
		p_station->thelog.printf("%s running, dirty_pos:%d timeout:%d\n", __func__, dirty_pos, timeout);

		if (timeout-- <= 0)
			break;
		
		dirty_pos = p_station->m_station_mcu_controller->getDin(8);
		if (dirty_pos == 0) {
			p_station->thelog.printf("%s dirty is empty, break\n", __func__);
			break;

		}
		
		if (p_station->supply_get_thread_status(mission) == SUPPLY_THREAD_STOP) {
			p_station->thelog.printf("%s SUPPLY_THREAD_STOP, return\n", __func__);
			if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
				p_station->thelog.printf("%s fetch STA_BASE_CMD_STOP fail, do nothing\n", __func__);
			}
			p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
			return;
		}
		

		loop_rate.sleep();
	}

	
	if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
		p_station->thelog.printf("%s fetch STA_BASE_CMD_STOP fail, do nothing\n", __func__);

		sleep(1);
		//TODO. retry
	
	}

		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);

	p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);

	p_station->m_station_mcu_controller->setPwm(4, 0, 5);//boom
	p_station->m_station_mcu_controller->setDout(13,0);//valve
	
	p_station->thelog.printf("%s end\n", __func__);

}

static void supply_charge_thread(int mission, int timeout, station *p_station)
{
	p_station->thelog.printf("%s start\n", __func__);
	
	ros::Rate loop_rate(1);
	int charge_status;
	int charge_pos_status;
	
	if (p_station->fetch_frame(STA_BASE_CMD+mission*8) < 0) {
		p_station->thelog.printf("%s fetch STA_BASE_CMD fail, return\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		//p_station.supply_set_and_pub_status(MISSION_WATER, SUPPLY_WATER_RUN);
	}
	
	charge_pos_status = p_station->m_station_mcu_controller->getDin(9);
	charge_status = p_station->m_station_mcu_controller->getDin(3);
	sleep(2);
	int charge_pos_timeout = 30;
	while (ros::ok()) {
		charge_pos_status = p_station->m_station_mcu_controller->getDin(9);
		p_station->thelog.printf("%s position, din9:%d, timeout:%d\n", __func__, charge_pos_status, charge_pos_timeout);
		if (charge_pos_status == 1) {
			break;
		}

		loop_rate.sleep();

		//charge_pos_timeout
		if (charge_pos_timeout-- <= 0) {
			p_station->thelog.printf("%s position timeout, return\n", __func__);

			
			if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
				p_station->thelog.printf("%s fetch STA_BASE_CMD_STOP fail, do nothing\n", __func__);
			
				sleep(1);
				//TODO. retry
			
			}
			
			p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
			p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
			
			return;
		}

		//


	}
	



	p_station->m_station_mcu_controller->setDout(1,1);//first charge touch, then request
	if (p_station->fetch_frame(STA_BASE_CMD_START+mission*8) < 0) {
			p_station->thelog.printf("%s fetch frame STA_BASE_CMD_START fail, return\n", __func__);
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
		p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
		//p_station.supply_set_status(MISSION_WATER, SUPPLY_WATER_STOP);//set stop status at stop service
		return;
	} else {
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_RUN);
	}


		charge_status = p_station->m_station_mcu_controller->getDin(3);
		sleep(2);

	while (ros::ok()) {
		p_station->thelog.printf("%s running, charge_status:%d timeout:%d\n", __func__, charge_status, timeout);
		if (timeout-- <= 0)
			break;
		
		charge_status = p_station->m_station_mcu_controller->getDin(3);
		if (charge_status == 1) {
			p_station->thelog.printf("%s charge status abnormal, return\n", __func__);
			
			/*if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {
			
			
				sleep(1);
				//TODO. retry
			
			}
			
			p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
			p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
			
			return;*/

		}
		
		
		if (p_station->supply_get_thread_status(mission) == SUPPLY_THREAD_STOP)
		{
			p_station->thelog.printf("%s SUPPLY_THREAD_STOP, return\n", __func__);
			p_station->m_station_mcu_controller->setDout(1,0);//charge touch

			
			if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {

				p_station->thelog.printf("%s fetch  STA_BASE_CMD_STOP fail, do nothing\n", __func__);
			}

			p_station->thelog.printf("%s wait 10s charger get in\n", __func__);
			sleep(10);//wait charge pipe get in
			
			p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FAIL);
			return;
		}


		loop_rate.sleep();
	}

	p_station->m_station_mcu_controller->setDout(1,0);//charge touch

	
	if (p_station->fetch_frame(STA_BASE_CMD_STOP+mission*8) < 0) {

			p_station->thelog.printf("%s fetch  STA_BASE_CMD_STOP fail, do nothing\n", __func__);
		sleep(1);
		//TODO. retry
	
	}
			p_station->thelog.printf("%s wait 10s charger get in\n", __func__);
	sleep(10);//wait charge pipe get in
	
		p_station->supply_set_and_pub_status(mission, SUPPLY_STATUS_FIN);
	p_station->supply_set_thread_status(mission, SUPPLY_THREAD_STOP);
	

	p_station->thelog.printf("%s end\n", __func__);

}



int station::supply_start(int water, int dirty, int charge)
{
	/*req.supply_id
	req.device_id
	req.workstation_id
	req.add_water
	req.discharge_water
	req.charge*/
	//0δ��ˮ��1���ڼ�ˮ��2��ˮ��ɣ�?-1��ˮʧ��

	thelog.printf("supply_start:water:%d dirty:%d charge:%d\n", water, dirty, charge);
	if (DOCKING_STATUS_COMPLETED != docking_get_status()) {printf("supply_start:docking not complete!!!!!!!!!!\n");
		return -1;
	}

	if (water == 1 && SUPPLY_THREAD_STOP == supply_get_thread_status(MISSION_WATER)) {
		supply_set_thread_status(MISSION_WATER, SUPPLY_THREAD_RUNNING);
		std::thread pid_water_thread(supply_water_thread, MISSION_WATER, 50, this);
		pid_water_thread.detach();
	}

	if (dirty == 1 && SUPPLY_THREAD_STOP == supply_get_thread_status(MISSION_DIRTY)) {
		supply_set_thread_status(MISSION_DIRTY, SUPPLY_THREAD_RUNNING);
		std::thread pid_dirty_thread(supply_dirty_thread, MISSION_DIRTY, 30, this);
		pid_dirty_thread.detach();
	}

	if (charge == 1 && SUPPLY_THREAD_STOP == supply_get_thread_status(MISSION_CHARGE)) {
		supply_set_thread_status(MISSION_CHARGE, SUPPLY_THREAD_RUNNING);
		std::thread pid_charge_thread(supply_charge_thread, MISSION_CHARGE, 50, this);
		pid_charge_thread.detach();
	}
	

	return 0;

}

int station::supply_stop()
{
	supply_set_thread_status(MISSION_WATER, SUPPLY_THREAD_STOP);
	supply_set_status(MISSION_WATER, SUPPLY_STATUS_STOP);

	supply_set_thread_status(MISSION_DIRTY, SUPPLY_THREAD_STOP);
	supply_set_status(MISSION_DIRTY, SUPPLY_STATUS_STOP);

	supply_set_thread_status(MISSION_CHARGE, SUPPLY_THREAD_STOP);
	supply_set_status(MISSION_CHARGE, SUPPLY_STATUS_STOP);

	return 0;
}

