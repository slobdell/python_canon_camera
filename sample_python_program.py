from boost_camera import CameraManager
import os

MAX_CONNECTION_RETRIES = 5

camera_manager = CameraManager()
num_retries = 0

# For Macs in particular, this daemon needs to be killed to free the camera for
# us
print "Killing interfering camera processes..."
os.popen("killall PTPCamera")
print "Trying to connect to the camera..."
while num_retries < MAX_CONNECTION_RETRIES:
    success = camera_manager.initialize()
    if success:
        print "Successfully connected to the camera"
        break
    num_retries += 1

for frame_number in xrange(10):
    # this byte array can easily be integrating into OpenCV or the Python
    # Imaging Library
    numpy_array_jpeg_bytes = camera_manager.grab_frame()
    print numpy_array_jpeg_bytes

# lastly, take a picture that saves directly to the camera
print "Taking a picture"
print camera_manager.take_picture_and_transfer()
