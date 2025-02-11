#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <protocol/packets/BoardPacket.h>
#include <protocol/packets/ServoMotorPacket.h>
#include <protocol/packets/DCMotorPacket.h>
#include <protocol/packets/DistanceSensorPacket.h>
#include <protocol/handlers/DCMotorBoardPacketHandler.h> // class's header file
#include <protocol/handlers/BatteryBoardPacketHandler.h>
#include <protocol/handlers/DistanceSensorBoardPacketHandler.h>
#include <protocol/handlers/ServoBoardPacketHandler.h>
#include <protocol/handlers/TrashBinBoardPacketHandler.h>
#include <protocol/Packet.h>
#include <protocol/PacketServer.h>
#include "serverUI.h"

#define MAX(a,b) (a>b?a:b)

//define group and board ids
#define DCMOTOR_GID 1
#define DISTANCESENSOR_GID 3

#define DCMOTOR_BOARD_ID0 0
#define DCMOTOR_BOARD_ID1 1
#define DISTANCESENSOR_BOARD_ID0 0
#define DISTANCESENSOR_BOARD_ID1 1
#define DISTANCESENSOR_BOARD_ID2 2

#define PIPE_IN 0
#define PIPE_OUT 1

typedef struct {
    const char * group;
    const char * cmd;
    void (*f)(char * data);
    const char * cmd_help;
    const char * cmd_help_param;
} cmd_type;

bool quit = false, groupBC = false, fullBC = false, debug = false, alarmSignal = false;
int fd = 5;
int pipes[2];
int lastCMD = -1;

int dest_group = 0x01, dest_card = 0x01;

// Packet Server
protocol::PacketServer * ps;

// Packet boards
protocol::packets::BoardPacket * boardPacket = NULL;
protocol::packets::DCMotorPacket * dcPacket = NULL;
protocol::packets::DistanceSensorPacket * dsPacket = NULL;

protocol::packets::DCMotorPacket * dcPacket0 = NULL;
protocol::packets::DCMotorPacket * dcPacket1 = NULL;

protocol::packets::DistanceSensorPacket * dsPacket0 = NULL;
protocol::packets::DistanceSensorPacket * dsPacket1 = NULL;
protocol::packets::DistanceSensorPacket * dsPacket2 = NULL;

// Handlers for the boards
protocol::handlers::DCMotorBoardPacketHandler * dcHandler = NULL;
protocol::handlers::DistanceSensorBoardPacketHandler * dsHandler = NULL;

protocol::handlers::DCMotorBoardPacketHandler * dcMotorHandler0 = NULL;
protocol::handlers::DCMotorBoardPacketHandler * dcMotorHandler1 = NULL;
protocol::handlers::DistanceSensorBoardPacketHandler * distanceSensorHandler0 = NULL;
protocol::handlers::DistanceSensorBoardPacketHandler * distanceSensorHandler1 = NULL;
protocol::handlers::DistanceSensorBoardPacketHandler * distanceSensorHandler2 = NULL;

// Common commands
void cmd_help(char * data);
void cmd_quit(char * data);
void cmd_getDest(char * data);
void cmd_setDest(char * data);
void cmd_groupBC(char * data);
void cmd_fullBC(char * data);

bool init();
char getDest();
void sendAPacket(protocol::Packet * p);

// Implementation

