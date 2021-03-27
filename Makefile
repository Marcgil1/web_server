DIRO = build/obj/
DIRS = src/
DIRB = build/
DIRR = resources/
DIRU = lib/unity/src/

.PHONY: build
.PHONY: build_test
.PHONY: test
.PHONY: start
.PHONY: stop
.PHONY: log
.PHONY: clean

$(DIRO)http.o:: $(DIRS)http/http.c
	gcc -I $(DIRS) -c -o $(DIRO)http.o $(DIRS)http/http.c

$(DIRO)http.test.o:: $(DIRS)http/http.test.c $(DIRS)http/http.c
	gcc -I $(DIRS) -I $(DIRU) -c -o $(DIRO)http.test.o $(DIRS)http/http.test.c

$(DIRO)unity.o:: $(DIRU)unity.c
	gcc -I $(DIRS) -I $(DIRU) -c -o $(DIRO)unity.o $(DIRU)unity.c

$(DIRO)main.o:: $(DIRS)main.c
	gcc -I $(DIRS) -c -o $(DIRO)main.o $(DIRS)main.c

$(DIRB)main.out: $(DIRO)main.o $(DIRO)http.o
	gcc -o $(DIRB)main.out $(DIRO)main.o $(DIRO)http.o



build: $(DIRB)main.out
	@echo "Building the project..."

build_test: $(DIRO)http.test.o $(DIRO)unity.o
	gcc -o $(DIRB)http.test.out $(DIRO)http.test.o $(DIRO)unity.o

test: build_test
	@echo "Running tests for http..."
	$(DIRB)http.test.out

start: $(DIRB)main.out
	@echo "Running the project..."
	$(DIRB)main.out 8080 resources/web

stop:
	@echo "Stopping the program..."
	killall main.out

clean_log:
	echo "" > resources/web/webserver.log

log:
	cat resources/web/webserver.log

clean:
	@echo "Cleaning the build..."
	if [ -f $(DIRB)main.out ];      then rm $(DIRB)main.out; fi
	if [ -f $(DIRB)http.test.out ]; then rm $(DIRB)http.test.out; fi
	rm $(DIRO)*.o



$(DIRO)%.o:: $(DIRS)%.c
	gcc -I $(DIRS) -c $< -o $@


