#include "fairino_hardware/fairino_hardware_interface.hpp"

namespace fairino_hardware{

hardware_interface::CallbackReturn FairinoHardwareInterface::on_init(const hardware_interface::HardwareInfo& sysinfo){
    if (hardware_interface::SystemInterface::on_init(sysinfo) != hardware_interface::CallbackReturn::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }
    info_ = sysinfo;// info_ is a variable defined in the parent class
    
    for (const hardware_interface::ComponentInfo& joint : info_.joints) {

        // Command interfaces section
        if (joint.command_interfaces.size() != 1) {// ServoJ enabled
            RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
                        "Joint '%s' has %zu command interfaces found. 1 expected.", joint.name.c_str(),
                        joint.command_interfaces.size());
            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
            RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
                   "Joint '%s' have %s command interfaces found as first command interface. '%s' expected.",
                   joint.name.c_str(), joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
            return hardware_interface::CallbackReturn::ERROR;
        }

        // if (joint.command_interfaces[1].name != hardware_interface::HW_IF_EFFORT){// Reserved for direct joint torque control
        //     RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
        //            "Joint '%s' have %s command interfaces found as first command interface. '%s' expected.",
        //            joint.name.c_str(), joint.command_interfaces[1].name.c_str(), hardware_interface::HW_IF_EFFORT);
        //     return hardware_interface::CallbackReturn::ERROR;
        // }

        // Joint state interfaces section
        if (joint.state_interfaces.size() != 1) {
            RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"), "Joint '%s' has %zu state interface. 3 expected.",
                        joint.name.c_str(), joint.state_interfaces.size());
            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
            RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
                        "Joint '%s' have %s state interface as first state interface. '%s' expected.", joint.name.c_str(),
                        joint.state_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
            return hardware_interface::CallbackReturn::ERROR;
        }

        // if (joint.state_interfaces[1].name != hardware_interface::HW_IF_VELOCITY) {
        //     RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
        //                 "Joint '%s' have %s state interface as second state interface. '%s' expected.", joint.name.c_str(),
        //                 joint.state_interfaces[1].name.c_str(), hardware_interface::HW_IF_VELOCITY);
        //     return hardware_interface::CallbackReturn::ERROR;
        // }

        // if (joint.state_interfaces[2].name != hardware_interface::HW_IF_EFFORT) {
        //     RCLCPP_FATAL(rclcpp::get_logger("FairinoHardwareInterface"),
        //                 "Joint '%s' have %s state interface as third state interface. '%s' expected.", joint.name.c_str(),
        //                 joint.state_interfaces[2].name.c_str(), hardware_interface::HW_IF_EFFORT);
        //     return hardware_interface::CallbackReturn::ERROR;
        // }

    }
    return hardware_interface::CallbackReturn::SUCCESS;
}//end on_init



std::vector<hardware_interface::StateInterface> FairinoHardwareInterface::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  // Export joint-related state interfaces (position, velocity, torque)
  for (size_t i = 0; i < info_.joints.size(); ++i){
    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &_jnt_position_state[i]));

    // state_interfaces.emplace_back(hardware_interface::StateInterface(
    //     info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &_jnt_velocity_state.at(i)));

    // state_interfaces.emplace_back(hardware_interface::StateInterface(
    //     info_.joints[i].name, hardware_interface::HW_IF_EFFORT, &_jnt_torque_state.at(i)));
  }

  // Export
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> FairinoHardwareInterface::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &_jnt_position_command[i]));

//     command_interfaces.emplace_back(hardware_interface::CommandInterface(// Reserved torque control interface
//         info_.joints[i].name, hardware_interface::HW_IF_EFFORT, &_jnt_torque_command.at(i)));
  }

  return command_interfaces;
}



