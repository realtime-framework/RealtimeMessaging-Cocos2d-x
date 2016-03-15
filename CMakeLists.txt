
add_library(ortc SHARED 
					${CMAKE_CURRENT_SOURCE_DIR}/libortc.h 
					${CMAKE_CURRENT_SOURCE_DIR}/ortc.c 
					${CMAKE_CURRENT_SOURCE_DIR}/connection.h
					${CMAKE_CURRENT_SOURCE_DIR}/connection.c 
					${CMAKE_CURRENT_SOURCE_DIR}/message.h 
					${CMAKE_CURRENT_SOURCE_DIR}/message.c 
					${CMAKE_CURRENT_SOURCE_DIR}/channel.h 
					${CMAKE_CURRENT_SOURCE_DIR}/channel.c 
					${CMAKE_CURRENT_SOURCE_DIR}/balancer.h 
					${CMAKE_CURRENT_SOURCE_DIR}/balancer.c 
					${CMAKE_CURRENT_SOURCE_DIR}/authentication.h 
					${CMAKE_CURRENT_SOURCE_DIR}/authentication.c 
					${CMAKE_CURRENT_SOURCE_DIR}/presence.h 
					${CMAKE_CURRENT_SOURCE_DIR}/presence.c 
					${CMAKE_CURRENT_SOURCE_DIR}/dlist.h 
					${CMAKE_CURRENT_SOURCE_DIR}/dlist.c 
					${CMAKE_CURRENT_SOURCE_DIR}/common.h 
					${CMAKE_CURRENT_SOURCE_DIR}/common.c 
					${CMAKE_CURRENT_SOURCE_DIR}/slre/slre.h
					${CMAKE_CURRENT_SOURCE_DIR}/slre/slre.c 
					${CMAKE_CURRENT_SOURCE_DIR}/loops.h 
					${CMAKE_CURRENT_SOURCE_DIR}/loops.c
					${CMAKE_CURRENT_SOURCE_DIR}/events.h 
					${CMAKE_CURRENT_SOURCE_DIR}/events.c 
				)

add_library(ortc_static STATIC 
					${CMAKE_CURRENT_SOURCE_DIR}/libortc.h 
					${CMAKE_CURRENT_SOURCE_DIR}/ortc.c 
					${CMAKE_CURRENT_SOURCE_DIR}/connection.h
					${CMAKE_CURRENT_SOURCE_DIR}/connection.c 
					${CMAKE_CURRENT_SOURCE_DIR}/message.h 
					${CMAKE_CURRENT_SOURCE_DIR}/message.c 
					${CMAKE_CURRENT_SOURCE_DIR}/channel.h 
					${CMAKE_CURRENT_SOURCE_DIR}/channel.c 
					${CMAKE_CURRENT_SOURCE_DIR}/balancer.h 
					${CMAKE_CURRENT_SOURCE_DIR}/balancer.c 
					${CMAKE_CURRENT_SOURCE_DIR}/authentication.h 
					${CMAKE_CURRENT_SOURCE_DIR}/authentication.c 
					${CMAKE_CURRENT_SOURCE_DIR}/presence.h 
					${CMAKE_CURRENT_SOURCE_DIR}/presence.c 
					${CMAKE_CURRENT_SOURCE_DIR}/dlist.h 
					${CMAKE_CURRENT_SOURCE_DIR}/dlist.c 
					${CMAKE_CURRENT_SOURCE_DIR}/common.h 
					${CMAKE_CURRENT_SOURCE_DIR}/common.c 
					${CMAKE_CURRENT_SOURCE_DIR}/slre/slre.h
					${CMAKE_CURRENT_SOURCE_DIR}/slre/slre.c 
					${CMAKE_CURRENT_SOURCE_DIR}/loops.h 
					${CMAKE_CURRENT_SOURCE_DIR}/loops.c 
					${CMAKE_CURRENT_SOURCE_DIR}/events.h 
					${CMAKE_CURRENT_SOURCE_DIR}/events.c 
				)


file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/libortc.h" DESTINATION "${CMAKE_BINARY_DIR}/")	


target_link_libraries(ortc ${CURL_LIBRARIES} ${LWS_LIBRARIES})
target_link_libraries(ortc_static ${CURL_LIBRARIES} ${LWS_LIBRARIES})

if(NOT CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(ortc ${PTHREAD_LIB})
  target_link_libraries(ortc_static ${PTHREAD_LIB})
else()
  target_link_libraries(ortc ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(ortc_static ${CMAKE_THREAD_LIBS_INIT})
endif()

if(NOT UNIX)
  target_link_libraries(ortc Ws2_32)
  target_link_libraries(ortc_static Ws2_32)
endif()

