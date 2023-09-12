from typing import Union
from janne.interfaces import IDecoder
from dataclasses import dataclass
from .PyEDMReader import EDMReader
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

    def __init__(self, config: Union[JaEDMReaderConfig, None] = None):
        super()
        if config:
            self.config = config
            self.initialize(self.config)
        self.reader: EDMReader | None = None
        self.mode = EventMode.DETSIM

    def initialize(self, config: JaEDMReaderConfig):
        self.config = config
        self.reader = EDMReader(config.filepath)

    def __next__(self):
        if self.reader is None:
            raise RuntimeError("Reader not yet initialize. Please initialize it first")

        evt_count = 0
        while evt_count < self.reader.size():
            evt = self.reader.get_event(evt_count)
            truth = None
            if evt.truth is not None:
                truth = np.array([evt.truth.edep, evt.truth.edepX, evt.truth.edepY, evt.truth.edepZ])
            signal = None
            if self.config.mode == EventMode.DETSIM: signal = evt.detsim_hits
            if self.config.mode == EventMode.CALIB: signal = evt.calib_hits

            if signal is None:
                raise StopIteration

            signal = np.array([[hit.pmtID, hit.charge, hit.tofh] for hit in signal])

            yield (signal, truth)

            evt_count += 1

