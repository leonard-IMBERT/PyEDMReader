#ifndef EDM_READER_H
#define EDM_READER_H

#include <Event/CdLpmtCalibEvt.h>
#include <Event/SimEvt.h>
#include <TBuffer.h>
#include <TDirectory.h>
#include <TDirectoryFile.h>
#include <TFile.h>
#include <TObject.h>
#include <TTree.h>

#include <optional>
#include <string>

namespace EDMReader {

/**
 * struct Truth - Struct represnting the thruth of an event
 */
struct Truth {
  /**
   * @edep Deposited energy
   */
  double edep;
  /**
   * @Qedep Quenched deposited energy
   */
  double Qedep;
  /**
   * @evis Visible deposited energy
   */
  double evis;

  /**
   * @edepX X coordinate of the energy deposit
   */
  double edepX;
  /**
   * @edepY Y coordinate of the energy deposit
   */
  double edepY;
  /**
   * @edepZ Z coordinate of the energy deposit
   */
  double edepZ;
};

/**
 * struct Hit - Struct representing a hit
 **/
struct Hit {
  /**
   * @pmtID The ID of the pmt
   **/
  long pmtID;
  /**
   * @charge Charge collected by the pmt [Npe]
   * */
  double charge;
  /**
   * @tofh Time of first hit of the pmt [ns]
   **/
  double tofh;
};

/**
 * struct Event - Struct representing an event
 **/
struct Event {
  /**
   * @truht Truth of the event
   **/
  std::optional<Truth> truth;

  /**
   * @detsim_hits Hits at detsim level
   **/
  std::optional<std::vector<Hit>> detsim_hits;
  /**
   * @calib_hits Hits at calib level
   **/
  std::optional<std::vector<Hit>> calib_hits;
};

/**
 * @brief Class representing a reader of a JUNO EDM file
 *
 * This class is thread safe if ROOT::EnableThreadSafety has been called
 **/
class EDMReader {
 public:
   /**
    * @brief construct an EDMReader from a filepath
    *
    * @param filename Filepath to the file to read
    *
    * @throw std::invalid_argument if cannot read the file
    **/
  EDMReader(const std::string filename);

  /**
   * @brief Retrieve the event located at idx
   *
   * @param idx The index of the event to read
   *
   * @throw std::out_of_range If the file does not contain event at idx
   * */
  Event getEvent(const long idx);

  /**
   * @brief Return the number of events in the file
   **/
  long size();

 private:
  std::string _filename;
  TFile* _source_file = nullptr;

  JM::SimEvt* _sim_evt = nullptr;
  TTree* _detsim_tree = nullptr;

  JM::CdLpmtCalibEvt* _calib_evt = nullptr;
  TTree* _calib_tree = nullptr;
};

}  // namespace EDMReader

#endif
