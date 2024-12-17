import os as _os
import ctypes as _ctypes

for path in __path__:
    _ctypes.cdll.LoadLibrary(_os.path.join(path, "libEDMReader.so"))

from ._core import __doc__, __version__, EDMReader, Hits, Event, Hit


__all__ = [
        "__doc__",
        "__version__",
        "EDMReader",
        "Hits",
        "Event",
        "Hit"
        ]
