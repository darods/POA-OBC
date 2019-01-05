
# .SUFFIXES:.c.o .cc.o

TARGET := MaverickDrone_HostApp

# Get Root Directory
ROOT_DIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
IMGPROC_ROOT_DIR := ./lib/imgproc

LIBYUV_ROOT_DIR := ./lib/libyuv

# Set Source List
SOURCE = $(ROOT_DIR)/MaverickDrone_Host/MaverickDrone_Host/main.c
include $(ROOT_DIR)/lib/imgproc/imgproc.mk
include $(ROOT_DIR)/lib/libyuv/libyuv.mk

LDFLAGS = -framework Carbon \
			-framework OpenGL \
			-framework GLUT \
			-framework CoreVideo \
			-framework AppKit \
			-framework AVFoundation \
			-framework CoreMedia \
			-framework QTKit \
			-framework QuartzCore \
			-framework AppKit \
			-lopencv_core -lopencv_imgproc -lopencv_highgui -llibjpeg -llibpng -llibjasper -lzlib -llibtiff -lIlmImf

# Set Object List
OBJECT = $(SOURCE:.c=.o)
# $(SOURCE:.cc=.o)

# Set Header Path
HEADER_PATH := -I$(ROOT_DIR)                                       \
					-I$(ROOT_DIR)/lib/imgproc/include              \
					-I$(ROOT_DIR)/lib/libyuv/include               \
					-I$(ROOT_DIR)/lib/libyuv/jpeg                  \
					-I$(ROOT_DIR)/lib/opencv/                      \
					-I$(ROOT_DIR)/lib/opencv/opencv                \
					-I$(ROOT_DIR)/lib/opencv/opencv2/calib3d       \
					-I$(ROOT_DIR)/lib/opencv/opencv2/core          \
					-I$(ROOT_DIR)/lib/opencv/opencv2/features2d    \
					-I$(ROOT_DIR)/lib/opencv/opencv2/flann         \
					-I$(ROOT_DIR)/lib/opencv/opencv2/highgui       \
					-I$(ROOT_DIR)/lib/opencv/opencv2/imgproc       \
					-I$(ROOT_DIR)/lib/opencv/opencv2/legacy        \
					-I$(ROOT_DIR)/lib/opencv/opencv2/objdetect     \
					-I$(ROOT_DIR)/lib/opencv/opencv2/video

# Set Library Path
LIBRARY_PATH := -L$(ROOT_DIR)/lib/opencv/opencv

# Set Compile Option
CFLAGS += $(HEADER_PATH)                                           \
					-O2                                            \
					-g                                             \
					-D__ARM__                                      \
					-D__ANDROID__

CXXFLAGS += $(HEADER_PATH)                                         \
					-O2                                            \
					-g                                             \
					-D__ARM__                                      \
					-D__ANDROID__

# Target
$(TARGET): $(OBJECT)
	# $(CC) $(LIBRARY_PATH) $(CFLAGS) -o $@ $(OBJECT) $(LDFLAGS)
	$(CXX) $(LIBRARY_PATH) $(CXXFLAGS) -o $@ $(OBJECT) $(LDFLAGS)





# all: $(TARGET)

# .PHONY: clean
# clean:
#     rm -rf $(OBJECTS) $(TARGET)