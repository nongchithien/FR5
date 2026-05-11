#include <thread>

#include <geometry_msgs/msg/pose.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <rclcpp/rclcpp.hpp>

static const rclcpp::Logger LOGGER = rclcpp::get_logger("fr5_movegroup");
static const std::string PLANNING_GROUP = "fr5_arm";

// Plan and execute a motion, returns true on success
static bool planAndExecute(moveit::planning_interface::MoveGroupInterface& mgi,
                           const std::string& description)
{
  moveit::planning_interface::MoveGroupInterface::Plan plan;
  RCLCPP_INFO(LOGGER, "[%s] Planning...", description.c_str());

  auto err = mgi.plan(plan);
  if (err != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(LOGGER, "[%s] Planning failed (code %d)", description.c_str(), err.val);
    return false;
  }

  RCLCPP_INFO(LOGGER, "[%s] Executing...", description.c_str());
  if (mgi.execute(plan) != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(LOGGER, "[%s] Execution failed", description.c_str());
    return false;
  }

  RCLCPP_INFO(LOGGER, "[%s] Done", description.c_str());
  return true;
}

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<rclcpp::Node>(
    "fr5_movegroup_node",
    rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));

  // MoveGroupInterface requires a spinning executor to receive feedback
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  auto spin_thread = std::thread([&executor]() { executor.spin(); });

  moveit::planning_interface::MoveGroupInterface mgi(node, PLANNING_GROUP);
  mgi.setMaxVelocityScalingFactor(0.3);
  mgi.setMaxAccelerationScalingFactor(0.3);
  mgi.setPlanningTime(5.0);

  RCLCPP_INFO(LOGGER, "Planning frame  : %s", mgi.getPlanningFrame().c_str());
  RCLCPP_INFO(LOGGER, "End effector    : %s", mgi.getEndEffectorLink().c_str());
  RCLCPP_INFO(LOGGER, "Available named targets:");
  for (const auto& t : mgi.getNamedTargets()) {
    RCLCPP_INFO(LOGGER, "  - %s", t.c_str());
  }

  // --- Move 1: named target "home_no_arm" ---
  mgi.setNamedTarget("home_no_arm");
  planAndExecute(mgi, "move_to_home");

  rclcpp::shutdown();
  spin_thread.join();
  return 0;
}
