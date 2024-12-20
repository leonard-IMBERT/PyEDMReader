#ifndef EDM_READER_H
#define EDM_READER_H

#include <Event/CdLpmtCalibEvt.h>
#include <Event/CdSpmtCalibEvt.h>
#include <Event/SimEvt.h>
#include <TBuffer.h>
#include <TDirectory.h>
#include <TDirectoryFile.h>
#include <TFile.h>
#include <TObject.h>
#include <TTree.h>

#include <optional>
#include <string>
#include <sstream>

const std::string SPMT_ID_FILE = "pmt_20230809_CDSPMT.csv";
const std::string LPMT_ID_FILE = "pmt_20230831_CDLPMT.csv";


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
  double pmtID;
  /**
   * @charge Charge collected by the pmt [Npe]
   * */
  double charge;
  /**
   * @tofh Time of first hit of the pmt [ns]
   **/
  double tofh;
};

struct Hits {
  Hits(size_t n_hits) : _n_hits(n_hits) { _data = std::shared_ptr<double[]>(new double[_n_hits * 3]); }

  /**
   * @brief Put a hit at the designated index
   *
   * @param hit_idx Index to modify
   * @param pmtID Id of the PMT hit
   * @param charge Charge in the PMT (recommended in Npe)
   * @param tofh Time Of First Hit (recommended in ns)
   */
  void PutHit(size_t hit_idx, long pmtID, double charge, double tofh) {
    _data[hit_idx * 3] = (double)pmtID;
    _data[(hit_idx * 3) + 1] = charge;
    _data[(hit_idx * 3) + 2] = tofh;
  }

  /**
   * @brief Get the hit at the index hit_idx
   *
   * @param hit_idx The index of the hit to get
   * @throws std::out_of_range Throw an exception if you try to access an index outside the size of the collection
   * @return A Hit struct containin the requested hit
   */
  const Hit GetHit(size_t hit_idx) const {
    if(hit_idx >= _n_hits) {
      std::stringstream s;
      s << "Hit with index " << hit_idx << " is out of range of hit collection of size " << _n_hits;
      throw std::out_of_range(s.str());
    }

    return Hit{_data[hit_idx * 3], _data[(hit_idx * 3) + 1], _data[(hit_idx * 3) + 2]};
  }

  /**
   * @brief Return the size of the collection
   *
   * @return The size of the collection
   */
  size_t size() const { return _n_hits; }

  /**
   * @brief Return a raw pointer to the underlying data
   *
   * DO NOT DELETE THIS POINTER. The Hits class handle its deallocation.
   *
   * @return A raw pointer to the underlying data
   */
  double* data_buffer() { return _data.get(); }

 private:
  std::shared_ptr<double[]> _data;
  size_t _n_hits;
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
  std::optional<Hits> detsim_hits;
  /**
   * @calib_hits Hits at calib level
   **/
  std::optional<Hits> calib_hits;
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
   * @brief Destruct an EDMReader
   **/
  ~EDMReader();

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

  JM::CdLpmtCalibEvt* _lpmt_calib_evt = nullptr;
  TTree* _lpmt_calib_tree = nullptr;

  JM::CdSpmtCalibEvt* _spmt_calib_evt = nullptr;
  TTree* _spmt_calib_tree = nullptr;

  std::unordered_map<unsigned int, unsigned int> id2copyNo;
};

}  // namespace EDMReader

#endif
