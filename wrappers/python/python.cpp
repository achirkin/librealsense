/* License: Apache 2.0. See LICENSE file in root directory.
Copyright(c) 2017 Intel Corporation. All Rights Reserved. */

#include "python.hpp"
#include "../include/librealsense2/rs.hpp"

PYBIND11_MODULE(NAME, m) {
    m.doc() = R"pbdoc(
        LibrealsenseTM Python Bindings
        ==============================
        Library for accessing Intel RealSenseTM cameras
    )pbdoc";

    init_c_files(m);
    init_types(m);
    init_frame(m);
    init_options(m);
    init_processing(m);
    init_sensor(m);
    init_device(m);
    init_record_playback(m);
    init_context(m);
    init_pipeline(m);
    init_internal(m);
    init_export(m);
    init_advanced_mode(m);
    init_util(m);
    py::class_<rs2::save_to_ply, rs2::filter> save_to_ply(m, "save_to_ply"); // No docstring in C++
    save_to_ply.def(py::init<std::string, rs2::pointcloud>(), "filename"_a = "RealSense Pointcloud ", "pc"_a = rs2::pointcloud())
        .def_readonly_static("option_ignore_color", &rs2::save_to_ply::OPTION_IGNORE_COLOR)
        .def_readonly_static("option_ply_mesh", &rs2::save_to_ply::OPTION_PLY_MESH)
        .def_readonly_static("option_ply_binary", &rs2::save_to_ply::OPTION_PLY_BINARY)
        .def_readonly_static("option_ply_normals", &rs2::save_to_ply::OPTION_PLY_NORMALS);

    /** rs.hpp **/
    m.def("log_to_console", &rs2::log_to_console, "min_severity"_a);
    m.def("log_to_file", &rs2::log_to_file, "min_severity"_a, "file_path"_a);
    // rs2::log?
    /** end rs.hpp **/
}
