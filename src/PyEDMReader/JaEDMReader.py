from typing import Optional
from janne.interfaces import IDecoder
from dataclasses import dataclass

from . import EDMReader
from enum import Enum
import numpy as np

class EventMode(Enum):
    DETSIM=0
    CALIB=1

@dataclass
class JaEDMReaderConfig:
    filepath: str
    mode: EventMode

class JaEDMReader(IDecoder):

    def __init__(self, config: Optional[JaEDMReaderConfig] = None):
        self.config : Optional[JaEDMReaderConfig] = config
        if self.config:
            self.initialize(self.config)
        self.reader: Optional[EDMReader] = None
        self.mode = EventMode.DETSIM

    def initialize(self, config: JaEDMReaderConfig):
        self.config = config
        self.reader = EDMReader(config.filepath)
        self.evt_count = 0

    def __next__(self):
        if self.reader is None or self.config is None:
            raise RuntimeError("Reader not yet initialize. Please initialize it first")

        if self.evt_count >= self.reader.size():
            raise StopIteration

        evt = self.reader.get_event(self.evt_count)
        truth = None
        if evt.truth is not None:
            truth = np.array([evt.truth.edep, evt.truth.edepX, evt.truth.edepY, evt.truth.edepZ], dtype=np.float64)
        signal = None
        if self.config.mode == EventMode.DETSIM: signal = evt.detsim_hits
        if self.config.mode == EventMode.CALIB: signal = evt.calib_hits

        if signal is None:
            raise StopIteration

        signal = np.array([[hit.pmtID, hit.charge, hit.tofh] for hit in signal], dtype=np.float64)


        self.evt_count += 1
        return (signal, truth)

