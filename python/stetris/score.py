"""A score counter."""


class Score:
    def __init__(self) -> None:
        self._value = 0

    def clear(self) -> None:
        """Reset the score to zero."""
        self._value = 0

    def add(self, amount: int) -> None:
        """Add to the score."""
        self._value += amount

    def set(self, amount: int) -> None:
        """Overwrite the score, used to restore a stored high score."""
        self._value = amount

    def get(self) -> int:
        """Return the score."""
        return self._value

    @property
    def value(self) -> int:
        """The score."""
        return self._value
