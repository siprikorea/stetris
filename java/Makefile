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

$(OUT)/.built: $(SOURCES) test/LogicTest.java
	$(JAVAC) -Xlint:all -d $(OUT) $(SOURCES) test/LogicTest.java
	@touch $@

clean:
	rm -rf $(OUT)

.PHONY: all test clean
