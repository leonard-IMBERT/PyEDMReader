import os as _os
import ctypes as _ctypes

for path in __path__:
    _ctypes.cdll.LoadLibrary(_os.path.join(path, "libEDMReader.so"))

from .PyEDMReader import __doc__, __version__
from ._jaedmreader import JaEDMReader, JaEDMReaderConfig, EventMode


__all__ = [
        "JaEDMReader",
        "JaEDMReaderConfig",
        "EventMode",
        "__doc__",
        "__version__"
        ]
