DIRO = build/obj/
DIRS = src/
DIRB = build/
DIRR = resources/
DIRU = lib/unity/src/

COMPILE		 = gcc -I $(DIRS) -c
COMPILE_TEST = gcc -I $(DIRS) -I $(DIRU) -c
LINK         = gcc 

.PHONY: build
.PHONY: build_test
.PHONY: test
.PHONY: start
.PHONY: stop
.PHONY: log
.PHONY: clean

$(DIRO)debug.o:: $(DIRS)dg/debug.c
	$(COMPILE)      -o $(DIRO)debug.o $(DIRS)dg/debug.c

$(DIRO)cookie.o:: $(DIRS)http/cookie.c
	$(COMPILE)      -o $(DIRO)cookie.o $(DIRS)http/cookie.c

$(DIRO)cookie.test.o:: $(DIRS)http/cookie.test.c $(DIRS)http/cookie.c
	$(COMPILE_TEST) -o $(DIRO)cookie.test.o $(DIRS)http/cookie.test.c

$(DIRO)http.o:: $(DIRS)http/http.c
	$(COMPILE)      -o $(DIRO)http.o $(DIRS)http/http.c

$(DIRO)http.test.o:: $(DIRS)http/http.test.c $(DIRS)http/http.c
	$(COMPILE_TEST) -o $(DIRO)http.test.o $(DIRS)http/http.test.c

$(DIRO)unity.o:: $(DIRU)unity.c
	$(COMPILE_TEST) -o $(DIRO)unity.o $(DIRU)unity.c

$(DIRO)main.o:: $(DIRS)main.c
	$(COMPILE)      -o $(DIRO)main.o $(DIRS)main.c

$(DIRB)main.out: $(DIRO)main.o $(DIRO)http.o $(DIRO)cookie.o $(DIRO)debug.o
	$(LINK)         -o $(DIRB)main.out $(DIRO)main.o $(DIRO)http.o $(DIRO)cookie.o $(DIRO)debug.o -lm 



build: $(DIRB)main.out
	@echo "-------------------------------------------------------------------"
	@echo "-------- BUILD ----------------------------------------------------"
	@echo "-------------------------------------------------------------------"

build_test: $(DIRO)http.test.o $(DIRO)cookie.test.o $(DIRO)unity.o $(DIRO)cookie.o
	$(LINK) -o $(DIRB)http.test.out   $(DIRO)http.test.o   $(DIRO)unity.o $(DIRO)cookie.o -lm
	$(LINK) -o $(DIRB)cookie.test.out $(DIRO)cookie.test.o $(DIRO)unity.o -lm

test: build_test
	@echo "-------------------------------------------------------------------"
	@echo "-------- TEST: http/http ------------------------------------------"
	@echo "-------------------------------------------------------------------"
	$(DIRB)http.test.out
	@echo "-------------------------------------------------------------------"
	@echo "-------- TEST: http/cookie ----------------------------------------"
	@echo "-------------------------------------------------------------------"
	$(DIRB)cookie.test.out

start: $(DIRB)main.out
	@echo "-------------------------------------------------------------------"
	@echo "-------- START ----------------------------------------------------"
	@echo "-------------------------------------------------------------------"
	$(DIRB)main.out 8080 resources/web

stop:
	@echo "-------------------------------------------------------------------"
	@echo "-------- STOP -----------------------------------------------------"
	@echo "-------------------------------------------------------------------"
	killall main.out

clean_log:
	@echo "-------------------------------------------------------------------"
	@echo "--------- CLEAN_LOG -----------------------------------------------"
	@echo "-------------------------------------------------------------------"
	echo "" > resources/log/webserver.log

log:
	@echo "-------------------------------------------------------------------"
	@echo "--------- LOG -----------------------------------------------------"
	@echo "-------------------------------------------------------------------"
	cat resources/log/webserver.log

clean:
	@echo "-------------------------------------------------------------------"
	@echo "--------- CLEAN_BUID ----------------------------------------------"
	@echo "-------------------------------------------------------------------"
	if [ -f $(DIRB)main.out ];        then rm $(DIRB)main.out; fi
	if [ -f $(DIRB)http.test.out ];   then rm $(DIRB)http.test.out; fi
	if [ -f $(DIRB)cookie.test.out ]; then rm $(DIRB)cookie.test.out; fi
	rm $(DIRO)*.o



$(DIRO)%.o:: $(DIRS)%.c
	gcc -I $(DIRS) -c $< -o $@


