// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include "RsSimpleRTPSink.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <compression/CompressionFactory.h>

RsSimpleRTPSink *
RsSimpleRTPSink::createNew(UsageEnvironment &t_env, Groupsock *t_RTPgs,
                           unsigned char t_rtpPayloadFormat,
                           unsigned t_rtpTimestampFrequency,
                           char const *t_sdpMediaTypeString,
                           char const *t_rtpPayloadFormatName,
                           rs2::video_stream_profile &t_videoStream,
                           // TODO Michal: this is a W/A for passing the sensor's metadata
                           rs2::device &t_device,
                           unsigned t_numChannels,
                           Boolean t_allowMultipleFramesPerPacket,
                           Boolean t_doNormalMBitRule)
{
  CompressionFactory::getIsEnabled() = IS_COMPRESSION_ENABLED;
  return new RsSimpleRTPSink(t_env, t_RTPgs, t_rtpPayloadFormat, t_rtpTimestampFrequency, t_sdpMediaTypeString, t_rtpPayloadFormatName,
                             t_videoStream, t_device, t_numChannels, t_allowMultipleFramesPerPacket, t_doNormalMBitRule);
}

// TODO Michal: oveload with other types if needed
std::string getSdpLineForField(const char* t_name, int t_val)
{
  std::ostringstream oss;
  oss << t_name << "=" << t_val << ";";
  return oss.str();
}

std::string getSdpLineForField(const char* t_name, const char* t_val)
{
  std::ostringstream oss;
  oss << t_name << "=" << t_val << ";";
  return oss.str();
}

std::string getSdpLineForVideoStream(rs2::video_stream_profile &t_videoStream, rs2::device &t_device)
{
  std::string str;
  str.append(getSdpLineForField("width", t_videoStream.width()));
  str.append(getSdpLineForField("height", t_videoStream.height()));
  str.append(getSdpLineForField("format", t_videoStream.format()));
  str.append(getSdpLineForField("uid", t_videoStream.unique_id()));
  str.append(getSdpLineForField("fps", t_videoStream.fps()));
  str.append(getSdpLineForField("stream_index", t_videoStream.stream_index()));
  str.append(getSdpLineForField("stream_type", t_videoStream.stream_type()));
  str.append(getSdpLineForField("bpp", RsSensor::getStreamProfileBpp(t_videoStream.format())));
  str.append(getSdpLineForField("cam_serial_num", t_device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
  str.append(getSdpLineForField("usb_type", t_device.get_info(RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR)));
  str.append(getSdpLineForField("compression", CompressionFactory::getIsEnabled()));

  // TODO: get all intrinsics as one data object
  str.append(getSdpLineForField("ppx", t_videoStream.get_intrinsics().ppx));
  str.append(getSdpLineForField("ppy", t_videoStream.get_intrinsics().ppy));
  str.append(getSdpLineForField("fx", t_videoStream.get_intrinsics().fx));
  str.append(getSdpLineForField("fy", t_videoStream.get_intrinsics().fy));
  str.append(getSdpLineForField("model", t_videoStream.get_intrinsics().model));
  
  // TODO: adjust serialization to camera distortion model
  for (size_t i = 0; i < 5; i++)
  {
    str.append(getSdpLineForField("coeff_"+i, t_videoStream.get_intrinsics().coeffs[i]));
  }

  std::string name = t_device.get_info(RS2_CAMERA_INFO_NAME);
  // We don't want to sent spaces over SDP
  // TODO Michal: Decide what character to use for replacing spaces
  std::replace(name.begin(), name.end(), ' ', '^');
  str.append(getSdpLineForField("cam_name", name.c_str()));

  return str;
}


RsSimpleRTPSink ::RsSimpleRTPSink(UsageEnvironment &t_env, Groupsock *t_RTPgs,
                                  unsigned char t_rtpPayloadFormat,
                                  unsigned t_rtpTimestampFrequency,
                                  char const *t_sdpMediaTypeString,
                                  char const *t_rtpPayloadFormatName,
                                  rs2::video_stream_profile &t_videoStream,
                                  rs2::device &t_device,
                                  unsigned t_numChannels,
                                  Boolean t_allowMultipleFramesPerPacket,
                                  Boolean t_doNormalMBitRule)
    : SimpleRTPSink(t_env, t_RTPgs, t_rtpPayloadFormat, t_rtpTimestampFrequency, t_sdpMediaTypeString, t_rtpPayloadFormatName,
                    t_numChannels, t_allowMultipleFramesPerPacket, t_doNormalMBitRule)
{
  t_env << "RsSimpleVideoRTPSink constructor\n";
  // Then use this 'config' string to construct our "a=fmtp:" SDP line:
  unsigned fmtpSDPLineMaxSize = 400; // 400 => extended for intrinsics
  m_fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
  std::string sdpStr =  getSdpLineForVideoStream(t_videoStream, t_device);
  sprintf(m_fFmtpSDPLine, "a=fmtp:%d;%s\r\n",
          rtpPayloadType(),
          sdpStr.c_str());
}

char const *RsSimpleRTPSink::auxSDPLine()
{
  return m_fFmtpSDPLine;
}
