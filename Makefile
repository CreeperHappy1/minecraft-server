SHELL = /bin/sh
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m755
INSTALL_DATA = $(INSTALL) -m644
confdir = /etc/conf.d
prefix = /usr
bindir = $(prefix)/bin
libdir = $(prefix)/lib
datarootdir = $(prefix)/share
mandir = $(prefix)/share/man
man1dir = $(mandir)/man1

BUILD_DIR = build
WEB_WAKE_DIR = web-wake
CXX ?= g++
CXXFLAGS ?= -O2 -Wall

SOURCES = minecraftd.sh.in minecraftd.conf.in minecraftd.service.in minecraftd.sysusers.in minecraftd.tmpfiles.in minecraftd-backup.service.in minecraftd-backup.timer.in
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.in=))

GAME = minecraft
INAME = minecraftd
SERVER_ROOT = /srv/$(GAME)
BACKUP_DEST = $(SERVER_ROOT)/backup
BACKUP_PATHS = world
BACKUP_FLAGS = -z
KEEP_BACKUPS = 10
GAME_USER = $(GAME)
MAIN_EXECUTABLE = minecraft_server.jar
SESSION_NAME = $(GAME)
SERVER_START_CMD = java -Xms512M -Xmx1024M -jar ./$${MAIN_EXECUTABLE} nogui
SERVER_START_SUCCESS = done
IDLE_SERVER = false
IDLE_SESSION_NAME = idle_server_$${SESSION_NAME}
GAME_PORT = 25565
CHECK_PLAYER_TIME = 30
IDLE_IF_TIME = 1200
IDLE_WAKE_MODE = any
IDLE_WAKE_WHITELIST = $(SERVER_ROOT)/whitelist.json
GAME_COMMAND_DUMP = /tmp/$${INAME}_$${SESSION_NAME}_command_dump.txt
MAX_SERVER_START_TIME = 150
MAX_SERVER_STOP_TIME = 100
IDLE_WAKE_WHITELIST_CMD = jq -r '.[].name'
WEBWAKE_SESSION_NAME = webwake_server_$${SESSION_NAME}

WEB_WAKE_IN = $(shell find $(WEB_WAKE_DIR) -type f)
WEB_WAKE_BUILD_SRCS = $(patsubst $(WEB_WAKE_DIR)/%, $(BUILD_DIR)/$(WEB_WAKE_DIR)/%, $(WEB_WAKE_IN))
WEB_WAKE_BIN = $(BUILD_DIR)/web-wake-server

webdir = $(datarootdir)/$(GAME)/web

.MAIN = all

define replace_all
	cp -a $(1) $(2)
	sed -i \
		-e 's#@INAME@#$(INAME)#g' \
		-e 's#@GAME@#$(GAME)#g' \
		-e 's#@SERVER_ROOT@#$(SERVER_ROOT)#g' \
		-e 's#@BACKUP_DEST@#$(BACKUP_DEST)#g' \
		-e 's#@BACKUP_PATHS@#$(BACKUP_PATHS)#g' \
		-e 's#@BACKUP_FLAGS@#$(BACKUP_FLAGS)#g' \
		-e 's#@KEEP_BACKUPS@#$(KEEP_BACKUPS)#g' \
		-e 's#@GAME_USER@#$(GAME_USER)#g' \
		-e 's#@MAIN_EXECUTABLE@#$(MAIN_EXECUTABLE)#g' \
		-e 's#@SESSION_NAME@#$(SESSION_NAME)#g' \
		-e 's#@SERVER_START_CMD@#$(SERVER_START_CMD)#g' \
		-e 's#@SERVER_START_SUCCESS@#$(SERVER_START_SUCCESS)#g' \
		-e 's#@IDLE_SERVER@#$(IDLE_SERVER)#g' \
		-e 's#@IDLE_SESSION_NAME@#$(IDLE_SESSION_NAME)#g' \
		-e 's#@GAME_PORT@#$(GAME_PORT)#g' \
		-e 's#@CHECK_PLAYER_TIME@#$(CHECK_PLAYER_TIME)#g' \
		-e 's#@IDLE_IF_TIME@#$(IDLE_IF_TIME)#g' \
		-e 's#@IDLE_WAKE_MODE@#$(IDLE_WAKE_MODE)#g' \
		-e 's#@IDLE_WAKE_WHITELIST@#$(IDLE_WAKE_WHITELIST)#g' \
		-e 's#@GAME_COMMAND_DUMP@#$(GAME_COMMAND_DUMP)#g' \
		-e 's#@MAX_SERVER_START_TIME@#$(MAX_SERVER_START_TIME)#g' \
		-e 's#@MAX_SERVER_STOP_TIME@#$(MAX_SERVER_STOP_TIME)#g' \
		-e 's#@IDLE_WAKE_WHITELIST_CMD@#$(IDLE_WAKE_WHITELIST_CMD)#g' \
		-e 's#@WEBWAKE_SESSION_NAME@#$(WEBWAKE_SESSION_NAME)#g' \
		$(2)