// Command list
cmd_type commands[] = {
    // Common commands
    {"common", "dc", cmd_setDC, "Set actual DC Motor board", "\%d for the board ID (0 or 1)"},
    {"common", "ds", cmd_setDS, "Set actual Distance Sensor board", "\%d for the board ID (0, 1 or 2)"},
    {"common", "setDest", cmd_setDest, "Set group and card id for destination", "\%d \%d for group and card (0 to 15)"},
    {"common", "getDest", cmd_getDest, "Get group and card id for destination", ""},
    {"common", "groupBC", cmd_groupBC, "Change group broadcast state", ""},
    {"common", "fullBC", cmd_fullBC, "Change full broadcast state", ""},
    {"common", "debug", cmd_debug, "Set if debug messages must be shown", "\%d value, 0 means off and 1 means on"},
    {"common", "telemetry", cmd_telemetry, "Set if telemetry messages must be shown every second", "\%d value, 0 means off and 1 means on"},
    // Commands for MainController (mc)
    {"mc", "mcInit", cmd_init, "Send init command", ""},
    {"mc", "mcReset", cmd_reset, "Send reset command", ""},
    {"mc", "mcPing", cmd_ping, "Send ping command", ""},
    {"mc", "mcError", cmd_error, "Send error command", "\%d for error"},
    // Commands for MotorDC (dc)                                                                      
    {"dc", "dcSetDirection", cmd_dcSetDirection, "Set motor turn", "\%d for Clockwise:0 or counter-clockwise:1"},
    {"dc", "dcSetSpeed", cmd_dcSetSpeed, "Set motor speed", "\%d \%d for Clockwise:0 or counter-clockwise:1 and counts per second"},
    {"dc", "dcSetEncoder", cmd_dcSetEncoder, "Set historical counts of encoder", "\%d number of counts"},
    {"dc", "dcGetEncoder", cmd_dcGetEncoder, "Get historical counts of encoder", ""},
    {"dc", "dcResetEncoder", cmd_dcResetEncoder, "Reset historical counts of encoder", ""},
    {"dc", "dcSetEncoderToStop", cmd_dcSetEncoderToStop, "Set remaining counts of encoder to stop", "\%hd number of counts"},
    {"dc", "dcGetEncoderToStop", cmd_dcGetEncoderToStop, "Obtain remaining counts of encoder to stop", ""},
    {"dc", "dcDontStop", cmd_dcDontStop, "Undo commands for stopping encoder, disables encoder counting", ""},
    {"dc", "dcConsumption", cmd_dcConsumption, "Gets current motor consumption", ""},
    {"dc", "dcGetSpeed", cmd_dcGetSpeed, "Get motor speed in counts per second", ""},
    // Commands for DistanceSensor (ds)
    {"ds", "dsOn", cmd_dsOn, "Turn on a sensor", "\%d for the sensor ID"},
    {"ds", "dsOff", cmd_dsOff, "Turn off a sensor", "\%d for the sensor ID"},
    {"ds", "dsSetMask", cmd_dsSetMask, "Set the sensors mask for the next reads", "\%hd representing each bit a sensor ID (LSB)"},
    {"ds", "dsGetMask", cmd_dsGetMask, "Get the actual sensors mask", ""},
    {"ds", "dsGetValue", cmd_dsGetValue, "Get an average value for each sensor", "\%hd representing each bit a sensor ID (LSB)"},
    {"ds", "dsGetOneValue", cmd_dsGetOneValue, "Get just one value for each sensor", "\%hd representing each bit a sensor ID (LSB)"},
    {"ds", "dsAlarmOn", cmd_dsAlarmOn, "Set the alarm to rising or falling edge", "\%d for the  status where 0 means off, 1 for rising edge, 2 for falling edge and 3 for any change"},
    {"ds", "dsAlarmCommand", cmd_dsAlarmCommand, "Alarm command on same state", "\%d for the alarm status to be sent. 0 for off, 1 for rising edge, 2 for falling edge and 3 for any change"},
    // Commands for ServoMotor (sm)
    {"sm", "smSetPos", cmd_smSetPos, "Set servo position", ""},
    {"sm", "smSetAllPos", cmd_smSetAllPos, "NOT YET IMPLEMENTED", ""},
    {"sm", "smGetPos", cmd_smGetPos, "NOT YET IMPLEMENTED", ""},   
    {"sm", "smGetAllPos", cmd_smGetAllPos, "NOT YET IMPLEMENTED", ""},
    {"sm", "smSetSpeed", cmd_smSetSpeed, "NOT YET IMPLEMENTED", ""}, 
    {"sm", "smSetAllSpeed", cmd_smSetAllSpeed, "NOT YET IMPLEMENTED", ""},
    {"sm", "smGetSpeed", cmd_smGetSpeed, "NOT YET IMPLEMENTED", ""},   
    {"sm", "smGetAllSpeed", cmd_smGetAllSpeed, "NOT YET IMPLEMENTED", ""},
    {"sm", "smFree", cmd_smFree, "NOT YET IMPLEMENTED", ""},       
    {"sm", "smFreeAll", cmd_smFreeAll, "NOT YET IMPLEMENTED", ""},    
    // Commands for BatteryController (bc)
    {"bc", "bcEnable", cmd_bcEnable, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcDisable", cmd_bcDisable, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcGetValue", cmd_bcGetValue, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcFullAlarm", cmd_bcFullAlarm, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcSetEmptyValue", cmd_bcSetEmptyValue, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcEmptyAlarm", cmd_bcEmptyAlarm, "NOT YET IMPLEMENTED", ""},
    {"bc", "bcSetFullValue", cmd_bcSetFullValue, "NOT YET IMPLEMENTED", ""},
    // Commands for TrashBin (tb)
    {"tb", "tbGetValue", cmd_tbGetValue, "NOT YET IMPLEMENTED", ""},
    {"tb", "tbFullAlarm", cmd_tbFullAlarm, "NOT YET IMPLEMENTED", ""},
    {"tb", "tbSetFullValue", cmd_tbSetFullValue, "NOT YET IMPLEMENTED", ""},
    
    {"common", "help", cmd_help, "This help", "\%s one of: all, common, mc, dc, sm, ds, bc, tb"},
    {"common", "quit", cmd_quit, "Quit to system", ""},
    {"common", NULL, cmd_quit, "", ""}
};

char getDest()
{
    int group = dest_group & 0x0F;
    int card = dest_card & 0x0F;
    
    if (fullBC == true) {
        group = 0x0F;
        card = 0x0F;
    } else if (groupBC == true) {
        card = 0x0F;
    } 
    
    return group * 16 + card;
}

void command(char * cmd, int lenght)
{
    int i = 0;
    bool found = false;
    char * data = NULL;
    
    // Get parameters
    if ((data = strstr(cmd, " ")) != NULL)
    {
        data[0] = '\0';
        data++;
    }
    
    if (strlen(cmd) < 1)
        return;

    // Resend last command?
    if (strlen(cmd) >= 3 && cmd[0] == '\033' && cmd[1] == '[' && cmd[2] == 'A' && lastCMD != -1)
        cmd = (char *)(commands[lastCMD].cmd);
    
    while (found == false && commands[i].cmd != NULL)
    {
        if (strcmp(commands[i].cmd, cmd) == 0)
        {
            // Command found!
            commands[i].f(data);
            found = true;
            lastCMD = i;
        }
        i++;
    }
    
    if (found == false)
        printf("Command not found\n");
    
    return;
}

// Ask for telemetry...
void getTelemetry() {
	char * str = NULL;
	// Backup...
	protocol::handlers::DCMotorBoardPacketHandler * dcHBackup = dcHandler;
	protocol::handlers::DistanceSensorBoardPacketHandler * dsHBackup = dsHandler;
	protocol::packets::DCMotorPacket * dcPBackup = dcPacket;
	protocol::packets::DistanceSensorPacket * dsPBackup = dsPacket;

	// Motor0
    dcHandler = dcMotorHandler0;
	dcPacket = dcPacket0;
	cmd_dcGetSpeed(str);
	cmd_dcGetEncoder(str);
	cmd_dcGetEncoderToStop(str);
	// Motor1
    dcHandler = dcMotorHandler1;
	dcPacket = dcPacket1;
	cmd_dcGetSpeed(str);
	cmd_dcGetEncoder(str);
	cmd_dcGetEncoderToStop(str);
	
	// Sensor0
	dsHandler = distanceSensorHandler0;
	dsPacket = dsPacket0;
	cmd_dsGetValue((char*)"3F");
	// Sensor1
	dsHandler = distanceSensorHandler1;
	dsPacket = dsPacket0;
	cmd_dsGetValue((char*)"3F");
	// Sensor2
	dsHandler = distanceSensorHandler2;
	dsPacket = dsPacket0;
	cmd_dsGetValue((char*)"3F");

	// Restore Backup
	dsPacket = dsPBackup;
	dcPacket = dcPBackup;
	dsHandler = dsHBackup;
	dcHandler = dcHBackup;
	
	return;
}

void alarmHandler(int s) {
	alarm(1);
	alarmSignal = true;
	return;
}

int main( int argc, const char **argv)
{
    int maxfd = 1, select_resp = 0;
    char stdin_buffer[256] = {0}, serial_buffer[256] = {0};
    int stdin_idx = 0;
    int c = 0;
    struct timeval timeout;
    fd_set readfd, writefd;
    fd_set readfd_b, writefd_b;
    
	ps = new protocol::PacketServer();
	registerHandlers(ps);
	ps->start();

    createPacketBoards();
        
    // Set file descriptors
    FD_SET(0,&readfd);

    readfd_b = readfd;
    writefd_b = writefd;
    
    cmd_help((char *)"common");

	// Set the alarm SIGNAL Handler
	signal(SIGALRM, &alarmHandler);

    while ( quit != true )
    {
        if ((select_resp = select(maxfd, &readfd, &writefd, NULL, NULL)) == 0)
        {
            // Select timed out!
            // TODO: retransmitir la lista de mensajes sin responder...
            printf("TIME OUT!\n");
        }

		if (alarmSignal == true) {
			getTelemetry();
			alarmSignal = false;
		}

        if (errno == EINTR)
        {
            // A signal was delivered befor time_out 
            errno = 0;
            continue;
        }

        // STDIN
        if ( FD_ISSET(0, &readfd) )
        {
            // Have things in buffer! :P
            c = read(0, stdin_buffer + stdin_idx, 1);
            
            if (stdin_buffer[stdin_idx] == '\n')
            {
                stdin_buffer[stdin_idx] = '\0';
                command(stdin_buffer, stdin_idx);
                stdin_idx = 0;
            } else {
                stdin_idx++;
            }
        }

        // Restore structures from backup
        readfd = readfd_b;
        writefd = writefd_b;
    }
    
    return 0;
}

void createPacketBoards() {
	
    dcPacket0 = new protocol::packets::DCMotorPacket(DCMOTOR_GID, DCMOTOR_BOARD_ID0);
    dcPacket1 = new protocol::packets::DCMotorPacket(DCMOTOR_GID, DCMOTOR_BOARD_ID1);

    dsPacket0 = new protocol::packets::DistanceSensorPacket(DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID0);
    dsPacket1 = new protocol::packets::DistanceSensorPacket(DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID1);
    dsPacket2 = new protocol::packets::DistanceSensorPacket(DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID2);
	
	dcPacket = dcPacket0;
	dsPacket = dsPacket0;
	
    boardPacket = dcPacket0;

	return;
}

void
registerHandlers(protocol::PacketServer * ps){

	// Create handlers
    dcMotorHandler0 = new protocol::handlers::DCMotorBoardPacketHandler(ps,DCMOTOR_GID,DCMOTOR_BOARD_ID0);
    dcMotorHandler1 = new protocol::handlers::DCMotorBoardPacketHandler(ps,DCMOTOR_GID,DCMOTOR_BOARD_ID1);
	distanceSensorHandler0 = new protocol::handlers::DistanceSensorBoardPacketHandler(ps,DISTANCESENSOR_GID,DISTANCESENSOR_BOARD_ID0);
	distanceSensorHandler1 = new protocol::handlers::DistanceSensorBoardPacketHandler(ps,DISTANCESENSOR_GID,DISTANCESENSOR_BOARD_ID1);
	distanceSensorHandler2 = new protocol::handlers::DistanceSensorBoardPacketHandler(ps,DISTANCESENSOR_GID,DISTANCESENSOR_BOARD_ID2);

    dcHandler = dcMotorHandler0;
	dsHandler = distanceSensorHandler0;
	    
	// Register handlers
    ps->registerHandler(dcMotorHandler0, DCMOTOR_GID, DCMOTOR_BOARD_ID0);
    ps->registerHandler(dcMotorHandler1, DCMOTOR_GID, DCMOTOR_BOARD_ID1);
    ps->registerHandler(distanceSensorHandler0, DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID0);
    ps->registerHandler(distanceSensorHandler1, DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID1);
    ps->registerHandler(distanceSensorHandler2, DISTANCESENSOR_GID, DISTANCESENSOR_BOARD_ID2);

}

void sendAPacket(protocol::Packet * p){
    unsigned char i;
    p->calculateCRC();
    char * packet = p->getPacket();
    if (debug == true)
		p->print();
    //printf("escribi: %d bytes en el pipe\n",write(pipes[PIPE_OUT],packet,p->getActualLength()));
    //this->waitingForResponse.push_back(p);
}

// Commands for MainController (mc)
void cmd_help(char * data)
{
    int i = 0;
    
    printf("\nPossible commands:\n\n");

    if (data == NULL)
        data = (char *)"common";
    
    while (commands[i].cmd != NULL)
    {
        if (strcmp(data, "all") == 0 || strcmp(commands[i].group, data) == 0)
        {
            printf("%-25s%s\n", commands[i].cmd, commands[i].cmd_help);
            if (commands[i].cmd_help_param[0] != '\0')
                printf("%25sParams: %s\n", "", commands[i].cmd_help_param);
        }
        i++;
    }
    
    return;
}

void cmd_quit(char * data)
{
    quit = true;
    return;
}

// COMMON_INIT 0X01
void cmd_init(char * data)
{
    boardPacket->clear();
    boardPacket->setInit();
    boardPacket->prepareToSend();
    sendAPacket(boardPacket);
    return;
}

// COMMON_RESET 0X02
void cmd_reset(char * data)
{
    boardPacket->clear();
    boardPacket->setReset();
    boardPacket->prepareToSend();
    sendAPacket(boardPacket);
    return;
}

// COMMON_PING 0X03
void cmd_ping(char * data)
{
    boardPacket->clear();
    boardPacket->setPing();
    boardPacket->prepareToSend();
    sendAPacket(boardPacket);
    return;
}

// COMMON_ERROR	0X04
void cmd_error(char * data)
{
    boardPacket->clear();
    boardPacket->setPing();
    boardPacket->prepareToSend();
    sendAPacket(boardPacket);
    // TODO: armar paquete de error y mandarlo
    return;
}

void cmd_setDC(char * data)
{
	int id = 0;
	
    if (data == NULL || sscanf(data, "%d", &id) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	dest_group = DCMOTOR_GID;
    
    switch (id)
    {
		case 0:
			dcHandler = dcMotorHandler0;
			dcPacket = dcPacket0;
			dest_card = DCMOTOR_BOARD_ID0;
			break;
		case 1:
			dcHandler = dcMotorHandler1;
			dcPacket = dcPacket1;
			dest_card = DCMOTOR_BOARD_ID1;
			break;
		default:
			// DEFAULT
			printf("Bad ID, setting first board\n");
			dcHandler = dcMotorHandler0;
			dcPacket = dcPacket0;
			dest_card = DCMOTOR_BOARD_ID0;
			break;
	}
	
	boardPacket = dcPacket;
    printf("DC Motor Board -> GroupID: %d CardID: %d -> (%X)\n", dest_group, dest_card, getDest());
    
    return;
}

void cmd_setDS(char * data)
{
	int id = 0;
	
    if (data == NULL || sscanf(data, "%d", &id) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	dest_group = DISTANCESENSOR_GID;
    
    switch (id)
    {
		case 0:
			dsHandler = distanceSensorHandler0;
			dsPacket = dsPacket0;
			dest_card = DISTANCESENSOR_BOARD_ID0;
			break;
		case 1:
			dsHandler = distanceSensorHandler1;
			dsPacket = dsPacket1;
			dest_card = DISTANCESENSOR_BOARD_ID1;
			break;
		case 2:
			dsHandler = distanceSensorHandler2;
			dsPacket = dsPacket2;
			dest_card = DISTANCESENSOR_BOARD_ID2;
			break;
		default:
			// DEFAULT
			printf("Bad ID, setting first board\n");
			dsHandler = distanceSensorHandler0;
			dsPacket = dsPacket0;
			dest_card = DISTANCESENSOR_BOARD_ID0;
			break;
	}
	
	boardPacket = dsPacket;
	
    printf("Distanse Sensor Board -> GroupID: %d CardID: %d -> (%X)\n", dest_group, dest_card, getDest());
    
    return;
}

void cmd_setDest(char * data)
{
    if (data == NULL || sscanf(data, "%d %d", &dest_group, &dest_card) != 2)
    {
        printf("Wrong parameters\n");
        return;
    }
    
    boardPacket = new protocol::packets::BoardPacket(dest_group,dest_card);
    
    printf("Board -> GroupID: %d CardID: %d -> (%X)\n", dest_group, dest_card, getDest());
    
    return;
}

void cmd_getDest(char * data)
{
    printf("GroupID: %d CardID: %d -> (%X)\n", dest_group, dest_card, getDest());
    return;
}

void cmd_groupBC(char * data)
{
    groupBC = !groupBC;
    printf("Group broadcast state: %s\n", (groupBC?"Activated":"Deactivated"));
    return;
}

void cmd_fullBC(char * data)
{
    fullBC = !fullBC;
    printf("Full broadcast state: %s\n", (fullBC?"Activated":"Deactivated"));
    return;
}

void cmd_telemetry(char * data)
{
	int v = 0;
	
    if (data == NULL || sscanf(data, "%d", &v) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
    
    if (v == 1) {
		alarm(1);
	} else {
		alarm(0);
	}

    printf("Telemetry mode %s\n", (v == 1?"ON":"OFF"));
    return;
}

void cmd_debug(char * data)
{
	int v = 0;
	
    if (data == NULL || sscanf(data, "%d", &v) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
    
    if (v == 1) {
		debug = true;
	} else {
		debug = false;
	}

	ps->setDebug(debug);
    printf("Debug mode %s\n", (debug == true?"ON":"OFF"));
    return;
}

// Commands for MotorDC (dc)

// DC_MOTOR_SET_DIRECTION      0X40
void cmd_dcSetDirection(char * data)
{
    int turn;
    
    if (data == NULL || sscanf(data, "%d", &turn) != 1)
    {
        printf("Wrong parameters\n");
			return;
    }

	// TODO:
    dcPacket->clear();

    if ( turn <= 0 )
        dcPacket->setDirection(true);
    else
        dcPacket->setDirection(false);
        
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_SET_DC_SPEED       0X41
void cmd_dcSetSpeed(char * data)
{
    int turn, speed;
    
    if (data == NULL || sscanf(data, "%d %d", &turn, &speed) != 2)
    {
        printf("Wrong parameters\n");
        return;
    }

    // Clockwise?
    if (turn == 0)
        turn = 1;
    else
        turn = -1;

    // set speed
	dcHandler->setSpeed(speed * turn);
    return;
}

// DC_MOTOR_SET_ENCODER        0X42
void cmd_dcSetEncoder(char * data)
{
    int turn;
    
    if (data == NULL || sscanf(data, "%d", &turn) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	// TODO:
    dcPacket->clear();
	
	dcPacket->setEncoder(turn);
    dcPacket->prepareToSend();
	
	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_GET_ENCODER        0X43
void cmd_dcGetEncoder(char * data)
{
    dcPacket->clear();

	dcPacket->getEncoder();
    dcPacket->prepareToSend();

    ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_RESET_ENCODER      0X44
void cmd_dcResetEncoder(char * data)
{
    dcPacket->clear();

	dcPacket->resetEncoder();
	dcPacket->prepareToSend();
	
	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_SET_ENCODER_TO_STOP    0X45
void cmd_dcSetEncoderToStop(char * data)
{
	short counts;
    
    if (data == NULL || sscanf(data, "%hd", &counts) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

    dcPacket->clear();

    dcPacket->setEncoderToStop(counts);
    dcPacket->prepareToSend();
    
	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_GET_ENCODER_TO_STOP    0X46
void cmd_dcGetEncoderToStop(char * data)
{
    dcPacket->clear();

	dcPacket->getEncoderToStop();
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_DONT_STOP      0X47
void cmd_dcDontStop(char * data)
{
    dcPacket->clear();

	dcPacket->setNonStop();
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_MOTOR_CONSUMPTION  0X48
void cmd_dcConsumption(char * data)
{
    dcPacket->clear();

	dcPacket->getMotorConsumption();
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_MOTOR_STRESS_ALARM 0X49
void cmd_dcStressAlarm(char * data)
{
	short consumption;
    
    if (data == NULL || sscanf(data, "%hd", &consumption) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
    
	dcPacket->clear();

	dcPacket->isMotorAlarm();
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_MOTOR_SHUT_DOWN_ALARM  0X4A
void cmd_dcShutDownAlarm(char * data)
{
	short consumption;
	
	if (data == NULL || sscanf(data, "%hd", &consumption) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
	
	dcPacket->clear();

	dcPacket->isMotorShutDown();
    dcPacket->prepareToSend();

	ps->sendPacket(dcPacket);

    return;
}

// DC_MOTOR_GET_DC_SPEED       0X4B
void cmd_dcGetSpeed(char * data)
{
	dcHandler->getSpeed();

    return;
}

// Commands for ServoMotor (sm)

// SERVO_MOTOR_SET_POSITION    0X40
void cmd_smSetPos(char * data)
{
	//TODO 
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_SET_ALL_POSITIONS   0X41
void cmd_smSetAllPos(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_GET_POSITION    0X42
void cmd_smGetPos(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_GET_ALL_POSITIONS   0X43
void cmd_smGetAllPos(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_SET_SERVO_SPEED 0X44
void cmd_smSetSpeed(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_SET_ALL_SPEEDS  0X45
void cmd_smSetAllSpeed(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_GET_SERVO_SPEED 0X46
void cmd_smGetSpeed(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_GET_ALL_SPEEDS  0X47
void cmd_smGetAllSpeed(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_FREE_SERVO      0X48
void cmd_smFree(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// SERVO_MOTOR_FREE_ALL_SERVOS 0X49
void cmd_smFreeAll(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// Commands for DistanceSensor (ds)

// DISTANCE_SENSOR_ON_DISTANCE_SENSOR		0X40
void cmd_dsOn(char * data)
{
	short id;
    
    if (data == NULL || sscanf(data, "%hd", &id) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	dsHandler->enable(id);
    return;
}

// DISTANCE_SENSOR_OFF_DISTANCE_SENSOR		0X41
void cmd_dsOff(char * data)
{
	short id;
    
    if (data == NULL || sscanf(data, "%hd", &id) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	dsHandler->disable(id);
    return;
  
}

// DISTANCE_SENSOR_ENABLE_DISTANCE_SENSORS		0X42
void cmd_dsSetMask(char * data)
{
    unsigned int id;
    
    if (data == NULL || sscanf(data, "%x", &id) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }

	dsHandler->setMask(id);
    return;
}

// DISTANCE_SENSOR_GET_STATUS			0X43
void cmd_dsGetMask(char * data)
{
    dsHandler->getMask();
    return;
}

// DISTANCE_SENSOR_GET_VALUE           0X44
void cmd_dsGetValue(char * data)
{
	int sensorMask;
	
	if (data == NULL || sscanf(data, "%x", &sensorMask) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
	
	dsHandler->getValue(sensorMask);
    return;
}

// DISTANCE_SENSOR_GET_ONE_VALUE          0X45
void cmd_dsGetOneValue(char * data)
{
    int sensorMask;
    
	if (data == NULL || sscanf(data, "%x", &sensorMask) != 1)
    {
        printf("Wrong parameters\n");
        return;
    }
	
	dsHandler->getOneValue(sensorMask);
    return;
}

// DISTANCE_SENSOR_ALARM_ON_STATE			0X46
void cmd_dsAlarmOn(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// DISTANCE_SENSOR_SWITCH_ALARM			0X47
void cmd_dsAlarmCommand(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// Commands for BatteryController (bc)

// BATTERY_CONTROLLER_ENABLE           0X40
void cmd_bcEnable(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_DISABLE          0X41
void cmd_bcDisable(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_GET_BATTERY_VALUE        0X42
void cmd_bcGetValue(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_BATTERY_FULL_ALARM       0X43
void cmd_bcFullAlarm(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_SET_BATTERY_EMPTY_VALUE  0X44
void cmd_bcSetEmptyValue(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_BATTERY_EMPTY_ALARM      0X45
void cmd_bcEmptyAlarm(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// BATTERY_CONTROLLER_SET_FULL_BATTERY_VALUE   0X46
void cmd_bcSetFullValue(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// Commands for TrashBin (tb)

// TRASH_BIN_GET_TRASH_BIN_VALUE   0X40
void cmd_tbGetValue(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// TRASH_BIN_BIN_FULL_ALARM    0X41
void cmd_tbFullAlarm(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}

// TRASH_BIN_SET_FULL_BIN_VALUE    0X42
void cmd_tbSetFullValue(char * data)
{
    // TODO
    printf("NOT YET IMPLEMENTED\n");
    return;
}
