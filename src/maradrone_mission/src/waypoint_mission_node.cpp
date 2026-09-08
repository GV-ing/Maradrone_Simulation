#include <iostream>
#include <vector>
#include <rclcpp/rclcpp.hpp>
#include <px4_msgs/msg/vehicle_local_position.hpp>
#include <px4_msgs/msg/vehicle_attitude.hpp>
#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <maradrone_utils/attitude_utils.h>
#include <maradrone_utils/quintic_trajectory.h>

using namespace std::chrono_literals;
using namespace px4_msgs::msg;

// Flies a fixed sequence of waypoints in PX4 offboard mode, one quintic
// trajectory leg at a time, reusing the same trajectory generator as
// offboard_rl/go_to_point. Waypoints are loaded from ROS parameters (see
// config/waypoints.yaml): parallel arrays waypoints.x/y/z/yaw/duration/hold_time,
// all the same length. z is expressed directly in the PX4 NED frame (negative
// = above ground).
class WaypointMission : public rclcpp::Node
{
public:
	struct Waypoint {
		double x, y, z, yaw, duration, hold_time;
	};

	enum class State { WAITING_ARM, TRAJECTORY, HOLD, MISSION_COMPLETE };

	WaypointMission() : Node("waypoint_mission")
	{
		this->declare_parameter<double>("trajectory_rate_hz", 50.0);
		rate_hz_ = this->get_parameter("trajectory_rate_hz").as_double();
		dt_ = 1.0 / rate_hz_;

		waypoints_ = loadWaypoints();
		if (waypoints_.empty()) {
			RCLCPP_FATAL(this->get_logger(), "No waypoints loaded (waypoints.x/y/z/yaw/duration/hold_time). Mission aborted.");
			rclcpp::shutdown();
			return;
		}
		RCLCPP_INFO(this->get_logger(), "Loaded %zu waypoint(s)", waypoints_.size());

		rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
		auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);

		local_position_subscription_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
			"/fmu/out/vehicle_local_position", qos,
			std::bind(&WaypointMission::vehicle_local_position_callback, this, std::placeholders::_1));
		attitude_subscription_ = this->create_subscription<px4_msgs::msg::VehicleAttitude>(
			"/fmu/out/vehicle_attitude", qos,
			std::bind(&WaypointMission::vehicle_attitude_callback, this, std::placeholders::_1));

		offboard_control_mode_publisher_ = this->create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
		trajectory_setpoint_publisher_ = this->create_publisher<px4_msgs::msg::TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);
		vehicle_command_publisher_ = this->create_publisher<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command", 10);

		timer_ = this->create_wall_timer(
			std::chrono::duration<double>(dt_),
			std::bind(&WaypointMission::timer_callback, this));
	}

