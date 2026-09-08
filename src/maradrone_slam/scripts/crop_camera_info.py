#!/usr/bin/env python3
"""Republishes a CameraInfo with width/height/K/P corrected for a centered
crop.

image_proc's CropDecimateNode crops the image itself correctly, but only
updates the roi/binning fields of the outgoing CameraInfo, leaving
width/height (and K/P) equal to the original, uncropped sensor. RTAB-Map's
mapping node (util3d.cpp::cloudFromDepthRGB) does not know about the ROI
convention: it asserts that CameraInfo.width/height exactly match the
actual RGB image size, and aborts otherwise. This node produces a
CameraInfo that satisfies that assertion by setting width/height to the
cropped size directly and shifting the principal point (cx, cy) by the crop
offset; fx/fy are unaffected by cropping.
"""

import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import CameraInfo


class CropCameraInfoRepublisher(Node):

    def __init__(self):
        super().__init__('crop_camera_info')
        self.declare_parameter('offset_x', 0)
        self.declare_parameter('offset_y', 0)
        self.declare_parameter('width', 0)
        self.declare_parameter('height', 0)

        self.offset_x = self.get_parameter('offset_x').value
        self.offset_y = self.get_parameter('offset_y').value
        self.width = self.get_parameter('width').value
        self.height = self.get_parameter('height').value

        self.sub = self.create_subscription(
            CameraInfo, 'in/camera_info', self.callback, qos_profile_sensor_data)
        self.pub = self.create_publisher(
            CameraInfo, 'out/camera_info', qos_profile_sensor_data)

    def callback(self, msg: CameraInfo):
        out = CameraInfo()
        out.header = msg.header
        out.height = self.height
        out.width = self.width
        out.distortion_model = msg.distortion_model
        out.d = list(msg.d)
        out.k = list(msg.k)
        out.k[2] -= float(self.offset_x)  # cx
        out.k[5] -= float(self.offset_y)  # cy
        out.r = list(msg.r)
        out.p = list(msg.p)
        out.p[2] -= float(self.offset_x)  # cx (P)
        out.p[6] -= float(self.offset_y)  # cy (P)
        out.binning_x = msg.binning_x
        out.binning_y = msg.binning_y
        # roi left at the default (all-zero => "full image"), which now
        # correctly means the full *cropped* image.
        self.pub.publish(out)


def main():
    rclpy.init()
    node = CropCameraInfoRepublisher()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
