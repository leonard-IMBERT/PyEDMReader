#include "EDMReader.hpp"

namespace EDMReader {

static const unsigned int MODULE_INDEX = 8;
static const unsigned int MODULE_MASK = 0x00FFFF00;

EDMReader::EDMReader(const std::string filename)
    : _filename(filename), _source_file(new TFile(_filename.c_str(), "READ")) {
  if (_source_file == nullptr) throw std::invalid_argument("Cannot read source file");

  _detsim_tree = _source_file->Get<TTree>("Event/Sim/SimEvt");
  if (_detsim_tree != nullptr) {
    _detsim_tree->SetBranchAddress("SimEvt", &_sim_evt);
  }

  _calib_tree = _source_file->Get<TTree>("Event/CdLpmtCalib/CdLpmtCalibEvt");
  if (_calib_tree != nullptr) {
    _calib_tree->SetBranchAddress("CdLpmtCalibEvt", &_calib_evt);
  }
}

// TODO: Delete TFile Access

EDMReader::~EDMReader() {
  if(_source_file != nullptr && !(_source_file->IsZombie())) {
    _source_file->Close("R");
  }

  delete _source_file;
}


long EDMReader::size() {
  if (_calib_tree != nullptr) return _calib_tree->GetEntries();
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

  if (_calib_tree != nullptr) {
    if (_calib_tree->GetEntry(idx) < 1) {
      throw std::out_of_range("The file does not enough entries in its calib tree");
    }

    evt.calib_hits = Hits(_calib_evt->calibPMTCol().size());

    size_t hit_idx = 0;
    for (const auto& channel : _calib_evt->calibPMTCol()) {
      evt.calib_hits->PutHit(hit_idx++, (channel->pmtId() & MODULE_MASK) >> MODULE_INDEX, channel->sumCharge(),
                             channel->firstHitTime());
    }
  }

  return evt;
}

}  // namespace EDMReader
