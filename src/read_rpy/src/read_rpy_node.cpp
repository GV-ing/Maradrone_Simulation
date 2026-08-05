#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <px4_msgs/msg/vehicle_attitude.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <read_RPY/utils.h>

using namespace std::chrono_literals;

class ReadRPY : public rclcpp::Node
{
	public:
	ReadRPY() : Node("read_rpy")
	{
		rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
		auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);

		subscription_ = this->create_subscription<px4_msgs::msg::VehicleAttitude>("/fmu/out/vehicle_attitude",
		qos, std::bind(&ReadRPY::attitude_callback, this, std::placeholders::_1));
		publisher_ = this->create_publisher<geometry_msgs::msg::Vector3>("/out/rpy_info", 10);
	}

	private:
	rclcpp::Subscription<px4_msgs::msg::VehicleAttitude>::SharedPtr subscription_;
	rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr publisher_;

	rclcpp::TimerBase::SharedPtr timer_;

	bool need_land;

	void attitude_callback(const px4_msgs::msg::VehicleAttitude::UniquePtr msg) 
	{
		auto q = msg->q;
		auto rpy = utilities::quatToRpy( Vector4d( q[0], q[1], q[2], q[3] ) );

		geometry_msgs::msg::Vector3 rpy_msg;
		rpy_msg.x = rpy(0);
		rpy_msg.y = rpy(1);
		rpy_msg.z = rpy(2);
		publisher_->publish(rpy_msg);
	}

};

int main(int argc, char *argv[])
{
	std::cout << "Starting vehicle_attitude listener node..." << std::endl;
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<ReadRPY>());
	rclcpp::shutdown();
	return 0;
}