NODE ?= node

test:
	$(NODE) test/logictest.js

console:
	$(NODE) app/console/main.js

web:
	$(NODE) app/web/serve.js

dist:
	$(NODE) app/web/build.js

.PHONY: test console web dist
