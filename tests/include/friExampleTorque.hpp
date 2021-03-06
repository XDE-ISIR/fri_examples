// Copyright (C) 2014 ISIR-CNRS
// Author: Sovannara Hak

#ifndef FRI_EXAMPLE_TORQUE_HPP
#define FRI_EXAMPLE_TORQUE_HPP

#include "friExampleAbstract.hpp"

#include <string>

#include <rtt/RTT.hpp>

#include <lwr_fri/typekit/Types.hpp>
#include <kuka_lwr_fri/friComm.h>

#include <motion_control_msgs/typekit/Types.hpp>
#include <sensor_msgs/typekit/Types.hpp>

class FriExampleTorque : public FriExampleAbstract{
    public:
        FriExampleTorque(std::string const& name);
        ~FriExampleTorque();

        void updateHook();

        bool doStart();

        lwr_fri::FriJointState fri_joint_state_data;

        /** @brief Set stiffness and damping for joint 
         * @param stiffness The desired stiffness for the joint (Nm/rad)
         * @param damping The desired damping (Normalized)
         */
        void setJointImpedance(std::vector<double> &stiffness, std::vector<double> &damping);

        void getFRIJointState();
        void setT(double t);
        double torque;
};

#endif

