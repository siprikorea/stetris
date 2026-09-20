package stetris;

/**
 * The random generator the game runs on.
 *
 * <p>A linear congruential generator rather than {@link java.util.Random},
 * so that a seed produces the same sequence in every port of this logic.
 */
public final class Rng {
    private int state = 1;

    /** Seeds the generator. A zero seed would make it stick at zero. */
    public void seed(int seed) {
        state = seed != 0 ? seed : 1;
    }

    /** Returns the next value. */
    public int next() {
        state = state * 1664525 + 1013904223;

        // The low bits of an LCG are weak, so use the high ones
        return state >>> 16;
    }

    /** Returns the next value in {@code [min, max]}. */
    public int nextRange(int min, int max) {
        if (max <= min) {
            return min;
        }
        return min + next() % (max - min + 1);
    }
}
