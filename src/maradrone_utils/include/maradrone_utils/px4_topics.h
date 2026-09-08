#ifndef MARADRONE_UTILS_PX4_TOPICS_H_
#define MARADRONE_UTILS_PX4_TOPICS_H_

#include <string>

namespace maradrone_utils {

// Builds the actual DDS topic name for a px4_msgs message type, honoring
// PX4's uXRCE-DDS message-versioning scheme: a message whose generated ROS 2
// type declares a MESSAGE_VERSION >= 1 constant is published/subscribed by
// PX4 under "<base>_v<MESSAGE_VERSION>" instead of the bare topic name (see
// PX4-Autopilot's src/modules/uxrce_dds_client/utilities.hpp,
// generate_topic_name()). Always build /fmu/... topic names through this
// helper instead of hardcoding the string, so nodes stay correct whichever
// px4_msgs version they are built against, and versioning that gets added
// to a currently-unversioned message later doesn't silently break them.
//
// NOTE: only px4_msgs types that declare a MESSAGE_VERSION constant (the
// ones under PX4-Autopilot's msg/versioned/, e.g. VehicleLocalPosition,
// VehicleStatus, VehicleAttitude, TrajectorySetpoint, VehicleCommand) can be
// used with this template. A handful of messages (e.g. OffboardControlMode)
// are never versioned and have no such constant at all — keep those topic
// strings as plain literals.
template<typename T>
inline std::string px4_topic(const std::string &base)
{
	return T::MESSAGE_VERSION == 0 ? base : base + "_v" + std::to_string(T::MESSAGE_VERSION);
}

} // namespace maradrone_utils

#endif // MARADRONE_UTILS_PX4_TOPICS_H_
