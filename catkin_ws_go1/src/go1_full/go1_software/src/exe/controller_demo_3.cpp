#include "unitree_legged_sdk/unitree_legged_sdk.h"
#include <math.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

#include <unitree_legged_msgs/HighCmd.h>
#include <unitree_legged_msgs/HighState.h>
#include "convert.h"

#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>

using namespace UNITREE_LEGGED_SDK;

#define PITCH_SIT_RAD  -0.30
#define BODY_HEIGHT  -0.1

class Custom
{
public:
    // Custom(uint8_t level): safe(LeggedType::A1), udp(level){
    //Custom(uint8_t level): safe(LeggedType::A1), udp(8090, "192.168.123.11", 8082, sizeof(HighCmd), sizeof(HighState)){
    //  Custom(uint8_t level): safe(LeggedType::A1), udp(8090, "192.168.123.161", 8082, sizeof(HighCmd), sizeof(HighState)){
    Custom(uint8_t level): safe(LeggedType::Go1), udp(8090, "192.168.123.161", 8082, sizeof(HighCmd), sizeof(HighState)){
    // Custom(uint8_t level): safe(LeggedType::Go1), udp(8090, "192.168.12.1", 8082, sizeof(HighCmd), sizeof(HighState)){
        udp.InitCmdData(cmd);
        // udp.SetDisconnectTime(dt, 1);
        // udp.SetDisconnectTime(0, 0);
    }
    void UDPRecv();
    void UDPSend();
    void RobotPosInit(void);

    void velcommand_to_udp(const geometry_msgs::Twist& cmd_vel);
    void posecommand_to_udp(const geometry_msgs::Twist& cmd_pose);
    void handcommand_to_udp(const std_msgs::String& cmd_hand);


    void velCallback(const geometry_msgs::Twist::ConstPtr& cmd_vel);
    void modeCallback(const std_msgs::String::ConstPtr& audio_mode);
    void handCallback(const std_msgs::String::ConstPtr& cmd_hand);




    void ROSInit();
    //

    Safety safe;
    UDP udp;
    HighCmd cmd = {0};
    HighState state = {0};
    unitree_legged_msgs::HighCmd SendHighROS;
    unitree_legged_msgs::HighState RecvHighROS;
    
    //added
    ros::Subscriber subVel;
    ros::Subscriber subHand;
    ros::Subscriber subMode;


    ros::NodeHandle nh;
    //

    int motiontime = 0;
    float dt = 0.002;     // 0.001~0.01

    string command_mode = "stop"; //among 'stop', 'find_human', 'vel_command', 'pose_command', ...
    string hand_command = "undefined";
};

void Custom::velcommand_to_udp(const geometry_msgs::Twist& cmd_vel)
{
    //std::cout<<"commandmode is "<<command_mode<<std::endl;
    cmd.bodyHeight = 0;

    if(command_mode=="undefined"){
        cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
        cmd.gaitType = 0;
        cmd.speedLevel = 0;
        cmd.footRaiseHeight = 0;
        cmd.bodyHeight = 0;
        cmd.euler[0]  = 0;
        cmd.euler[1] = 0;
        cmd.euler[2] = 0;
        cmd.velocity[0] = 0.0f;
        cmd.velocity[1] = 0.0f;
        cmd.yawSpeed = 0.0f;
    }
    else if(command_mode=="find_human")
    {
        std::cout<<"find human"<<std::endl;
        // move forward
        if(cmd_vel.linear.x > 0 && cmd_vel.angular.z == 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[0] = 0.2f; // -1  ~ +1
            //cmd.bodyHeight = 0.1;
        }
        // move backward
        else if(cmd_vel.linear.x < 0 && cmd_vel.angular.z == 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[0] = -0.2f; // -1  ~ +1
            //cmd.bodyHeight = 0.1;
        }
        //turn ON ITSELF left
        else if(cmd_vel.linear.x == 0 && cmd_vel.angular.z > 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[2] = 0.2f; 
        }
        else if(cmd_vel.linear.x == 0 && cmd_vel.angular.z < 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[2] = -0.2f; 
        }
        else{
            cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
            cmd.gaitType = 0;
            cmd.speedLevel = 0;
            cmd.footRaiseHeight = 0;
            cmd.bodyHeight = 0;
            cmd.euler[0]  = 0;
            cmd.euler[1] = 0;
            cmd.euler[2] = 0;
            cmd.velocity[0] = 0.0f;
            cmd.velocity[1] = 0.0f;
            cmd.yawSpeed = 0.0f;
        }
        udp.SetSend(cmd);

    }
    else if(command_mode=="vel_command")
    {
        std::cout<<"human body command"<<std::endl;
        // move forward

        if(cmd_vel.linear.x > 0 && cmd_vel.linear.y == 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[0] = 0.2f; // -1  ~ +1
            //cmd.bodyHeight = 0.1;
            std::cout<<"forward"<<std::endl;
        }
        // move backward
        else if(cmd_vel.linear.x < 0 && cmd_vel.linear.y  == 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[0] = -0.2f; // -1  ~ +1
            //cmd.bodyHeight = 0.1;
            std::cout<<"backward"<<std::endl;
        }
        //turn left (follow the facing arm)
        else if(cmd_vel.linear.x == 0 && cmd_vel.linear.y  > 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[1] = -0.2f; 
            std::cout<<"left lateral"<<std::endl;
        }
        else if(cmd_vel.linear.x == 0 && cmd_vel.linear.y < 0){
            cmd.mode = 2;
            cmd.gaitType = 1;
            cmd.velocity[1] = 0.2f; 
            std::cout<<"right lateral"<<std::endl;
        }
        else{
            cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
            cmd.gaitType = 0;
            cmd.speedLevel = 0;
            cmd.footRaiseHeight = 0;
            cmd.bodyHeight = 0;
            cmd.euler[0]  = 0;
            cmd.euler[1] = 0;
            cmd.euler[2] = 0;
            cmd.velocity[0] = 0.0f;
            cmd.velocity[1] = 0.0f;
            cmd.yawSpeed = 0.0f;
    //        std::cout<<"stopped"<<std::endl;
        }
        udp.SetSend(cmd);
    }
}


