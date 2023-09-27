#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "EDMReader.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(PyEDMReader, m) {
  m.doc() = "This is the EDMReader module";

  py::class_<EDMReader::EDMReader>(m, "EDMReader")
      .def(py::init<const std::string>())
      .def("get_event", &EDMReader::EDMReader::getEvent, "Get the event at the index", py::arg("idx"))
      .def("size", &EDMReader::EDMReader::size, "Return the length of the reader");

  py::class_<EDMReader::Event>(m, "Event")
      .def_readonly("truth", &EDMReader::Event::truth)
      .def_readonly("detsim_hits", &EDMReader::Event::detsim_hits)
      .def_readonly("calib_hits", &EDMReader::Event::calib_hits);

  py::class_<EDMReader::Truth>(m, "Truth")
      .def_readonly("edep", &EDMReader::Truth::edep)
      .def_readonly("Qedep", &EDMReader::Truth::Qedep)
      .def_readonly("evis", &EDMReader::Truth::evis)
      .def_readonly("edepX", &EDMReader::Truth::edepX)
      .def_readonly("edepY", &EDMReader::Truth::edepY)
      .def_readonly("edepZ", &EDMReader::Truth::edepZ);
  py::class_<EDMReader::Hit>(m, "Hit")
      .def_readonly("pmtID", &EDMReader::Hit::pmtID)
      .def_readonly("charge", &EDMReader::Hit::charge)
      .def_readonly("tofh", &EDMReader::Hit::tofh);

  m.attr("__version__") = "0.3.0";
}
