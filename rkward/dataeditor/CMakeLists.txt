INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(dataeditor_STAT_SRCS
   twintable.cpp
   twintablemember.cpp
   rkeditor.cpp
   rkeditordataframe.cpp
   rkvareditmodel.cpp
   rktextmatrix.cpp
   )

ADD_LIBRARY(dataeditor STATIC ${dataeditor_STAT_SRCS})
TARGET_LINK_LIBRARIES(dataeditor Qt5::Widgets KF5::WidgetsAddons KF5::Parts)