void Custom::handcommand_to_udp(const std_msgs::String& cmd_hand)
{
    std::cout <<"GENERAL mode is: "<< command_mode<<std::endl;

    std::cout <<"hand mode is: "<< cmd_hand<<std::endl;
    hand_command = cmd_hand.data;
    if(command_mode=="undefined"){
        std::cout<<"in undefined"<<std::endl;
        cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
        cmd.gaitType = 0;
        cmd.speedLevel = 0;
        cmd.footRaiseHeight = 0;
        cmd.bodyHeight = 0;
        cmd.euler[0]  = 0;
        cmd.euler[1] = 0;
        cmd.euler[2] = 0;
        cmd.velocity[0] = 0.0f;
        cmd.velocity[1] = 0.0f;
        cmd.yawSpeed = 0.0f;
        std::cout<<cmd.mode<<std::endl;
        udp.SetSend(cmd);
    }
    else if(command_mode =="hand_action"){
        if(hand_command=="sit")
        {
            cmd.mode=1;
            
            cmd.euler[1] = PITCH_SIT_RAD; //pitch
            cmd.bodyHeight = 0; //body height

            udp.SetSend(cmd);
            std::cout<<"pitch angle given to udp: "<<cmd.euler[1]<<std::endl;
        }
        else if(hand_command=="down")
        {
            cmd.mode=1;
            cmd.euler[1] = 0;

            cmd.bodyHeight = BODY_HEIGHT; //body height
            udp.SetSend(cmd);
            std::cout<<"body height given to udp: "<<cmd.bodyHeight<<std::endl;
        }
        else if(hand_command=="stay")
        {
            cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
            cmd.gaitType = 0;
            cmd.speedLevel = 0;
            cmd.footRaiseHeight = 0;
            cmd.bodyHeight = 0;
            cmd.euler[0]  = 0;
            cmd.euler[1] = 0;
            cmd.euler[2] = 0;
            cmd.velocity[0] = 0.0f;
            cmd.velocity[1] = 0.0f;
            cmd.yawSpeed = 0.0f;
            udp.SetSend(cmd);
            std::cout<<"stay = init"<<std::endl;
        }

        std::cout<<cmd.mode<<std::endl;

    }
}

void Custom::modeCallback(const std_msgs::String::ConstPtr& audio_mode)
{
    //if robot was controlled by tilting, make sure that it gets to initial body pose when following velocity commands
    command_mode = audio_mode->data;
}

void Custom::handCallback(const std_msgs::String::ConstPtr& cmd_hand)
{
    std::cout<<" in hand callback !! "<<std::endl;
    Custom::handcommand_to_udp(*cmd_hand);
}

void Custom::velCallback(const geometry_msgs::Twist::ConstPtr& cmd_vel)
{
    Custom::velcommand_to_udp(*cmd_vel);
}

void Custom::ROSInit()
{
    subMode = nh.subscribe("/cmd_mode", 10, &Custom::modeCallback, this);
    subVel = nh.subscribe("/cmd_vel", 10, &Custom::velCallback, this);
    subHand = nh.subscribe("/hand_pose", 10, &Custom::handCallback, this);

}


void Custom::UDPRecv()
{
    udp.Recv();
}

void Custom::UDPSend()
{  
    ros::spinOnce();
    udp.Send();
}

void Custom::RobotPosInit(void)
{
    std::cout << "INIT !!!!!!!" << std::endl<<std::endl;
    cmd.mode = 0;    
    cmd.gaitType = 0;
    cmd.speedLevel = 0;
    cmd.footRaiseHeight = 0;
    cmd.bodyHeight = 0;
    cmd.euler[0]  = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = 0.0f;
    cmd.yawSpeed = 0.0f;
}

int main(int argc, char **argv) 
{

    // initialize ROS
    ros::init(argc, argv, "hardware_high_cmd_node");

    std::cout << "Communication level is set to HIGH-level." << std::endl
              << "WARNING: Make sure the robot is standing on the ground." << std::endl
              << "Press Enter to continue..." << std::endl;
    std::cin.ignore();


    Custom custom(HIGHLEVEL);
    InitEnvironment();

    // init robot position
    custom.RobotPosInit();

    // create a node handle
    ros::NodeHandle nh;

    // subscribe to the "/cmd_vel" topic and specify the callback function
    ros::Subscriber subHand = nh.subscribe("/hand_pose", 1000, &Custom::handCallback, &custom);
    ros::Subscriber subMode = nh.subscribe("/cmd_mode", 1000, &Custom::modeCallback, &custom);
    ros::Subscriber subVel = nh.subscribe("/cmd_vel", 1000, &Custom::velCallback, &custom);


   // LoopFunc loop_control("control_loop", custom.dt,    boost::bind(&Custom::twistCallback, &custom));
    LoopFunc loop_udpSend("udp_send",     custom.dt, 3, boost::bind(&Custom::UDPSend,      &custom));
    LoopFunc loop_udpRecv("udp_recv",     custom.dt, 3, boost::bind(&Custom::UDPRecv,      &custom));

    loop_udpSend.start();
    loop_udpRecv.start();
   // loop_control.start();

    while(1){
        sleep(10);
    };

    return 0; 
}
