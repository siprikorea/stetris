package stetris;

/** A score counter. */
public final class Score {
    private long value;

    /** Resets the score to zero. */
    public void clear() {
        value = 0;
    }

    /** Adds to the score. */
    public void add(long amount) {
        value += amount;
    }

    /** Overwrites the score, used to restore a stored high score. */
    public void set(long amount) {
        value = amount;
    }

    /** Returns the score. */
    public long get() {
        return value;
    }
}
