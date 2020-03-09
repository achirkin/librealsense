// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <liveMedia.hh>

#ifndef _RS_SERVER_MEDIA_SUBSESSION_HH
#define _RS_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "RsSource.hh"

class RsServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
   static RsServerMediaSubsession *createNew(UsageEnvironment &t_env, rs2::video_stream_profile &t_video_stream_profile);
   rs2::frame_queue &getFrameQueue();
   rs2::video_stream_profile getStreamProfile();

protected:
   RsServerMediaSubsession(UsageEnvironment &t_env, rs2::video_stream_profile &t_video_stream_profile); 
   virtual ~RsServerMediaSubsession();
   virtual FramedSource *createNewStreamSource(unsigned t_clientSessionId,
                                               unsigned &t_estBitrate);
   virtual RTPSink *createNewRTPSink(Groupsock *t_rtpGroupsock,
                                     unsigned char t_rtpPayloadTypeIfDynamic,
                                     FramedSource *t_inputSource);

private:
   rs2::video_stream_profile m_videoStreamProfile;
   rs2::frame_queue m_frameQueue;
   int m_pixelSize;
};
#endif //_RS_SERVER_MEDIA_SUBSESSION_HH
