#ifndef MARADRONE_UTILS_QUINTIC_TRAJECTORY_H_
#define MARADRONE_UTILS_QUINTIC_TRAJECTORY_H_

#include <Eigen/Dense>
#include <cmath>
#include <maradrone_utils/attitude_utils.h>

namespace maradrone_utils {

// Generates a quintic time-scaling trajectory (zero velocity/acceleration at
// both endpoints) along the straight line between pos_i and pos_f in
// (x, y, z, yaw) space. Ported as-is from GoToPoint::compute_trajectory_setpoint.
class QuinticTrajectory {
public:
	QuinticTrajectory() = default;

	QuinticTrajectory(const Eigen::Vector4d &pos_i, const Eigen::Vector4d &pos_f, double T) {
		reset(pos_i, pos_f, T);
	}

	void reset(const Eigen::Vector4d &pos_i, const Eigen::Vector4d &pos_f, double T) {
		T_ = T;
		pos_i_ = pos_i;

		e_ = pos_f - pos_i;
		e_(3) = angleError(pos_f(3), pos_i(3));
		s_f_ = e_.norm();

		Eigen::VectorXd b(6);
		Eigen::Matrix<double, 6, 6> A;

		b << 0.0, 0.0, 0.0, s_f_, 0.0, 0.0;
		A << 0, 0, 0, 0, 0, 1,
			0, 0, 0, 0, 1, 0,
			0, 0, 0, 1, 0, 0,
			pow(T, 5), pow(T, 4), pow(T, 3), pow(T, 2), T, 1,
			5 * pow(T, 4), 4 * pow(T, 3), 3 * pow(T, 2), 2 * T, 1, 0,
			20 * pow(T, 3), 12 * pow(T, 2), 6 * T, 1, 0, 0;

		coeffs_ = A.inverse() * b;
	}

	// Samples position/velocity/acceleration at time t (not clamped to [0, T];
	// callers are expected to stop sampling once t > duration()).
	void sample(double t, Eigen::Vector4d &pos, Eigen::Vector4d &vel, Eigen::Vector4d &acc) const {
		const Eigen::VectorXd &x = coeffs_;

		double s = x(0) * std::pow(t, 5.0)
			+ x(1) * std::pow(t, 4.0)
			+ x(2) * std::pow(t, 3.0)
			+ x(3) * std::pow(t, 2.0)
			+ x(4) * t
			+ x(5);

		double s_d = 5.0 * x(0) * std::pow(t, 4.0)
			+ 4.0 * x(1) * std::pow(t, 3.0)
			+ 3.0 * x(2) * std::pow(t, 2.0)
			+ 2.0 * x(3) * t
			+ x(4);

		double s_dd = 20.0 * x(0) * std::pow(t, 3.0)
			+ 12.0 * x(1) * std::pow(t, 2.0)
			+ 6.0 * x(2) * t
			+ x(3);

		// Guard against a zero-length leg (pos_f == pos_i), which would
		// otherwise divide by s_f_ == 0. Assigning per-branch (instead of a
		// ternary) avoids mixing Eigen's CwiseBinaryOp/CwiseNullaryOp
		// expression-template types, which have no common type.
		Eigen::Vector4d dir;
		if (s_f_ > 1e-9) {
			dir = e_ / s_f_;
		} else {
			dir = Eigen::Vector4d::Zero();
		}

		pos = pos_i_ + s * dir;
		vel = s_d * dir;
		acc = s_dd * dir;
	}

	double duration() const noexcept { return T_; }

private:
	double T_{0.0};
	double s_f_{0.0};
	Eigen::Vector4d pos_i_{Eigen::Vector4d::Zero()};
	Eigen::Vector4d e_{Eigen::Vector4d::Zero()};
	Eigen::VectorXd coeffs_{Eigen::VectorXd::Zero(6)};
};

} // namespace maradrone_utils

#endif // MARADRONE_UTILS_QUINTIC_TRAJECTORY_H_
