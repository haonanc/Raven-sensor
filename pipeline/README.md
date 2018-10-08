# Stereolabs ZED - Pipeline

This script records 24 frames every second from the Stereolabs ZED stereo camera feed. It keeps both depth and RGB channels and save them in a single pgn file, with left camera color channel on the left and depth channel on the right. Color channel is used for object detection. After objects being detected, depth channel utilize the object 2d coordinates to calculate object 3d locations.


# Dependencies
- ZED SDK on [stereolabs.com](https://www.stereolabs.com).
- For more information, read the ZED [API documentation](https://www.stereolabs.com/developers/documentation/API/).
