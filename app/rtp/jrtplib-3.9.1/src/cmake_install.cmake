# Install script for directory: /data/local/lyin/workspace/rtp/jrtplib-3.9.1/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jrtplib3" TYPE FILE FILES
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpapppacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpbyepacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpcompoundpacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpcompoundpacketbuilder.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcppacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcppacketbuilder.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcprrpacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpscheduler.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpsdesinfo.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpsdespacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpsrpacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtcpunknownpacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpaddress.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpcollisionlist.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpconfig.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpdebug.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpdefines.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtperrors.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtphashtable.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpinternalsourcedata.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpipv4address.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpipv4destination.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpipv6address.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpipv6destination.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpkeyhashtable.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtplibraryversion.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpmemorymanager.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpmemoryobject.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtppacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtppacketbuilder.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtppollthread.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtprandom.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtprandomrand48.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtprandomrands.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtprandomurandom.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtprawpacket.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpsession.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpsessionparams.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpsessionsources.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpsourcedata.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpsources.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpstructs.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtptimeutilities.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtptransmitter.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtptypes_win.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtptypes.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpudpv4transmitter.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpudpv6transmitter.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpbyteaddress.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/rtpexternaltransmitter.h"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/extratransmitters/rtpfaketransmitter.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/libjrtp.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1"
         RPATH "")
  ENDIF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1")
  FILE(INSTALL DESTINATION "/usr/local/lib" TYPE SHARED_LIBRARY FILES
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/libjrtp.so.3.9.1"
    "/data/local/lyin/workspace/rtp/jrtplib-3.9.1/src/libjrtp.so"
    )
  IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.so.3.9.1")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

