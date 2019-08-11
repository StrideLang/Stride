#set(INCLUDE_DIRS rtaudio-4.1.2)
#set(ADDITIONAL_SOURCES rtaudio-4.1.2/RtAudio.cpp)
#set(ADDITIONAL_DEFINES -D__LINUX_PULSE__)
#set(LINK_LIBRARIES pthread pulse-simple pulse)

set(INCLUDE_DIRS %%include_dirs%%)
set(ADDITIONAL_SOURCES %%additional_sources%%)
set(ADDITIONAL_DEFINES %%additional_defines%%)
set(LINK_LIBRARIES %%link_libraries%%)