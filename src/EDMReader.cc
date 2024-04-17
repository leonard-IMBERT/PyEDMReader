#include "EDMReader.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

const unsigned int MODULE_MAX = 65535;
const unsigned int MODULE_20INCH_MIN = 0;
const unsigned int MODULE_20INCH_MAX = 17611;  //  findPmt20inchNum()-1
const unsigned int MODULE_3INCH_MIN = 17612;   //  findPmt20inchNum()
const unsigned int MODULE_3INCH_MAX = 43211;   //  findPmt20inchNum()+findPmt3inchNum()-1
const unsigned int MODULE_INDEX = 8;
const unsigned int MODULE_MASK = 0x00FFFF00;
const unsigned int OFFSET_3INCH = 300000;

const unsigned int CD_ID = 0x1;
const unsigned int WP_ID = 0x2;
const unsigned int TT_ID = 0x3;

const unsigned int SUB_INDEX = 28;
const unsigned int SUB_MASK = 0xF0000000;

const unsigned int LargeOrSmall_INDEX = 27;
const unsigned int LargeOrSmall_MASK = 0x08000000;

const unsigned int NorthOrSouth_INDEX = 26;
const unsigned int NorthOrSouth_MASK = 0x04000000;

const unsigned int Circle_INDEX = 20;
const unsigned int Circle_MASK = 0x03F00000;

const unsigned int Position_INDEX = 12;
const unsigned int Positon_MASK = 0x000FF000;

const unsigned int UpOrDown_INDEX = 11;
const unsigned int UpOrDown_MASK = 0x00000800;

const unsigned int PmtType_INDEX = 8;
const unsigned int PmtType_MASK = 0x00000700;

static bool is_cd(unsigned int id) { return (id & SUB_MASK) >> SUB_INDEX == CD_ID; }

enum ParsingStatus {
  SOF,       //< Start of file
  VALUE,     //< Next token is value
  SKIPPING,  //< Skipping tokens
  EndOF,     //< End of file
  EndOL      //< End of line
};

/**
 * @brief Read a csv file of pmt data into an unordered map
 *
 * @param filename The qualified path to the csv
 * @param[out] map The map to fill
 */
static void read_file_in_map(std::string filename, std::unordered_map<unsigned int, unsigned int>& map) {
  const char* env = std::getenv("EDM_READER_DATA");  //< Base location of the data needed by the module
  if (env == nullptr) {
    std::cerr << "Cannot read environement variable EDM_READER_DATA, please set it to the localisation of the PMT "
                 "data. Aborting"
              << std::endl;
    throw std::runtime_error(
        "Cannot read environement variable EDM_READER_DATA, please set it to the localisation of the PMT data. "
        "Aborting");
  }

  std::ifstream pmt_file(std::string(env) + "/" + filename);
  int CopyNumber, LargeOrSmall, NorthOrSouth, CircleNumber, PositionNumber, UpOrDown, PmtType, GCU, Channel;

  const int N_VALUES = 9;
  std::array<int*, N_VALUES> values = {&CopyNumber, &LargeOrSmall, &NorthOrSouth, &CircleNumber, &PositionNumber,
                                       &UpOrDown,   &PmtType,      &GCU,          &Channel};

  ParsingStatus status = SOF;  // Start in start of file mode
  int value_index = 0;
  int n_line = 0;
  for (auto next_char = pmt_file.peek(); status != EndOF;
       next_char = pmt_file.peek()) {  // While not the end of the file
    switch (next_char) {
      case '#':  // Comments
        pmt_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        break;
      case std::char_traits<char>::eof():  // End of file
        status = EndOF;
        break;
      case '.':  // Floating point number. Ingore the non integer part
        pmt_file.ignore(2, ',');
        break;
      case ',':  // Sepparator in csv
        ++value_index;
        pmt_file.ignore(1);
        break;
      case 13:  // CR character. Needed for windows files
        pmt_file.ignore(1);
        break;
      case '\n':  // LF or end of line character
        pmt_file.ignore(1);
        status = EndOL;
        break;
      default:  // If no special characters, expect value
        status = VALUE;
    }

    if (status == VALUE) {
      if (value_index >= N_VALUES) {
        std::cerr << "[Error] Malformated file. Too much enties on line " << n_line << " in file " << filename
                  << std::endl;
        exit(-1);
      }
      pmt_file >> *(values[value_index]);
      status = SKIPPING;
    }

    if (status == EndOL) {
      if (value_index < N_VALUES - 1) {
        std::cerr << "[Warn] Line " << n_line << " of file " << filename << "got too few entries" << std::endl;
      }
      unsigned int id = (CD_ID << SUB_INDEX) | (LargeOrSmall << LargeOrSmall_INDEX) |
                        (NorthOrSouth << NorthOrSouth_INDEX) | (CircleNumber << Circle_INDEX) |
                        (PositionNumber << Position_INDEX) | (UpOrDown << UpOrDown_INDEX) | (PmtType << PmtType_INDEX);
      map[id] = CopyNumber;
      status = SKIPPING;
      value_index = 0;
      ++n_line;
    }
  }
}

