import os as _os
import ctypes as _ctypes

for path in __path__:
    _ctypes.cdll.LoadLibrary(_os.path.join(path, "libEDMReader.so"))

from .PyEDMReader import EDMReader, Event, Hit, Truth, __doc__, __version__
from _jaedmreader import JaEDMReader, JaEDMReaderConfig


__all__ = [
        "EDMReader",
        "Event",
        "Hit",
        "Truth",
        "JaEDMReader",
        "JaEDMReaderConfig",
        "__doc__",
        "__version__"
        ]
