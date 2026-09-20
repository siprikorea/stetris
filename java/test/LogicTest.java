import stetris.Block;
import stetris.Board;
import stetris.Play;

/**
 * Tests for the logic, with no front end involved.
 *
 * <p>The reference table at the bottom is shared with the C++ and Python
 * ports. If one of them drifts, the three have stopped agreeing on the rules.
 */
public final class LogicTest {
    private static int failures;

    private static void check(String name, boolean pass, String detail) {
        System.out.printf("  %-14s %-28s %s%n", name, detail, pass ? "ok" : "FAIL");
        if (!pass) {
            failures++;
        }
    }

    /** Plays a whole game out with hard drops only. */
    private static long playOut(int seed) {
        Play play = new Play();
        play.newGame(seed);

        int guard = 0;
        while (!play.isGameOver() && guard++ < 100000) {
            play.hardDrop();
        }
        return play.getScore().get();
    }

    private static void testDeterminism() {
        long first = playOut(12345);
        long second = playOut(12345);
        check("determinism", first == second, "seed 12345 -> " + first + ", " + second);

        long other = playOut(999);
        check("variation", other != first, "seed 999 -> " + other);
    }

    private static void testIdleScore() {
        Play play = new Play();
        play.newGame(7);

        // Ten seconds of falling without touching a key
        for (int step = 0; step < 100; step++) {
            play.tick(100);
        }

        check("idle score", play.getScore().get() == 0, "10s idle -> " + play.getScore().get());
    }

    private static void testPause() {
        Play play = new Play();
        play.newGame(7);
        play.setPause(true);

        int yPos = play.getCurrentBlock().getYPos();
        for (int step = 0; step < 50; step++) {
            play.tick(100);
        }

        boolean frozen = play.getCurrentBlock().getYPos() == yPos;
        boolean blocked = !play.moveLeft() && !play.rotate() && !play.hardDrop();
        check("pause", frozen && blocked, "frozen " + frozen + ", input blocked " + blocked);

        play.setPause(false);
        check("resume", play.isPlaying() && play.moveLeft(), "moves again");
    }

    private static void testGameOver() {
        Play play = new Play();
        play.newGame(42);

        int drops = 0;
        while (!play.isGameOver() && drops < 100000) {
            play.hardDrop();
            drops++;
        }

        check("game over", play.isGameOver(), "ended after " + drops + " drops");
        check("stays over", !play.hardDrop() && !play.rotate() && !play.moveLeft(), "input ignored");

        play.setPause(false);
        check("no revive", play.isGameOver(), "unpause does nothing");
    }

    private static void testLineClear() {
        Play play = new Play();

        // Look for a seed that starts with the square block, so the shape
        // that lands in the gap left below is known
        int seed = 1;
        while (seed < 1000 && play.getCurrentBlock().getType() != 3) {
            play.newGame(seed);
            seed++;
        }

        if (play.getCurrentBlock().getType() != 3) {
            check("line clear", false, "no square block seed found");
            return;
        }

        Board board = play.getBoard();
        int xSize = board.getXSize();
        int ySize = board.getYSize();

        // Fill the bottom two rows except the two leftmost columns
        for (int y = ySize - 2; y < ySize; y++) {
            for (int x = 2; x < xSize; x++) {
                board.setValue(x, y, 1);
            }
        }

        long before = play.getScore().get();

        while (play.moveLeft()) {
            // slide the square into the gap
        }
        play.hardDrop();

        long gained = play.getScore().get() - before;
        String detail = play.getLines() + " lines, +" + gained;

        check("line clear", play.getLines() == 2, detail);

        // Two lines at level 1 is 300, plus 2 per cell of the hard drop
        check("line score", gained >= 300, detail);

        boolean empty = true;
        for (int x = 0; x < xSize; x++) {
            if (board.getValue(x, ySize - 1) != 0) {
                empty = false;
            }
        }
        check("rows removed", empty, "bottom row cleared");
    }

    private static void testSpeed() {
        Play play = new Play();
        play.newGame(7);
        check("fall speed", play.getLevel() == 1 && play.getFallInterval() == 800,
                "level " + play.getLevel() + " -> " + play.getFallInterval() + " ms");
    }

