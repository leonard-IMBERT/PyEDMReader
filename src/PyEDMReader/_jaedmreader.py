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
        self._config : Optional[JaEDMReaderConfig] = config
        self._reader: Optional[EDMReader] = None
        self._mode = EventMode.DETSIM

        if self._config:
            self.initialize(self._config)

    def initialize(self, config: JaEDMReaderConfig):
        self._config = config
        self._reader = EDMReader(config.filepath)
        self._evt_count = 0
        self._mode = self._config.mode

    def __next__(self):
        if self._reader is None or self._config is None:
            raise RuntimeError("Reader not yet initialize. Please initialize it first")

        if self._evt_count >= self._reader.size():
            raise StopIteration

        evt = self._reader.get_event(self._evt_count)
        truth = None
        if evt.truth is not None:
            truth = np.array([evt.truth.edep, evt.truth.edepX, evt.truth.edepY, evt.truth.edepZ], dtype=np.float64)
        signal = None
        if self._mode == EventMode.DETSIM: signal = evt.detsim_hits
        if self._mode == EventMode.CALIB: signal = evt.calib_hits

        if signal is None:
            raise StopIteration

        signal = np.array([[hit.pmtID, hit.charge, hit.tofh] for hit in signal], dtype=np.float64)


        self._evt_count += 1
        return (signal, truth)

    def config(self):
        return self.config

