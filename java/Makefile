JAVAC ?= javac
JAVA  ?= java
OUT    = out

SOURCES = stetris/Blocks.java \
          stetris/Block.java \
          stetris/Board.java \
          stetris/Play.java \
          stetris/Rng.java \
          stetris/Score.java \
          stetris/State.java

all: $(OUT)/.built

test: $(OUT)/.built
	$(JAVA) -cp $(OUT) LogicTest

console: $(OUT)/.built
	$(JAVA) -cp $(OUT) app.console.Main

APP = app/console/ConsoleView.java \
      app/console/Main.java

$(OUT)/.built: $(SOURCES) $(APP) test/LogicTest.java
	$(JAVAC) -Xlint:all -d $(OUT) $(SOURCES) $(APP) test/LogicTest.java
	@touch $@

clean:
	rm -rf $(OUT)

.PHONY: all test console clean
