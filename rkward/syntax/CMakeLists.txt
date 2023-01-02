# rkward.xml is also installed as a qresource via the parent CMakeLists.txt, but will not be found, there, before KF5 5.86.0
IF(${KF5_VERSION} VERSION_LESS "5.86.0")
    INSTALL(FILES rkward.xml DESTINATION ${DATA_INSTALL_DIR}/org.kde.syntax-highlighting/syntax/)
ENDIF()

