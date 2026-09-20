package app.console;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

import stetris.Play;

/**
 * Terminal front end.
 *
 * <p>Everything the game does not do lives here: reading keys, measuring
 * elapsed time to hand to {@code tick()}, seeding a new game and storing
 * the high score in a file.
 */
public final class Main {
    // Keys
    private static final int KEY_NONE = 0;
    private static final int KEY_LEFT = 1;
    private static final int KEY_RIGHT = 2;
    private static final int KEY_DOWN = 3;
    private static final int KEY_UP = 4;
    private static final int KEY_DROP = 5;
    private static final int KEY_PAUSE = 6;
    private static final int KEY_RESTART = 7;
    private static final int KEY_QUIT = 8;

    /** Input poll interval in milliseconds. */
    private static final int POLL_INTERVAL = 20;

    private static boolean rawMode;

    /**
     * Puts the terminal in raw mode.
     *
     * <p>Java has no way to do this itself, so it asks stty, which means
     * this part is Unix only. Everything above it is not.
     */
    private static void enterRawMode() {
        if (System.console() == null) {
            // Input is piped, there is no terminal to reconfigure
            return;
        }
        rawMode = stty("-icanon min 1 -echo");
    }

    private static void leaveRawMode() {
        if (rawMode) {
            stty("icanon echo");
            rawMode = false;
        }
    }

    private static boolean stty(String args) {
        try {
            return new ProcessBuilder("/bin/sh", "-c", "stty " + args + " < /dev/tty")
                    .inheritIO()
                    .start()
                    .waitFor() == 0;
        } catch (IOException | InterruptedException e) {
            return false;
        }
    }

    /** Returns a pending key, or {@code KEY_NONE} when there is none. */
    private static int readKey(InputStream in) throws IOException {
        if (in.available() <= 0) {
            return KEY_NONE;
        }

        int ch = in.read();
        if (ch < 0) {
            return KEY_QUIT;
        }

        // Arrow keys arrive as "ESC [ A" ~ "ESC [ D"
        if (ch == 27) {
            if (in.available() < 2 || in.read() != '[') {
                return KEY_NONE;
            }
            return switch (in.read()) {
                case 'A' -> KEY_UP;
                case 'B' -> KEY_DOWN;
                case 'C' -> KEY_RIGHT;
                case 'D' -> KEY_LEFT;
                default -> KEY_NONE;
            };
        }

        return switch (ch) {
            case ' ' -> KEY_DROP;
            case 'p', 'P' -> KEY_PAUSE;
            case 'r', 'R' -> KEY_RESTART;
            case 'q', 'Q' -> KEY_QUIT;
            // WASD as an alternative to the arrow keys
            case 'a', 'A' -> KEY_LEFT;
            case 'd', 'D' -> KEY_RIGHT;
            case 's', 'S' -> KEY_DOWN;
            case 'w', 'W' -> KEY_UP;
            default -> KEY_NONE;
        };
    }

    private static Path highScorePath() {
        // HOME first, so the four apps agree on one file; user.home is the
        // fallback for platforms that do not set it
        String home = System.getenv("HOME");
        if (home == null || home.isEmpty()) {
            home = System.getProperty("user.home");
        }
        return Path.of(home != null ? home : ".", ".stetris_highscore");
    }

    private static long loadHighScore() {
        try {
            return Long.parseLong(Files.readString(highScorePath()).trim());
        } catch (IOException | NumberFormatException e) {
            return 0;
        }
    }

    private static void saveHighScore(long value) {
        try {
            Files.writeString(highScorePath(), value + "\n", StandardCharsets.UTF_8);
        } catch (IOException e) {
            // Not being able to keep the high score is not worth failing over
        }
    }

    public static void main(String[] args) throws Exception {
        Play play = new Play();
        play.newGame((int) (System.currentTimeMillis() / 1000));

        // The high score is stored by the platform, the game only holds it
        play.getHighScore().set(loadHighScore());

        ConsoleView view = new ConsoleView();
        InputStream in = System.in;

        enterRawMode();
        try {
            view.enterScreen();

            boolean quit = false;
            boolean saved = false;
            long lastTick = System.currentTimeMillis();
            view.render(play);

            while (!quit) {
                switch (readKey(in)) {
                    case KEY_QUIT -> quit = true;
                    case KEY_RESTART -> {
                        play.newGame((int) (System.currentTimeMillis() / 1000));
                        saved = false;
                        lastTick = System.currentTimeMillis();
                    }
                    case KEY_PAUSE -> play.togglePause();
                    case KEY_LEFT -> play.moveLeft();
                    case KEY_RIGHT -> play.moveRight();
                    case KEY_UP -> play.rotate();
                    case KEY_DOWN -> play.softDrop();
                    case KEY_DROP -> play.hardDrop();
                    default -> { }
                }

                // Hand the elapsed time to the game, it decides when to fall
                long now = System.currentTimeMillis();
                play.tick((int) (now - lastTick));
                lastTick = now;

                // Persist the high score as soon as the game ends
                if (play.isGameOver() && !saved) {
                    saveHighScore(play.getHighScore().get());
                    saved = true;
                }

                view.render(play);
                Thread.sleep(POLL_INTERVAL);
            }

            saveHighScore(play.getHighScore().get());
            view.leaveScreen();
        } finally {
            leaveRawMode();
        }

        System.out.printf("SCORE %d   HIGH SCORE %d   LEVEL %d   LINES %d%n",
                play.getScore().get(), play.getHighScore().get(),
                play.getLevel(), play.getLines());
    }
}
