PYTHON_INCLUDE = /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
BOOST_INCLUDE = /usr/local/Cellar/boost/1.58.0/include
NUMPY_INCLUDE = /usr/local/Cellar/numpy/1.9.2_1/lib/python2.7/site-packages/numpy/core/include
GPHOTO_INCLUDE = /usr/local/Cellar/libgphoto2/2.5.3.1/include

BASE_LIB_DIR = /usr/local/lib
PYTHON_LIB_DIR = /System/Library/Frameworks/Python.framework/Versions/2.7/lib/

PYTHON_LIB = python2.7
BOOST_LIB = boost_python
GPHOTO_LIB = gphoto2

TARGET = boost_camera

# .so file is the file that python will import as a module
$(TARGET).so: $(TARGET).o
	g++ -shared -Wl, $(TARGET).o -L$(BASE_LIB_DIR) -L$(PYTHON_LIB_DIR) -l$(BOOST_LIB) -l$(PYTHON_LIB) -l$(GPHOTO_LIB) -I$(PYTHON_INCLUDE) -I$(BOOST_INCLUDE) -I$(NUMPY_INCLUDE) -I$(GPHOTO_INCLUDE) -o $(TARGET).so

$(TARGET).o: $(TARGET).cpp
	g++ -L$(BASE_LIB_DIR)  -L$(PYTHON_LIB_DIR) -l$(BOOST_LIB) -l$(PYTHON_LIB) -l$(GPHOTO_LIB) -I$(PYTHON_INCLUDE) -I$(BOOST_INCLUDE) -I$(NUMPY_INCLUDE) -I$(GPHOTO_INCLUDE) -fPIC -c $(TARGET).cpp