namespace EDMReader {

EDMReader::EDMReader(const std::string filename)
    : _filename(filename), _source_file(new TFile(_filename.c_str(), "READ")) {
  if (_source_file == nullptr) throw std::invalid_argument("Cannot read source file");

  read_file_in_map(SPMT_ID_FILE, id2copyNo);
  read_file_in_map(LPMT_ID_FILE, id2copyNo);

  _detsim_tree = _source_file->Get<TTree>("Event/Sim/SimEvt");
  if (_detsim_tree != nullptr) {
    _detsim_tree->SetBranchAddress("SimEvt", &_sim_evt);
  } else {
    std::cerr << "[Warn] No sim tree in file " << filename << std::endl;
  }

  _lpmt_calib_tree = _source_file->Get<TTree>("Event/CdLpmtCalib/CdLpmtCalibEvt");
  if (_lpmt_calib_tree != nullptr) {
    _lpmt_calib_tree->SetBranchAddress("CdLpmtCalibEvt", &_lpmt_calib_evt);
  } else {
    std::cerr << "[Warn] No lpmt calib tree in file " << filename << std::endl;
  }

  _spmt_calib_tree = _source_file->Get<TTree>("Event/CdSpmtCalib/CdSpmtCalibEvt");
  if (_spmt_calib_tree != nullptr) {
    _spmt_calib_tree->SetBranchAddress("CdSpmtCalibEvt", &_spmt_calib_evt);
  } else {
    std::cerr << "[Warn] No spmt calib tree in file " << filename << std::endl;
  }
}

EDMReader::~EDMReader() {
  if (_source_file != nullptr && !(_source_file->IsZombie())) {
    _source_file->Close("R");
  }

  delete _source_file;
}

long EDMReader::size() {
  if (_lpmt_calib_tree != nullptr) return _lpmt_calib_tree->GetEntries();
  if (_spmt_calib_tree != nullptr) return _spmt_calib_tree->GetEntries();
  if (_detsim_tree != nullptr) return _detsim_tree->GetEntries();

  return -1;
}

Event EDMReader::getEvent(const long idx) {
  Event evt;

  if (_detsim_tree != nullptr) {
    if (_detsim_tree->GetEntry(idx) < 1) {
      throw std::out_of_range("The file does not enough entries in its detsim tree");
    }

    double edep = 0;
    double qedep = 0;
    double evis = 0;

    double edepX = 0;
    double edepY = 0;
    double edepZ = 0;

    for (const auto& track : _sim_evt->getTracksVec()) {
      edep += track->getEdep();
      qedep += track->getQEdep();
      // Placeholder for when evis will be defined
      // evis += 0;

      edepX += track->getEdepX();
      edepY += track->getEdepY();
      edepZ += track->getEdepZ();
    }

    int n_tracks = _sim_evt->getTracksVec().size();

    if (n_tracks != 0) {
      edep /= n_tracks;
      qedep /= n_tracks;

      edepX /= n_tracks;
      edepZ /= n_tracks;
      edepZ /= n_tracks;
    }

    evt.truth = {edep, qedep, evis, edepX, edepY, edepZ};

    evt.detsim_hits = Hits(_sim_evt->getCDHitsVec().size());
    for (size_t hit_idx = 0; hit_idx < evt.detsim_hits->size(); ++hit_idx) {
      const auto& hit = _sim_evt->getCDHitsVec()[hit_idx];
      evt.detsim_hits->PutHit(hit_idx, hit->getPMTID(), (double)hit->getNPE(), hit->getHitTime());
    }
  }

  // Here we merge the lpmt and spmt trees into the same event.
  if (_lpmt_calib_tree != nullptr || _spmt_calib_tree != nullptr) {
    unsigned int lpmt_tot_hits = (_lpmt_calib_tree != nullptr && _lpmt_calib_tree->GetEntry(idx) > 0)
                                     ? _lpmt_calib_evt->calibPMTCol().size()
                                     : 0;
    unsigned int spmt_tot_hits = (_spmt_calib_tree != nullptr && _spmt_calib_tree->GetEntry(idx) > 0)
                                     ? _spmt_calib_evt->calibPMTCol().size()
                                     : 0;

    if ((lpmt_tot_hits + spmt_tot_hits) < 1) {
      throw std::out_of_range("The file does not enough entries in its calib trees");
    }

    evt.calib_hits = Hits(lpmt_tot_hits + spmt_tot_hits);

    size_t hit_idx = 0;
    for (const auto& channel : _lpmt_calib_evt->calibPMTCol()) {
      if (!is_cd(channel->pmtId())) continue;

      if (auto copy_no = id2copyNo.find(channel->pmtId()); copy_no == id2copyNo.end()) {
        std::cerr << "[Warning] No pmt with id 0x" << std::hex << channel->pmtId() << std::dec << ", Id will be the original file id"
                  << std::endl;
        evt.calib_hits->PutHit(hit_idx++, channel->pmtId(), channel->sumCharge(), channel->firstHitTime());
      } else {
        evt.calib_hits->PutHit(hit_idx++, copy_no->second, channel->sumCharge(), channel->firstHitTime());
      }
    }

    for (const auto& channel : _spmt_calib_evt->calibPMTCol()) {
      if (!is_cd(channel->pmtId())) continue;

      if (auto copy_no = id2copyNo.find(channel->pmtId()); copy_no == id2copyNo.end()) {
        std::cerr << "[Warning] No pmt with id 0x" << std::hex << channel->pmtId() << std::dec << ", Id will be the original file id"
                  << std::endl;
        evt.calib_hits->PutHit(hit_idx++, channel->pmtId(), channel->sumCharge(), channel->firstHitTime());
      } else {
        // Relocate the spmt_id to be 300'000 + pmt_id
        evt.calib_hits->PutHit(hit_idx++, (copy_no->second - MODULE_3INCH_MIN) + OFFSET_3INCH, channel->sumCharge(),
                               channel->firstHitTime());
      }
    }
  }

  return evt;
}

}  // namespace EDMReader
