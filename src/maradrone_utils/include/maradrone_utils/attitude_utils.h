#ifndef MARADRONE_UTILS_ATTITUDE_UTILS_H_
#define MARADRONE_UTILS_ATTITUDE_UTILS_H_

#include <Eigen/Dense>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

namespace maradrone_utils {

inline double angleError(double target, double actual) {
	double MAX_VALUE = 2.0 * M_PI;
	double signedDiff = 0.0;
	double raw_diff = actual > target ? actual - target : target - actual;
	double mod_diff = fmod(raw_diff, MAX_VALUE); //equates rollover values. E.g 0 == 360 degrees in circle

	if (mod_diff > (MAX_VALUE / 2)) {
		//There is a shorter path in opposite direction
		signedDiff = (MAX_VALUE - mod_diff);
		if (target > actual) signedDiff = signedDiff * -1;
	} else {
		signedDiff = mod_diff;
		if (actual > target) signedDiff = signedDiff * -1;
	}

	return signedDiff;
}

inline Eigen::Vector3d MatToRpy(Eigen::Matrix3d R) {
	Eigen::Vector3d rpy;

	rpy(0) = atan2(R(2, 1), R(2, 2)); // roll
	rpy(1) = atan2(-R(2, 0), sqrt(R(2, 1) * R(2, 1) + R(2, 2) * R(2, 2))); //pitch
	rpy(2) = atan2(R(1, 0), R(0, 0)); //yaw

	return rpy;
}

// Quaternion to rotation matrix
inline Eigen::Matrix3d QuatToMat(Eigen::Vector4d Quat) {
	Eigen::Matrix3d Rot;
	float s = Quat[0];
	float x = Quat[1];
	float y = Quat[2];
	float z = Quat[3];
	Rot << 1 - 2 * (y * y + z * z), 2 * (x * y - s * z), 2 * (x * z + s * y),
		2 * (x * y + s * z), 1 - 2 * (x * x + z * z), 2 * (y * z - s * x),
		2 * (x * z - s * y), 2 * (y * z + s * x), 1 - 2 * (x * x + y * y);
	return Rot;
}

inline Eigen::Vector3d quatToRpy(Eigen::Vector4d q) {
	return maradrone_utils::MatToRpy(maradrone_utils::QuatToMat(q));
}

} // namespace maradrone_utils

#endif // MARADRONE_UTILS_ATTITUDE_UTILS_H_
