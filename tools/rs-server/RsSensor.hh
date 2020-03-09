#ifndef _RS_SENSOR_HH
#define _RS_SENSOR_HH

#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_types.hpp>
#include <unordered_map>
#include <chrono>
#include "compression/ICompression.h"
#include <ipDeviceCommon/MemoryPool.h>

typedef struct RsOption //todo use the client struct 
{
  rs2_option m_opt;
  rs2::option_range m_range;
} RsOption;


class RsSensor
{
public:
	RsSensor(rs2::sensor t_sensor, rs2::device t_device);
	int open(std::unordered_map<long long int, rs2::frame_queue> &t_streamProfilesQueues);
	int start(std::unordered_map<long long int, rs2::frame_queue> &t_streamProfilesQueues);
	int close();
	int stop();
	rs2::sensor &getRsSensor() { return m_sensor; }
	std::unordered_map<long long int, rs2::video_stream_profile> getStreamProfiles() { return m_streamProfiles; }
	long long int getStreamProfileKey(rs2::stream_profile t_profile);
	std::string getSensorName();
	static int getStreamProfileBpp(rs2_format t_format);
	rs2::device getDevice() { return m_device; }
	std::vector<RsOption> gerSupportedOptions();

private:
	rs2::sensor m_sensor;
	std::unordered_map<long long int, rs2::video_stream_profile> m_streamProfiles;
	std::unordered_map<long long int, std::shared_ptr<ICompression>> m_iCompress;
	rs2::device m_device;
	MemoryPool *m_memPool;
	std::unordered_map<long long int,std::chrono::high_resolution_clock::time_point> m_prevSample;
};

#endif
