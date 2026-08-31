#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <cmath>
#include <px4_msgs/msg/vehicle_local_position.hpp>
#include <px4_msgs/msg/vehicle_attitude.hpp>
#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <maradrone_utils/attitude_utils.h>
#include <maradrone_utils/quintic_trajectory.h>
#include <maradrone_utils/px4_topics.h>

using namespace std::chrono_literals;
using namespace px4_msgs::msg;

class GoToPoint : public rclcpp::Node
{
	public:
	GoToPoint() : Node("go_to_point")
	{
		this->declare_parameter<double>("trajectory_rate_hz", 50.0);
		rate_hz_ = this->get_parameter("trajectory_rate_hz").as_double();
		dt_ = 1.0 / rate_hz_;

		rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
		auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);

		local_position_subscription_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
		maradrone_utils::px4_topic<px4_msgs::msg::VehicleLocalPosition>("/fmu/out/vehicle_local_position"),
		qos, std::bind(&GoToPoint::vehicle_local_position_callback, this, std::placeholders::_1));
		attitude_subscription_ = this->create_subscription<px4_msgs::msg::VehicleAttitude>(
		maradrone_utils::px4_topic<px4_msgs::msg::VehicleAttitude>("/fmu/out/vehicle_attitude"),
		qos, std::bind(&GoToPoint::vehicle_attitude_callback, this, std::placeholders::_1));
		offboard_control_mode_publisher_ = this->create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
		trajectory_setpoint_publisher_ = this->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
		maradrone_utils::px4_topic<px4_msgs::msg::TrajectorySetpoint>("/fmu/in/trajectory_setpoint"), 10);
		vehicle_command_publisher_ = this->create_publisher<px4_msgs::msg::VehicleCommand>(
		maradrone_utils::px4_topic<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command"), 10);

		timer_offboard_ = this->create_wall_timer(100ms, std::bind(&GoToPoint::activate_offboard, this));
		timer_trajectory_publish_ = this->create_wall_timer(
			std::chrono::duration<double>(dt_),
			std::bind(&GoToPoint::publish_trajectory_setpoint, this));

		keyboard_thread = std::thread(&GoToPoint::keyboard_listener, this);
	}

	private:
	rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr local_position_subscription_;
	rclcpp::Subscription<px4_msgs::msg::VehicleAttitude>::SharedPtr attitude_subscription_;
	rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr offboard_control_mode_publisher_;
	rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr vehicle_command_publisher_;
	rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher_;

	rclcpp::TimerBase::SharedPtr timer_offboard_;
	rclcpp::TimerBase::SharedPtr timer_trajectory_publish_;

	std::thread keyboard_thread;

	bool set_point_received{false};
	bool offboard_active{false};
	bool trajectory_computed{false};
	maradrone_utils::QuinticTrajectory trajectory_;

	double rate_hz_{50.0};
	double dt_{1.0 / 50.0};
	double T, t{0.0};
	Eigen::Vector4d pos_i, pos_f;
	VehicleLocalPosition current_position_{};
	VehicleAttitude current_attitude_{};
	double offboard_counter{0};

	void vehicle_local_position_callback(const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg)
	{
		current_position_ = *msg;
	}

	void vehicle_attitude_callback(const px4_msgs::msg::VehicleAttitude::SharedPtr msg)
	{
		current_attitude_ = *msg;
	}

	void keyboard_listener()
	{
		// Set x, y, z, yaw setpoints
		while (rclcpp::ok() && !set_point_received)
		{
			std::cout << "Enter setpoints as x y z yaw (meters, meters, meters, radiants) & time to complete path (seconds): ";
			std::string line;
			std::getline(std::cin, line);
			std::istringstream iss(line);
			if (!(iss >> pos_f(0) >> pos_f(1) >> pos_f(2) >> pos_f(3) >> T)) {
				std::cout << "Invalid input. Please enter FIVE numeric values." << std::endl;
				continue;
			}
			else {
				pos_f(2) = -pos_f(2); // PX4 uses NED frame
				set_point_received = true;
				std::cout << "Setpoints received: x=" << pos_f(0) << ", y=" << pos_f(1) << ", z=" << pos_f(2) << ", yaw=" << pos_f(3) << ", T=" << T << std::endl;
				std::cout << "Activating offboard mode" << std::endl;
				std::cout << "----------------------------------------" << std::endl;
			}
		}
	}

	void activate_offboard()
	{
		if (set_point_received)
		{
			if(offboard_counter == 10) {
				// Change to Offboard mode after 1 second of sending offboard messages
				VehicleCommand msg{};
				msg.param1 = 1;
				msg.param2 = 6;
				msg.command = VehicleCommand::VEHICLE_CMD_DO_SET_MODE;
				msg.target_system = 1;
				msg.target_component = 1;
				msg.source_system = 1;
				msg.source_component = 1;
				msg.from_external = true;
				msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
				vehicle_command_publisher_->publish(msg);

				// Arm the vehicle
				msg.param1 = 1.0;
				msg.param2 = 0.0;
				msg.command = VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM;
				msg.target_system = 1;
				msg.target_component = 1;
				msg.source_system = 1;
				msg.source_component = 1;
				msg.from_external = true;
				msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
				vehicle_command_publisher_->publish(msg);

				// Set initial position
				pos_i(0) = current_position_.x;
				pos_i(1) = current_position_.y;
				pos_i(2) = current_position_.z;

				auto rpy = maradrone_utils::quatToRpy( Eigen::Vector4d( current_attitude_.q[0], current_attitude_.q[1], current_attitude_.q[2], current_attitude_.q[3] ) );
				pos_i(3) = rpy[2];

				offboard_active = true;
			}

			OffboardControlMode msg{};
			msg.position = true;
			msg.velocity = false;
			msg.acceleration = false;
			msg.attitude = false;
			msg.body_rate = false;
			msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
			offboard_control_mode_publisher_->publish(msg);

			if (offboard_counter < 11) offboard_counter++;
		}
	}

	void publish_trajectory_setpoint()
	{
		if (!set_point_received || !offboard_active || t > T) {
			return;
		}

		TrajectorySetpoint msg{compute_trajectory_setpoint(t)};
		trajectory_setpoint_publisher_->publish(msg);
		t += dt_;
	}

	TrajectorySetpoint compute_trajectory_setpoint(double t)
	{
		if (!trajectory_computed)
		{
			trajectory_.reset(pos_i, pos_f, T);
			trajectory_computed = true;
		}

		Eigen::Vector4d ref_traj_pos, ref_traj_vel, ref_traj_acc;
		trajectory_.sample(t, ref_traj_pos, ref_traj_vel, ref_traj_acc);

		TrajectorySetpoint msg{};
		msg.position = {float(ref_traj_pos(0)), float(ref_traj_pos(1)), float(ref_traj_pos(2))};
		msg.velocity = {float(ref_traj_vel(0)), float(ref_traj_vel(1)), float(ref_traj_vel(2))};
		msg.acceleration = {float(ref_traj_acc(0)), float(ref_traj_acc(1)), float(ref_traj_acc(2))};
		msg.yaw = float(ref_traj_pos(3)); // [-PI:PI]
		msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;

		return msg;
	}
};

int main(int argc, char *argv[])
{
	std::cout << "Starting vehicle_local_position listener node..." << std::endl;
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<GoToPoint>());
	rclcpp::shutdown();
	return 0;
}
