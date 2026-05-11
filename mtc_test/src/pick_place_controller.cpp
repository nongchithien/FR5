#include <atomic>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <control_msgs/action/gripper_command.hpp>
#include <moveit/move_group_interface/move_group_interface.h>

// State machine for pick & place using MoveGroupInterface (blocking plan+execute per phase)
// IDLE → APPROACH → GRASP → LIFT → PLACE → DONE

enum class State { IDLE, APPROACH, GRASP, LIFT, PLACE, DONE };

class PickPlaceController : public rclcpp::Node
{
public:
  explicit PickPlaceController(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("pick_place_controller", options),
    state_(State::IDLE),
    is_moving_(false)
  {
    this->declare_parameter("planning_group", "fr5_arm");
    this->declare_parameter("eef_frame", "wrist3_link");
    this->declare_parameter("base_frame", "base_link");
    this->declare_parameter("approach_z_offset", 0.15);  // approach from 15 cm above
    this->declare_parameter("lift_height", 0.15);        // lift 15 cm after grasp

    planning_group_   = this->get_parameter("planning_group").as_string();
    eef_frame_        = this->get_parameter("eef_frame").as_string();
    base_frame_       = this->get_parameter("base_frame").as_string();
    approach_z_offset_ = this->get_parameter("approach_z_offset").as_double();
    lift_height_       = this->get_parameter("lift_height").as_double();

    object_pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "/object_pose", 10,
        std::bind(&PickPlaceController::onObjectPose, this, std::placeholders::_1));

    gripper_client_ = rclcpp_action::create_client<control_msgs::action::GripperCommand>(
        this, "/gripper_cmd");

    // 10 Hz is plenty for this state machine
    control_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100),
        std::bind(&PickPlaceController::controlLoop, this));

    RCLCPP_INFO(this->get_logger(), "PickPlaceController ready. Waiting for /object_pose...");
  }

  // Must be called after the node is managed by a shared_ptr (i.e. after make_shared)
  void init()
  {
    arm_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(
        shared_from_this(), planning_group_);
    arm_->setPlanningTime(5.0);
    arm_->setEndEffectorLink(eef_frame_);
    RCLCPP_INFO(this->get_logger(), "MoveGroupInterface ready (group=%s, eef=%s)",
                planning_group_.c_str(), eef_frame_.c_str());
  }

private:
  // ── Callbacks ─────────────────────────────────────────────────────────────

  void onObjectPose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
  {
    latest_object_pose_ = msg;
    if (state_ == State::IDLE && !is_moving_) {
      RCLCPP_INFO(this->get_logger(), "Object detected → APPROACH");
      state_ = State::APPROACH;
    }
  }

  void controlLoop()
  {
    if (is_moving_) return;

    switch (state_) {
      case State::IDLE:     break;
      case State::APPROACH: doApproach(); break;
      case State::GRASP:    doGrasp();    break;
      case State::LIFT:     doLift();     break;
      case State::PLACE:    doPlace();    break;
      case State::DONE:
        RCLCPP_INFO_ONCE(this->get_logger(), "Pick & place complete.");
        break;
    }
  }

  // ── Phase: APPROACH ───────────────────────────────────────────────────────

  void doApproach()
  {
    if (!latest_object_pose_ || !arm_) return;

    geometry_msgs::msg::PoseStamped target;
    target.header.frame_id = base_frame_;
    target.pose = latest_object_pose_->pose;
    target.pose.position.z += approach_z_offset_;

    RCLCPP_INFO(this->get_logger(), "APPROACH → (%.3f, %.3f, %.3f)",
                target.pose.position.x, target.pose.position.y, target.pose.position.z);

    moveAsync(target, [this](bool ok) {
      if (ok) {
        RCLCPP_INFO(this->get_logger(), "APPROACH done → GRASP");
        state_ = State::GRASP;
      } else {
        RCLCPP_WARN(this->get_logger(), "APPROACH failed, will retry");
      }
    });
  }

  // ── Phase: GRASP ──────────────────────────────────────────────────────────

  void doGrasp()
  {
    if (!gripper_client_->wait_for_action_server(std::chrono::seconds(0))) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                           "Gripper action server not available");
      return;
    }

    is_moving_ = true;

    auto goal = control_msgs::action::GripperCommand::Goal();
    goal.command.position = 0.0;    // close
    goal.command.max_effort = 20.0;

    auto opts = rclcpp_action::Client<control_msgs::action::GripperCommand>::SendGoalOptions();
    opts.result_callback = [this](const auto& result) {
      is_moving_ = false;
      if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
        RCLCPP_INFO(this->get_logger(), "Gripper closed → LIFT");
        state_ = State::LIFT;
      } else {
        RCLCPP_ERROR(this->get_logger(), "Gripper close failed, retrying");
        state_ = State::GRASP;
      }
    };
    gripper_client_->async_send_goal(goal, opts);
  }

  // ── Phase: LIFT ───────────────────────────────────────────────────────────

  void doLift()
  {
    if (!latest_object_pose_ || !arm_) return;

    geometry_msgs::msg::PoseStamped target;
    target.header.frame_id = base_frame_;
    target.pose = latest_object_pose_->pose;
    target.pose.position.z += approach_z_offset_ + lift_height_;

    RCLCPP_INFO(this->get_logger(), "LIFT → z=%.3f", target.pose.position.z);

    moveAsync(target, [this](bool ok) {
      if (ok) {
        RCLCPP_INFO(this->get_logger(), "LIFT done → PLACE");
        state_ = State::PLACE;
      } else {
        RCLCPP_WARN(this->get_logger(), "LIFT failed, will retry");
      }
    });
  }

  // ── Phase: PLACE ──────────────────────────────────────────────────────────

  void doPlace()
  {
    if (!arm_) return;

    RCLCPP_INFO(this->get_logger(), "PLACE: returning to home");
    is_moving_ = true;
    state_ = State::DONE;

    std::thread([this]() {
      arm_->setNamedTarget("home");
      auto result = arm_->move();
      if (result != moveit::core::MoveItErrorCode::SUCCESS) {
        RCLCPP_WARN(this->get_logger(), "PLACE move failed (code=%d)", result.val);
      }
      is_moving_ = false;
    }).detach();
  }

  // ── Helpers ───────────────────────────────────────────────────────────────

  void moveAsync(const geometry_msgs::msg::PoseStamped& target,
                 std::function<void(bool)> on_done)
  {
    is_moving_ = true;
    std::thread([this, target, on_done]() {
      arm_->setPoseTarget(target);
      bool ok = (arm_->move() == moveit::core::MoveItErrorCode::SUCCESS);
      is_moving_ = false;
      on_done(ok);
    }).detach();
  }

  // ── Members ───────────────────────────────────────────────────────────────

  State state_;
  std::atomic<bool> is_moving_;
  geometry_msgs::msg::PoseStamped::SharedPtr latest_object_pose_;

  std::string planning_group_;
  std::string eef_frame_;
  std::string base_frame_;
  double approach_z_offset_;
  double lift_height_;

  std::shared_ptr<moveit::planning_interface::MoveGroupInterface> arm_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr object_pose_sub_;
  rclcpp_action::Client<control_msgs::action::GripperCommand>::SharedPtr gripper_client_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<PickPlaceController>(options);
  node->init();

  // MultiThreadedExecutor required so arm_->move() (blocking) can still
  // process MoveGroup action callbacks while running in a detached thread.
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
