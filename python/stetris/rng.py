"""The random generator the game runs on."""

_MASK = 0xFFFFFFFF


class Rng:
    """A linear congruential generator.

    Not :mod:`random`, so that a seed produces the same sequence in every
    port of this logic.
    """

    def __init__(self) -> None:
        self._state = 1

    def seed(self, seed: int) -> None:
        """Seed the generator. A zero seed would make it stick at zero."""
        self._state = (seed & _MASK) or 1

    def next(self) -> int:
        """Return the next value."""
        self._state = (self._state * 1664525 + 1013904223) & _MASK

        # The low bits of an LCG are weak, so use the high ones
        return self._state >> 16

    def next_range(self, low: int, high: int) -> int:
        """Return the next value in ``[low, high]``."""
        if high <= low:
            return low
        return low + self.next() % (high - low + 1)
