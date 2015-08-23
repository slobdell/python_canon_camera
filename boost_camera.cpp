#include <stdio.h>
#include <fcntl.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION //disable annoying compilation warnings

#include "pyport.h"
#include "Python.h"

#include <iostream>
#include <vector>

#include <gphoto2/gphoto2-camera.h>

#include <boost/python.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

#include <numpy/ndarrayobject.h>

boost::python::object stdVecToNumpyArray( std::vector<char> const& vec )
    // took this function from http://stackoverflow.com/questions/10701514/how-to-return-numpy-array-from-boostpython
{
    npy_intp size = vec.size();
    char* data = size ? const_cast<char *>(&vec[0]) : static_cast<char *>(NULL);
    PyObject *pyObj = PyArray_SimpleNewFromData( 1, &size, NPY_UINT8, data );
    boost::python::handle<> handle( pyObj );
    boost::python::numeric::array arr(handle);
    return arr.copy(); // copy the object. numpy owns the copy now.
}
boost::python::object empty_numpy_array(){
    return stdVecToNumpyArray(std::vector<char>());
}
boost::python::tuple std_vector_to_tuple(std::vector<char> const& vec){
    boost::python::list copy_list;
    for(int i=0; i < vec.size(); i++){
        copy_list.append(vec[i]);
    }
    return boost::python::tuple(copy_list);
}


boost::python::object get_frame_from_live_view(Camera *camera, GPContext *context){
    int error;
    CameraFile *file;
    gp_file_new(&file);
    error = gp_camera_capture_preview(camera, file, context);
    if(error){
        return empty_numpy_array();
    }
    unsigned long int size;
    const char *data;
    gp_file_get_data_and_size(file, &data, &size);
    int actual_size = size;
    std::vector<char> jpeg_bytes(data, data + actual_size);
    gp_file_free(file);
    return stdVecToNumpyArray(jpeg_bytes);
}

// TODO DRY the function above
boost::python::object gphoto_file_to_jpeg_bytes(CameraFile *file){
    unsigned long int size;
    const char *data;
    gp_file_get_data_and_size(file, &data, &size);
    int actual_size = size;
    std::vector<char> jpeg_bytes(data, data + actual_size);
    return stdVecToNumpyArray(jpeg_bytes);

}

boost::python::object grab_photo_from_camera(Camera *camera, CameraFilePath &camera_file_path, GPContext *context){
    char *folder = (camera_file_path.folder);
    char *file = (camera_file_path.name);

    CameraFile *camera_file;
    gp_file_new(&camera_file);
    int error = gp_camera_file_get(camera, folder, file, GP_FILE_TYPE_NORMAL, camera_file, context);
    if(error){
        return empty_numpy_array();
    }
    boost::python::object jpeg_bytes = gphoto_file_to_jpeg_bytes(camera_file);
    gp_file_free(camera_file);
    return jpeg_bytes;
}

boost::python::object take_picture_and_transfer_from_camera(Camera *camera, GPContext *context){
    // bad code, this is copied and pasted from take_pic_to_camera
    CameraFilePath camera_file_path;
    gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);
    return grab_photo_from_camera(camera, camera_file_path, context);
}

int take_picture_to_camera(Camera *camera, GPContext *context){
    int error;
    CameraFilePath camera_file_path;
    error = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);
    if (error) {
        return 0;
    }
    return 1;
}

int init_camera_and_context(Camera* &camera, GPContext* &context){
    gp_camera_new(&camera);
    context = gp_context_new();
    int camera_found = gp_camera_init(camera, context);
    if (camera_found < GP_OK) {
        gp_camera_free(camera);
        return 0;
    }
    return 1;
}

void close_camera_and_context(Camera *camera, GPContext *context){
    gp_camera_unref(camera);
    gp_context_unref(context);
}

void initialize_env(){
    Py_Initialize(); // initialize the python module
    import_array(); // This is a function from NumPy that MUST be called.
    boost::python::numeric::array::set_module_and_type("numpy", "ndarray");  // required for the function to convert vector to numpy array
}

class CameraManager{
    private:
        Camera *camera;
        GPContext *context;
        int camera_ready;
    public:
        CameraManager();
        ~CameraManager();
        bool initialize();
        bool camera_is_ready(){return this->camera_ready;};
        boost::python::object grab_frame();
        int take_picture();
        boost::python::object take_picture_and_transfer();
};
CameraManager::CameraManager(){
}
CameraManager::~CameraManager(){
    if(this->camera_ready){
        close_camera_and_context(this->camera, this->context);
    }
}

bool CameraManager::initialize(){
    initialize_env();
    this->camera_ready = init_camera_and_context(this->camera, this->context);
    return this->camera_ready;
}

boost::python::object CameraManager::grab_frame(){
    if(!this->camera_ready){
        return empty_numpy_array();
    }
    return get_frame_from_live_view(this->camera, this->context);
}
int CameraManager::take_picture(){
    if(!this->camera_ready){
        return 0;
    }
    return take_picture_to_camera(this->camera, this->context);
}

boost::python::object CameraManager::take_picture_and_transfer(){
    if(!this->camera_ready){
        return empty_numpy_array();
    }
    return take_picture_and_transfer_from_camera(this->camera, this->context);

}

using namespace boost::python;
BOOST_PYTHON_MODULE(boost_camera) // this parameter needs to match filename
{
    class_<CameraManager>("CameraManager")
        .def("initialize", &CameraManager::initialize)
        .def("grab_frame", &CameraManager::grab_frame)
        .def("take_picture", &CameraManager::take_picture)
        .def("take_picture_and_transfer", &CameraManager::take_picture_and_transfer)
        .def("camera_is_ready", &CameraManager::camera_is_ready);
}
