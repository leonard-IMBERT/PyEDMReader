#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "EDMReader.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
  m.doc() = "This module aim to provide a python reader for the EDM format of the JUNO collaboration."
    " The file read by this module are READONLY, this module DOES NOT PROVIDE EDITION CAPABILITIES.";

  auto _C_EDMReader =
      py::class_<EDMReader::EDMReader>(m, "EDMReader")
          .def(py::init<const std::string>(), R"DOC(
      EDMReader(file: str)

      Create and EDMReader object. This object open an EDM
      ROOT file for reading. Under the hood, the root file
      is kept open while the EDMReader object is alive.
      )DOC",
               py::arg("filename"))
          .def("get_event", &EDMReader::EDMReader::getEvent, "Get the event at the index idx", py::arg("idx"))
          .def("size", &EDMReader::EDMReader::size, "Return the number of event in the EDM file");
  _C_EDMReader.attr("__doc__") = R"DOC(
  The EDMReader will try to read the Monte Carlo truth of
  the event (see the Truth class) and the hits collection at
  the detsim and calib level (see the Hits class).

  If any of those information is missing, the corresponding
  field will be set to None.
  )DOC";

  auto _C_Event = py::class_<EDMReader::Event>(m, "Event")
      .def_readonly("truth", &EDMReader::Event::truth, "Truth about the event")
      .def_readonly("detsim_hits", &EDMReader::Event::detsim_hits, "Collection of PMTs hits at the detsim level")
      .def_readonly("calib_hits", &EDMReader::Event::calib_hits, "Collection of PMTs hits at the calib level");

  _C_Event.attr("__doc__") = R"DOC(
  An event, i.e. the monte carlo truth, the detsim hits and the calib hits of an event.
  If one of these information is missing from the source file, the property is set to None.

  The exact definition of what is an event depend on the file read by the EDMReader.
  If the EDMReader read the event from a detsim level file, an Event correspond to one
  Geant4 Event.
  If the EDMReader read the event from a calib level file, an Event correspond to one trigger
  of the event trigger system. It mean that there can be more or less event that what was simulated
  using Geant4 !
  )DOC";

  auto _C_Truth = py::class_<EDMReader::Truth>(m, "Truth")
      .def_readonly("edep", &EDMReader::Truth::edep, "Deposited energy")
      .def_readonly("Qedep", &EDMReader::Truth::Qedep, "Quenched deposited energy")
      .def_readonly("evis", &EDMReader::Truth::evis, "Visible energy -- always 0 for now")
      .def_readonly("edepX", &EDMReader::Truth::edepX, "X position of the event")
      .def_readonly("edepY", &EDMReader::Truth::edepY, "Y position of the event")
      .def_readonly("edepZ", &EDMReader::Truth::edepZ, "Z position of the event");
  _C_Truth.attr("__doc__") = R"DOC(
  The Monte Carlo truth about an Event.

  This truth has been verified to be correct for positron
  and electron events.
  If you are reading any other kind of event, the position might be wrong.
  )DOC";

  auto _C_Hits = py::class_<EDMReader::Hits>(m, "Hits", py::buffer_protocol())
      .def_buffer([](EDMReader::Hits &h) -> py::buffer_info {
        return py::buffer_info(h.data_buffer(), sizeof(double), py::format_descriptor<double>::format(), 2,
                               {(py::ssize_t)h.size(), (py::ssize_t)3},
                               {(py::ssize_t)(sizeof(double) * 3), (py::ssize_t)sizeof(double)});
      })
      .def("size", &EDMReader::Hits::size, "Get the number of hits in the collection")
      .def("__getitem__", &EDMReader::Hits::GetHit);
  _C_Hits.attr("__doc__") = R"DOC(
  Collection of Hits.

  This class expose two way to retrieve the hits.

  - Buffer protocol (recommended). By using the numpy library, you can directly read the event collection into a numpy array:
    ```
    import numpy as np

    ...

    event = reader.get_event(idx)
    hits = np.array(evet.calib_hits)

    ```
    The resulting array will have the dimension (N_hits x 3) with, indexed in the following order, pmtID, charge and time.
  - Direct access. You can retrieve a singular Hit object by accessing the hit via its index.
    ```
    event = reader.get_event(idx)
    hit = evet.calib_hits[0]
    ```

  )DOC";

  auto _C_Hit = py::class_<EDMReader::Hit>(m, "Hit")
      .def_readonly("pmtID", &EDMReader::Hit::pmtID, "Id of the PMT")
      .def_readonly("charge", &EDMReader::Hit::charge, "Charge collected of the PMT")
      .def_readonly("tofh", &EDMReader::Hit::tofh, "Time of collection of the first charge");
  _C_Hit.attr("__doc__") = R"DOC(
  A single hit
  )DOC";


  m.attr("__version__") = "0.5.2";
}
