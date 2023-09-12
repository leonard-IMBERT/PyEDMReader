import os
import ctypes

for path in __path__:
    ctypes.cdll.LoadLibrary(os.path.join(path, "libEDMReader.so"))

from .PyEDMReader import EDMReader, Event, Hit, Truth, __doc__, __version__
from .JaEDMReader import JaEDMReader, JaEDMReaderConfig

janne = {
        "JaEDMReader": JaEDMReader,
        "JaEDMReaderConfig": JaEDMReaderConfig
        }

__all__ = [
        "EDMReader",
        "Event",
        "Hit",
        "Truth",
        "janne",
        "__doc__",
        "__version__"
        ]
