############### C++ compiler flags ###################
CXX := clang++
CXX_FLAGS = -std=c++2a -stdlib=libc++ -fmodules-ts -fmodules -fbuiltin-module-map -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=.

############### External C++ libraries  ###################
LIB_NLOHMANN := -I/opt/homebrew/Cellar/nlohmann-json/3.10.5

LIB_OPENSSL := /opt/homebrew/Cellar/openssl@3/3.0.1

################## Project dirs ##################
PROJ_DIR := $(CURDIR)
BUILD_DIR := $(PROJ_DIR)/build
MODULES_DIR := $(BUILD_DIR)/modules
SOURCES_DIR := $(PROJ_DIR)/modules
TARGETS_DIR := $(PROJ_DIR)/targets
OBJECTS_DIR := $(BUILD_DIR)/objects
APPS_DIR := $(BUILD_DIR)/apps

#################### Targets ###############
SERVER := $(BUILD_DIR)/server
CLIENT := $(BUILD_DIR)/client

######################################
SOURCES := $(wildcard $(SOURCES_DIR)/*.cpp)
TARGETS := $(wildcard $(TARGETS_DIR)/*.cpp)

__MODULES := $(subst .cpp,.pcm,$(SOURCES))
_MODULES := $(foreach F,$(__MODULES),$(word $(words $(subst /, ,$F)),$(subst /, ,$F)))
MODULES := $(foreach name, $(_MODULES), $(addprefix $(MODULES_DIR)/, $(name)))

# __APPS := $(subst .cpp,,$(TARGETS))
__APPS = $(basename $(TARGETS))
_APPS := $(foreach F,$(__APPS),$(word $(words $(subst /, ,$F)),$(subst /, ,$F)))
APPS := $(foreach name, $(_APPS), $(addprefix $(APPS_DIR)/, $(name)))

__OBJECTS := $(subst .cpp,.o,$(TARGETS))
_OBJECTS := $(foreach F,$(__OBJECTS),$(word $(words $(subst /, ,$F)),$(subst /, ,$F)))
OBJECTS := $(foreach name, $(_OBJECTS), $(addprefix $(OBJECTS_DIR)/, $(name)))

_BUILD_DIRS := apps modules targets objects docs tests
BUILD_DIRS := $(foreach dir, $(_BUILD_DIRS), $(addprefix $(BUILD_DIR)/, $(dir)))

######################################
all: $(APPS)

######## Client ###########
$(APPS_DIR)/client: $(OBJECTS_DIR)/client.o 
	$(CXX) $(CXX_FLAGS) $(OBJECTS_DIR)/client.o -o $@ $(LIB_OPENSSL)/lib/libssl.a $(LIB_OPENSSL)/lib/libcrypto.a

$(OBJECTS_DIR)/client.o: $(TARGETS_DIR)/client.cpp $(MODULES)
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -o $@ $(LIB_NLOHMANN)/include

######## Server ###########
$(APPS_DIR)/server: $(OBJECTS_DIR)/server.o
	$(CXX) $(CXX_FLAGS) $(OBJECTS_DIR)/server.o -o $@

$(OBJECTS_DIR)/server.o: $(TARGETS_DIR)/server.cpp $(MODULES)
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -o $@ $(LIB_NLOHMANN)/include

# $(info $$NAMES is [${NAMES}])


######## Modules ###########
$(MODULES_DIR)/Server.pcm: $(SOURCES_DIR)/Server.cpp $(MODULES_DIR)/RemoteClient.pcm $(MODULES_DIR)/Http.pcm $(MODULES_DIR)/Common.pcm
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ $(LIB_NLOHMANN)/include

$(MODULES_DIR)/RemoteClient.pcm: $(SOURCES_DIR)/RemoteClient.cpp
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ $(LIB_NLOHMANN)/include

$(MODULES_DIR)/Email.pcm: $(SOURCES_DIR)/Email.cpp $(MODULES_DIR)/RemoteServer.pcm
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ $(LIB_NLOHMANN)/include -I/$(LIB_OPENSSL)/include

$(MODULES_DIR)/RemoteServer.pcm: $(SOURCES_DIR)/RemoteServer.cpp
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ $(LIB_NLOHMANN)/include

$(MODULES_DIR)/Usr.pcm: $(SOURCES_DIR)/Usr.cpp $(MODULES_DIR)/Http.pcm $(MODULES_DIR)/Common.pcm
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ $(LIB_NLOHMANN)/include

$(MODULES_DIR)/Http.pcm: $(SOURCES_DIR)/Http.cpp $(MODULES_DIR)/Common.pcm 
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ 

$(MODULES_DIR)/Common.pcm: $(SOURCES_DIR)/Common.cpp
	$(CXX) $(CXX_FLAGS) $(addprefix -fmodule-file=, $(filter-out $<, $^)) -c $< -Xclang -emit-module-interface -o $@ 



######################################
directories := $(foreach dir, $(BUILD_DIRS), $(shell [ -d $(dir) ] || mkdir -p $(dir)))

clean:
	rm -rf $(BUILD_DIR)/*