endef

all: $(OBJECTS) $(WEB_WAKE_BIN)
	@echo $^

$(BUILD_DIR) $(BUILD_DIR)/$(WEB_WAKE_DIR):
	mkdir -p $@

$(BUILD_DIR)/%: %.in | $(BUILD_DIR)
	$(call replace_all,$<,$@)

$(BUILD_DIR)/$(WEB_WAKE_DIR)/%: $(WEB_WAKE_DIR)/% | $(BUILD_DIR)/$(WEB_WAKE_DIR)
	@mkdir -p $(dir $@)
	$(call replace_all,$<,$@)

$(WEB_WAKE_BIN): $(WEB_WAKE_BUILD_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) \
	-I$(BUILD_DIR)/$(WEB_WAKE_DIR)/include -I$(BUILD_DIR)/$(WEB_WAKE_DIR)/src \
	-DWEB_DIR=\"$(webdir)/\" -DCONFIG_PATH=\"$(confdir)/$(GAME)-web-wake.json\" -DTOKEN_PATH=\"$(SERVER_ROOT)/.web-wake-token\" \
	$(BUILD_DIR)/$(WEB_WAKE_DIR)/src/*.cpp -lssl -lcrypto -o $@

clean:
	rm -rf $(BUILD_DIR)

distclean: clean

maintainer-clean: clean

install:
	$(INSTALL_PROGRAM) -D $(BUILD_DIR)/minecraftd.sh "$(DESTDIR)$(bindir)/$(INAME)"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd.conf           "$(DESTDIR)$(confdir)/$(GAME)"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd.service        "$(DESTDIR)$(libdir)/systemd/system/$(INAME).service"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd-backup.service "$(DESTDIR)$(libdir)/systemd/system/$(INAME)-backup.service"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd-backup.timer   "$(DESTDIR)$(libdir)/systemd/system/$(INAME)-backup.timer"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd.sysusers       "$(DESTDIR)$(libdir)/sysusers.d/$(INAME).conf"
	$(INSTALL_DATA) -D $(BUILD_DIR)/minecraftd.tmpfiles       "$(DESTDIR)$(libdir)/tmpfiles.d/$(INAME).conf"

	$(INSTALL_PROGRAM) -D $(WEB_WAKE_BIN)	"$(DESTDIR)$(bindir)/$(INAME)-web-wake"
	$(INSTALL_DATA) -D $(BUILD_DIR)/$(WEB_WAKE_DIR)/config.json	"$(DESTDIR)$(confdir)/$(GAME)-web-wake.json"
	$(INSTALL) -d "$(DESTDIR)$(webdir)"
	$(INSTALL_DATA) $(BUILD_DIR)/$(WEB_WAKE_DIR)/web/*	"$(DESTDIR)$(webdir)/"

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(INAME)"
	rm -f "$(DESTDIR)$(confdir)/$(GAME)"
	rm -f "$(DESTDIR)$(libdir)/systemd/system/$(INAME).service"
	rm -f "$(DESTDIR)$(libdir)/systemd/system/$(INAME)-backup.service"
	rm -f "$(DESTDIR)$(libdir)/systemd/system/$(INAME)-backup.timer"
	rm -f "$(DESTDIR)$(libdir)/sysusers.d/$(INAME).conf"
	rm -f "$(DESTDIR)$(libdir)/tmpfiles.d/$(INAME).conf"
	rm -f "$(DESTDIR)$(bindir)/$(INAME)-web-wake"
	rm -f "$(DESTDIR)$(confdir)/$(GAME)-web-wake.json"
	rm -rf "$(DESTDIR)$(webdir)"