hardware_interface::CallbackReturn FairinoHardwareInterface::on_activate(const rclcpp_lifecycle::State& previous_state)
{
    using namespace std::chrono_literals;
    RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Starting ...please wait...");
    // Variable initialization
    _ptr_robot = std::make_unique<FRRobot>();// Create robot instance
    for(int i=0;i<6;i++){// Initialize variables
        _jnt_position_command[i] = 0;
        _jnt_velocity_command[i] = 0;
        _jnt_torque_command[i] = 0;
        _jnt_position_state[i] = 0;
        _jnt_velocity_state[i] = 0;
        _jnt_torque_state[i] = 0;
    }
    _control_mode = 0;// Default is position control: 0-position, 1-torque, 2-velocity
    errno_t returncode = _ptr_robot->RPC(_controller_ip.c_str());// Establish XML-RPC connection
    rclcpp::sleep_for(200ms);// Wait for the controller RPC connection to be established
    if(returncode != 0){
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Robot SDK connection failed! Please check if the port is occupied.");
        return hardware_interface::CallbackReturn::ERROR;
    }else{
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Robot SDK connection successful!");
    }
    // First step: read current state data
    JointPos jntpos;
    returncode = _ptr_robot->GetActualJointPosDegree(0,&jntpos);
    /*
    After obtaining the feedback position, sync it to the command position to maintain the current state.
    If reading fails, the plugin cannot be activated, because incorrect feedback positions
    would cause severe deviations in the initial command position, leading to accidents.
    */
    if(returncode == 0){
        for(int j=0;j<6;j++){
            _jnt_position_command[j] = jntpos.jPos[j]/180.0*M_PI;
        }
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"),"Initial command position: %f,%f,%f,%f,%f,%f",_jnt_position_command[0],\
        _jnt_position_command[1],_jnt_position_command[2],_jnt_position_command[3],_jnt_position_command[4],_jnt_position_command[5]);    
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Robot hardware started successfully!");
        return hardware_interface::CallbackReturn::SUCCESS;
    }else{
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Failed to read initial joint angles, hardware cannot start! Please check the communication.");
        return hardware_interface::CallbackReturn::ERROR;
    }
}



hardware_interface::CallbackReturn FairinoHardwareInterface::on_deactivate(const rclcpp_lifecycle::State& previous_state)
{
    RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Stopping ...please wait...");
    _ptr_robot->StopMotion();// Stop the robot
    _ptr_robot->CloseRPC();// Destroy instance and disconnect
    _ptr_robot.release();
    RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "System successfully stopped!");
    return hardware_interface::CallbackReturn::SUCCESS;
}



hardware_interface::return_type FairinoHardwareInterface::read(const rclcpp::Time& time,const rclcpp::Duration& period)
{// Get required position, velocity and torque data from RTDE feedback
    JointPos state_data;
    error_t returncode = _ptr_robot->GetActualJointPosDegree(1,&state_data);
    if(returncode == 0){
        for(int i=0;i<6;i++){
            _jnt_position_state[i] = state_data.jPos[i]/180.0*M_PI;// Unit conversion: MoveIt uses radians
            //_jnt_torque_state[i] = state_data.jt_cur_tor[i];// Unit conversion needed
        }
    }else{
        return hardware_interface::return_type::ERROR;
    }
    //RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "System successfully read: %f,%f,%f,%f,%f,%f",_jnt_position_state[0],\
    _jnt_position_state[1],_jnt_position_state[2],_jnt_position_state[3],_jnt_position_state[4],_jnt_position_state[5]);

  return hardware_interface::return_type::OK;

}

hardware_interface::return_type FairinoHardwareInterface::write(const rclcpp::Time& time,const rclcpp::Duration& period)
{
    if(_control_mode == 0){// Position control mode
        if (std::any_of(&_jnt_position_command[0], &_jnt_position_command[5],\
            [](double c) { return not std::isfinite(c); })) {
            return hardware_interface::return_type::ERROR;
        }
        JointPos cmd;
        ExaxisPos extcmd{0,0,0,0};
        for(auto j=0;j<6;j++){
            cmd.jPos[j] = _jnt_position_command[j]/M_PI*180; // Unit conversion (radians to degrees)
        }
        //RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "ServoJ command position: %f,%f,%f,%f,%f,%f",\
            cmd.jPos[0],cmd.jPos[1],cmd.jPos[2],cmd.jPos[3],cmd.jPos[4],cmd.jPos[5]);
        int returncode = _ptr_robot->ServoJ(&cmd,&extcmd,0,0,0.008,0,0);
        if(returncode != 0){
            RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "ServoJ command error, error code: %d",returncode);
        }
    }else if(_control_mode == 1){// Torque control mode
        if (std::any_of(&_jnt_torque_command[0], &_jnt_torque_command[5],\
            [](double c) { return not std::isfinite(c); })) {
            return hardware_interface::return_type::ERROR;
        }
        //_ptr_robot->write(_jnt_torque_command);// Unit conversion needed
    }else{
        RCLCPP_INFO(rclcpp::get_logger("FairinoHardwareInterface"), "Command error: unrecognized control mode");
        return hardware_interface::return_type::ERROR;
    }
 
    return hardware_interface::return_type::OK;
}


}//end namesapce

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(fairino_hardware::FairinoHardwareInterface, hardware_interface::SystemInterface)
