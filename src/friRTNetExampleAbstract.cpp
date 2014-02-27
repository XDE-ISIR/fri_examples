// Copyright 2014 ISIR-CNRS
// Author: Guillaume Hamon

#include "friRTNetExampleAbstract.hpp"


FriRTNetExampleAbstract::FriRTNetExampleAbstract(std::string const& name) : RTT::TaskContext(name){


  // this->addAttribute("toFRI", m_toFRI);
  // this->addAttribute("fromFRI", m_fromFRI);

    this->addPort("RobotState_i", iport_robot_state);
    this->addPort("FriState_i", iport_Fri_state);
    this->addPort("MsrJntPos_i", iport_msr_joint_pos);
    this->addPort("CmdJntPos_i", iport_cmd_joint_pos);
    this->addPort("CmdJntPosFriOffset_i", iport_cmd_joint_pos_fri_offset);
    this->addPort("MsrCartPos_i", iport_msr_cart_pos);
    this->addPort("CmdCartPos_i", iport_cmd_cart_pos);
    this->addPort("CmdCartPosFriOffset_i", iport_cmd_cart_pos_fri_offset);
    this->addPort("MsrJntTrq_i", iport_msr_joint_trq);
    this->addPort("EstExtJntTrq_i", iport_est_ext_joint_trq);
    this->addPort("EstExtTcpWrench_i", iport_est_ext_tcp_wrench);
   // this->addPort("MassMatrix_i", iport_mass_matrix);

    this->addPort("JointPositions_o", oport_joint_position);
    this->addPort("JointVelocities_o", oport_joint_velocities);
    this->addPort("JointTorques_o", oport_add_joint_trq);
    this->addPort("CartesianPosition_o", oport_cartesian_pose);
    this->addPort("CartesianVelocity_o", oport_cartesian_twist);
    this->addPort("CartesianWrench_o", oport_cartesian_wrench);


    this->addOperation("getFRIMode", &FriRTNetExampleAbstract::getFRIMode, this, RTT::OwnThread);
    this->addOperation("setPeer", &FriRTNetExampleAbstract::setPeer, this, RTT::OwnThread);

    this->addOperation("setControlStrategy", &FriRTNetExampleAbstract::setControlStrategy, this, RTT::OwnThread);

    this->addOperation("friStart", &FriRTNetExampleAbstract::friStart, this, RTT::OwnThread);
    this->addOperation("friStop", &FriRTNetExampleAbstract::friStop, this, RTT::OwnThread);
    this->addOperation("stopKrlScript", &FriRTNetExampleAbstract::stopKrlScript, this, RTT::OwnThread);

    LWRDOF = 7;
}

FriRTNetExampleAbstract::~FriRTNetExampleAbstract(){
}

bool FriRTNetExampleAbstract::configureHook(){
	setPeer("lwr");
    //initialize the arrays that will be send to KRL
    for(int i=0; i<16; ++i){
        fri_to_krl.intData[i]=0;
        fri_to_krl.realData[i]=0.0;
    }

    //We chose to put 1 on the $FRI_FRM_INT[1] to trigger the fri_start()
    //In KRL, index starts at 1
    fri_to_krl.intData[0]=1;
    //Set control strategy to joint position
    fri_to_krl.intData[1]=10;
   /* controlMode = 10;*/ //this is control mode 1 for friRTNet

    return true;
}

bool FriRTNetExampleAbstract::startHook(){
    initializeCommand();
    return doStart();
}

bool FriRTNetExampleAbstract::doStart(){
    friStart();
    return true;
}

void FriRTNetExampleAbstract::stopHook(){
    //Reset all commands sent to the fri
    initializeCommand();
    doStop();
}

void FriRTNetExampleAbstract::doStop(){
    friStop();
    //stopKrlScript();
}

void FriRTNetExampleAbstract::cleanupHook(){}

void FriRTNetExampleAbstract::setPeer(std::string name){
	peer = getPeer(name);
	assert(peer);
        m_toFRI = peer->attributes()->getAttribute("toKRL");
	m_fromFRI= peer->attributes()->getAttribute("fromKRL");
}