private:
	std::vector<Waypoint> loadWaypoints()
	{
		this->declare_parameter<std::vector<double>>("waypoints.x", std::vector<double>{});
		this->declare_parameter<std::vector<double>>("waypoints.y", std::vector<double>{});
		this->declare_parameter<std::vector<double>>("waypoints.z", std::vector<double>{});
		this->declare_parameter<std::vector<double>>("waypoints.yaw", std::vector<double>{});
		this->declare_parameter<std::vector<double>>("waypoints.duration", std::vector<double>{});
		this->declare_parameter<std::vector<double>>("waypoints.hold_time", std::vector<double>{});

		auto x = this->get_parameter("waypoints.x").as_double_array();
		auto y = this->get_parameter("waypoints.y").as_double_array();
		auto z = this->get_parameter("waypoints.z").as_double_array();
		auto yaw = this->get_parameter("waypoints.yaw").as_double_array();
		auto duration = this->get_parameter("waypoints.duration").as_double_array();
		auto hold_time = this->get_parameter("waypoints.hold_time").as_double_array();

		size_t n = x.size();
		std::vector<Waypoint> waypoints;
		if (n == 0 || y.size() != n || z.size() != n || yaw.size() != n || duration.size() != n || hold_time.size() != n) {
			RCLCPP_ERROR(this->get_logger(),
				"waypoints.x/y/z/yaw/duration/hold_time must all be non-empty and have the same length "
				"(got x=%zu y=%zu z=%zu yaw=%zu duration=%zu hold_time=%zu)",
				x.size(), y.size(), z.size(), yaw.size(), duration.size(), hold_time.size());
			return {};
		}

		for (size_t i = 0; i < n; ++i) {
			waypoints.push_back(Waypoint{x[i], y[i], z[i], yaw[i], duration[i], hold_time[i]});
		}
		return waypoints;
	}

	void vehicle_local_position_callback(const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg)
	{
		current_position_ = *msg;
	}

	void vehicle_attitude_callback(const px4_msgs::msg::VehicleAttitude::SharedPtr msg)
	{
		current_attitude_ = *msg;
	}

	void publish_offboard_control_mode()
	{
		OffboardControlMode msg{};
		msg.position = true;
		msg.velocity = false;
		msg.acceleration = false;
		msg.attitude = false;
		msg.body_rate = false;
		msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
		offboard_control_mode_publisher_->publish(msg);
	}

	void publish_vehicle_command(uint16_t command, float param1 = 0.0f, float param2 = 0.0f)
	{
		VehicleCommand msg{};
		msg.command = command;
		msg.param1 = param1;
		msg.param2 = param2;
		msg.target_system = 1;
		msg.target_component = 1;
		msg.source_system = 1;
		msg.source_component = 1;
		msg.from_external = true;
		msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
		vehicle_command_publisher_->publish(msg);
	}

	void publish_trajectory_setpoint(const Eigen::Vector4d &pos, const Eigen::Vector4d &vel, const Eigen::Vector4d &acc)
	{
		TrajectorySetpoint msg{};
		msg.position = {float(pos(0)), float(pos(1)), float(pos(2))};
		msg.velocity = {float(vel(0)), float(vel(1)), float(vel(2))};
		msg.acceleration = {float(acc(0)), float(acc(1)), float(acc(2))};
		msg.yaw = float(pos(3));
		msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
		trajectory_setpoint_publisher_->publish(msg);
	}

	Eigen::Vector4d waypointVector(const Waypoint &wp) const
	{
		return Eigen::Vector4d(wp.x, wp.y, wp.z, wp.yaw);
	}

	void startLegTo(size_t wp_index)
	{
		Eigen::Vector4d pos_i;
		if (wp_index == 0) {
			auto rpy = maradrone_utils::quatToRpy(Eigen::Vector4d(
				current_attitude_.q[0], current_attitude_.q[1], current_attitude_.q[2], current_attitude_.q[3]));
			pos_i = Eigen::Vector4d(current_position_.x, current_position_.y, current_position_.z, rpy[2]);
		} else {
			pos_i = waypointVector(waypoints_[wp_index - 1]);
		}

		const Waypoint &wp = waypoints_[wp_index];
		trajectory_.reset(pos_i, waypointVector(wp), wp.duration);
		leg_t_ = 0.0;
		state_ = State::TRAJECTORY;
		RCLCPP_INFO(this->get_logger(), "Waypoint %zu/%zu: heading to (%.2f, %.2f, %.2f, yaw=%.2f) over %.1fs",
			wp_index + 1, waypoints_.size(), wp.x, wp.y, wp.z, wp.yaw, wp.duration);
	}

	void timer_callback()
	{
		publish_offboard_control_mode();

		if (state_ == State::WAITING_ARM) {
			if (offboard_setpoint_counter_ == 10) {
				publish_vehicle_command(VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
				publish_vehicle_command(VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);
				RCLCPP_INFO(this->get_logger(), "Offboard mode requested, arm command sent");
				startLegTo(current_wp_index_);
			}
			// Keep publishing a position setpoint at the current pose while
			// waiting, so PX4 accepts the offboard mode switch.
			publish_trajectory_setpoint(
				Eigen::Vector4d(current_position_.x, current_position_.y, current_position_.z, 0.0),
				Eigen::Vector4d::Zero(), Eigen::Vector4d::Zero());
			if (offboard_setpoint_counter_ < 11) offboard_setpoint_counter_++;
			return;
		}

		if (state_ == State::TRAJECTORY) {
			Eigen::Vector4d pos, vel, acc;
			trajectory_.sample(leg_t_, pos, vel, acc);
			publish_trajectory_setpoint(pos, vel, acc);
			leg_t_ += dt_;

			if (leg_t_ > trajectory_.duration()) {
				hold_elapsed_ = 0.0;
				last_setpoint_ = waypointVector(waypoints_[current_wp_index_]);
				state_ = State::HOLD;
			}
			return;
		}

		if (state_ == State::HOLD) {
			publish_trajectory_setpoint(last_setpoint_, Eigen::Vector4d::Zero(), Eigen::Vector4d::Zero());
			hold_elapsed_ += dt_;

			if (hold_elapsed_ > waypoints_[current_wp_index_].hold_time) {
				current_wp_index_++;
				if (current_wp_index_ < waypoints_.size()) {
					startLegTo(current_wp_index_);
				} else {
					RCLCPP_INFO(this->get_logger(), "Mission complete, holding last waypoint");
					state_ = State::MISSION_COMPLETE;
				}
			}
			return;
		}

		if (state_ == State::MISSION_COMPLETE) {
			publish_trajectory_setpoint(last_setpoint_, Eigen::Vector4d::Zero(), Eigen::Vector4d::Zero());
		}
	}

	std::vector<Waypoint> waypoints_;
	size_t current_wp_index_{0};

	State state_{State::WAITING_ARM};
	maradrone_utils::QuinticTrajectory trajectory_;
	double leg_t_{0.0};
	double hold_elapsed_{0.0};
	Eigen::Vector4d last_setpoint_{Eigen::Vector4d::Zero()};

	double rate_hz_{50.0};
	double dt_{1.0 / 50.0};
	uint64_t offboard_setpoint_counter_{0};

	VehicleLocalPosition current_position_{};
	VehicleAttitude current_attitude_{};

	rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr local_position_subscription_;
	rclcpp::Subscription<px4_msgs::msg::VehicleAttitude>::SharedPtr attitude_subscription_;
	rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr offboard_control_mode_publisher_;
	rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher_;
	rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr vehicle_command_publisher_;
	rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
	std::cout << "Starting waypoint_mission node..." << std::endl;
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<WaypointMission>());
	rclcpp::shutdown();
	return 0;
}