    private static void testRestart() {
        Play play = new Play();
        play.newGame(3);

        int guard = 0;
        while (!play.isGameOver() && guard++ < 100000) {
            play.hardDrop();
        }

        long high = play.getHighScore().get();
        play.newGame(4);

        boolean reset = play.getScore().get() == 0 && play.getLines() == 0 && play.isPlaying();
        boolean kept = play.getHighScore().get() == high && high > 0;

        boolean empty = true;
        Board board = play.getBoard();
        for (int y = 0; y < board.getYSize(); y++) {
            for (int x = 0; x < board.getXSize(); x++) {
                if (board.getValue(x, y) != 0) {
                    empty = false;
                }
            }
        }

        check("restart", reset && empty, "high " + high + " kept");
        check("high score", kept, "high " + high + " kept");
    }

    private static void testWallKick() {
        Play play = new Play();

        // The bar is the shape that needs the kick the most
        int seed = 1;
        while (seed < 1000 && play.getCurrentBlock().getType() != 7) {
            play.newGame(seed);
            seed++;
        }

        if (play.getCurrentBlock().getType() != 7) {
            check("wall kick", false, "no bar block seed found");
            return;
        }

        // Stand the bar up, then push it against the right wall and turn it flat
        play.rotate();
        while (play.moveRight()) {
            // push to the wall
        }

        check("wall kick", play.rotate(), "bar rotates at the wall");
    }

    /** Scenario A, a fixed move script from an empty board. */
    private static void scenarioA(Play play) {
        for (int drop = 0; !play.isGameOver() && drop < 100000; drop++) {
            for (int n = 0; n < drop % 4; n++) {
                play.rotate();
            }

            int shift = drop % 11;
            if (shift < 5) {
                for (int n = 0; n < 5 - shift; n++) {
                    play.moveLeft();
                }
            } else {
                for (int n = 0; n < shift - 5; n++) {
                    play.moveRight();
                }
            }

            if (drop % 7 == 0) {
                play.softDrop();
            }

            play.hardDrop();
        }
    }

    /** Scenario B, dropping into a gap so lines clear. */
    private static void scenarioB(Play play) {
        // Leave a four wide gap at the right, so any shape pushed against
        // the wall drops into it and the rows fill up
        Board board = play.getBoard();
        for (int y = 14; y < board.getYSize(); y++) {
            for (int x = 0; x < board.getXSize() - 4; x++) {
                board.setValue(x, y, 1);
            }
        }

        for (int drop = 0; !play.isGameOver() && drop < 100000; drop++) {
            while (play.moveRight()) {
                // push to the wall
            }
            play.hardDrop();
        }
    }

    // scenario, seed, score, lines, level
    private static final long[][] REFERENCE = {
        { 'A',     1, 176, 0, 1 },
        { 'A',     7, 263, 0, 1 },
        { 'A',    42, 267, 0, 1 },
        { 'A',   999, 174, 0, 1 },
        { 'A', 12345, 323, 0, 1 },
        { 'A',  2024, 353, 0, 1 },
        { 'A', 65535, 156, 0, 1 },
        { 'B',     1, 402, 1, 1 },
        { 'B',     7, 200, 0, 1 },
        { 'B',    42, 186, 0, 1 },
        { 'B',   999, 324, 1, 1 },
        { 'B', 12345, 362, 1, 1 },
        { 'B',  2024, 212, 0, 1 },
        { 'B', 65535, 198, 0, 1 },
    };

    private static void testReference() {
        int mismatch = 0;

        for (long[] row : REFERENCE) {
            char scenario = (char) row[0];
            int seed = (int) row[1];

            Play play = new Play();
            play.newGame(seed);

            if (scenario == 'A') {
                scenarioA(play);
            } else {
                scenarioB(play);
            }

            if (play.getScore().get() != row[2] || play.getLines() != row[3]
                    || play.getLevel() != row[4]) {
                System.out.printf("    %c seed %d: got %d/%d/%d, want %d/%d/%d%n",
                        scenario, seed, play.getScore().get(), play.getLines(),
                        play.getLevel(), row[2], row[3], row[4]);
                mismatch++;
            }
        }

        check("reference", mismatch == 0, REFERENCE.length + " cases");
    }

    public static void main(String[] args) {
        System.out.println("stetris logic tests (java)\n");

        testDeterminism();
        testIdleScore();
        testPause();
        testGameOver();
        testLineClear();
        testSpeed();
        testRestart();
        testWallKick();
        testReference();

        System.out.println("\n" + (failures != 0 ? "FAILED" : "all passed"));
        System.exit(failures != 0 ? 1 : 0);
    }
}