void FriRTNetExampleAbstract::setControlStrategy(int mode){
    if(mode != 1 && mode != 2 && mode != 3 && mode != 4 && mode != 5 && mode != 6){
        std::cout << "Please set a valid control mode: " << std::endl;
        std::cout << "1: Joint position" << std::endl;
        std::cout << "2: Joint velocity" << std::endl;
        std::cout << "3: Joint torque" << std::endl; 
	std::cout << "4: Cartesian position" << std::endl;
        std::cout << "5: Cartesian force" << std::endl;
	std::cout << "6: Cartesian Twist" << std::endl;
        return;
    }
    else{
        RTT::Property<int> control_mode_prop = peer->properties()->getProperty("control_mode");
        control_mode_prop.set(mode);
        if (mode == 1 || mode == 2){
            fri_to_krl.intData[1] = 10;
            controlMode = 10;
        }
        else if (mode == 3){
            fri_to_krl.intData[1] = 30;
            controlMode = 30;
        }
        else if (mode == 4 || mode == 5 || mode == 6){
            fri_to_krl.intData[1] = 20;
            controlMode = 20;
        }

        m_toFRI.set(fri_to_krl);
    }
}

bool FriRTNetExampleAbstract::requiresControlMode(int modeRequired){
    if (controlMode == modeRequired){
        return true;
    }
    else{
        std::cout << "Cannot proceed, current control mode is " << controlMode
            << " required control mode is " << modeRequired << std::endl;
        return false;
    }
}

void FriRTNetExampleAbstract::getFRIMode(){

    //RTT::FlowStatus fri_frm_krl_fs = port_fri_frm_krl.read(fri_frm_krl);
	fri_frm_krl = m_fromFRI.get();
   // if(fri_frm_krl_fs == RTT::NewData){
        if(fri_frm_krl.intData[0] == 1){
            std::cout << "FRI in Command Mode" << std::endl;
        }else{
	    if(fri_frm_krl.intData[0] == 2){
            std::cout << "FRI in Monitor Mode" << std::endl;
            }else{
        	std::cout << "Cannot read FRI Mode" << std::endl;
    	    }
       }
    /*else{
        std::cout << "Cannot read FRI Mode" << std::endl;
    }*/
}

void FriRTNetExampleAbstract::friStop(){

    //Put 2 in $FRI_FRM_INT[1] to trigger fri_stop()
    fri_to_krl.intData[0]=2;
    m_toFRI.set(fri_to_krl);
    return;
}

void FriRTNetExampleAbstract::friStart(){

    //Put 1 in $FRI_FRM_INT[1] to trigger fri_stop()
    fri_to_krl.intData[0]=1;
    m_toFRI.set(fri_to_krl);
    return;
}

void FriRTNetExampleAbstract::stopKrlScript(){

    //Put 3 in $FRI_FRM_INT[1] to trigger fri_stop()
    fri_to_krl.intData[0]=3;
    m_toFRI.set(fri_to_krl);
}


void FriRTNetExampleAbstract::initializeCommand(){
    //Get current joint position and set it as desired position
    if (oport_joint_position.connected()){
        std::vector<double> fri_joint_state_data;
        //RTT::FlowStatus fri_joint_state_fs = iport_fri_joint_state.read(fri_joint_state_data);
	RTT::FlowStatus fri_joint_state_fs = iport_msr_joint_pos.read(fri_joint_state_data);
        if (fri_joint_state_fs == RTT::NewData){
            std::vector<double> joint_position_command_init;
            joint_position_command_init.assign(7, 0.0);
            for (unsigned int i = 0; i < LWRDOF; i++){
                joint_position_command_init[i] = fri_joint_state_data[i];
            }
            oport_joint_position.write(joint_position_command_init);
        }
    }

    if(oport_cartesian_pose.connected()){
        //Get cartesian position and set it as desired position
        geometry_msgs::Pose cartPosData;
        RTT::FlowStatus cart_pos_fs = iport_msr_cart_pos.read(cartPosData);
        if (cart_pos_fs == RTT::NewData){
            oport_cartesian_pose.write(cartPosData);
        }
    }

    if(oport_cartesian_wrench.connected()){
        //Send 0 torque and force
        geometry_msgs::Wrench cart_wrench_command;
        cart_wrench_command.force.x = 0.0;
        cart_wrench_command.force.y = 0.0;
        cart_wrench_command.force.z = 0.0;
        cart_wrench_command.torque.x = 0.0;
        cart_wrench_command.torque.y = 0.0;
        cart_wrench_command.torque.z = 0.0;

        oport_cartesian_wrench.write(cart_wrench_command);
    }

    if (oport_add_joint_trq.connected()){
        //Send 0 joint torque

        std::vector<double> joint_eff_command;
        joint_eff_command.assign(LWRDOF, 0.0);
        oport_add_joint_trq.write(joint_eff_command);

    }

}