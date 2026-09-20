"""Stetris game logic.

The package knows nothing about how the game is presented. A front end
calls the input methods, hands elapsed time to :meth:`Play.tick` and reads
the state back to draw it.
"""

from .block import Block
from .board import Board
from .play import Play
from .rng import Rng
from .score import Score
from .state import State

__all__ = ["Block", "Board", "Play", "Rng", "Score", "State"]
