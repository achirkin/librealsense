// Generated by gencpp from file realsense_msgs/ImuIntrinsic.msg
// DO NOT EDIT!


#ifndef REALSENSE_MSGS_MESSAGE_IMUINTRINSIC_H
#define REALSENSE_MSGS_MESSAGE_IMUINTRINSIC_H


#include <string>
#include <vector>
#include <map>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace realsense_msgs
{
template <class ContainerAllocator>
struct ImuIntrinsic_
{
  typedef ImuIntrinsic_<ContainerAllocator> Type;

  ImuIntrinsic_()
    : data()
    , noise_variances()
    , bias_variances()  {
      data.fill(0.0);

      noise_variances.fill(0.0);

      bias_variances.fill(0.0);
  }
  ImuIntrinsic_(const ContainerAllocator& _alloc)
    : data()
    , noise_variances()
    , bias_variances()  {
  (void)_alloc;
      data.fill(0.0);

      noise_variances.fill(0.0);

      bias_variances.fill(0.0);
  }



   typedef std::array<float, 12>  _data_type;
  _data_type data;

   typedef std::array<float, 3>  _noise_variances_type;
  _noise_variances_type noise_variances;

   typedef std::array<float, 3>  _bias_variances_type;
  _bias_variances_type bias_variances;




  typedef std::shared_ptr< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> > Ptr;
  typedef std::shared_ptr< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> const> ConstPtr;

}; // struct ImuIntrinsic_

typedef ::realsense_msgs::ImuIntrinsic_<std::allocator<void> > ImuIntrinsic;

typedef std::shared_ptr< ::realsense_msgs::ImuIntrinsic > ImuIntrinsicPtr;
typedef std::shared_ptr< ::realsense_msgs::ImuIntrinsic const> ImuIntrinsicConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> & v)
{
rs2rosinternal::message_operations::Printer< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace realsense_msgs

namespace rs2rosinternal
{
namespace message_traits
{



// BOOLTRAITS {'IsFixedSize': True, 'IsMessage': True, 'HasHeader': False}
// {'sensor_msgs': ['/opt/ros/kinetic/share/sensor_msgs/cmake/../msg'], 'geometry_msgs': ['/opt/ros/kinetic/share/geometry_msgs/cmake/../msg'], 'std_msgs': ['/opt/ros/kinetic/share/std_msgs/cmake/../msg'], 'realsense_msgs': ['/home/zivs/dev/realsense_msg_generator/msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__doc__', '__eq__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsFixedSize< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
{
  static const char* value()
  {
    return "aebdc2f8f9726f1c3ca823ab56e47429";
  }

  static const char* value(const ::realsense_msgs::ImuIntrinsic_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0xaebdc2f8f9726f1cULL;
  static const uint64_t static_value2 = 0x3ca823ab56e47429ULL;
};

template<class ContainerAllocator>
struct DataType< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
{
  static const char* value()
  {
    return "realsense_msgs/ImuIntrinsic";
  }

  static const char* value(const ::realsense_msgs::ImuIntrinsic_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
{
  static const char* value()
  {
    return "# Motion device intrinsics: scale, bias, and variances\n\
\n\
# Interpret data array values\n\
                            # [ Scale X       cross axis        cross axis      Bias X]\n\
                            # [cross axis      Scale Y          cross axis      Bias Y]\n\
                            # [cross axis     cross axis          Scale Z       Bias Z]\n\
float32[12] data            # 3x4 Row-major matrix\n\
\n\
float32[3] noise_variances  # Variance of noise for X, Y, and Z axis\n\
float32[3] bias_variances   # Variance of bias for X, Y, and Z axis\n\
";
  }

  static const char* value(const ::realsense_msgs::ImuIntrinsic_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace rs2rosinternal

namespace rs2rosinternal
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.data);
      stream.next(m.noise_variances);
      stream.next(m.bias_variances);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct ImuIntrinsic_

} // namespace serialization
} // namespace rs2rosinternal

namespace rs2rosinternal
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::realsense_msgs::ImuIntrinsic_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::realsense_msgs::ImuIntrinsic_<ContainerAllocator>& v)
  {
    s << indent << "data[]" << std::endl;
    for (size_t i = 0; i < v.data.size(); ++i)
    {
      s << indent << "  data[" << i << "]: ";
      Printer<float>::stream(s, indent + "  ", v.data[i]);
    }
    s << indent << "noise_variances[]" << std::endl;
    for (size_t i = 0; i < v.noise_variances.size(); ++i)
    {
      s << indent << "  noise_variances[" << i << "]: ";
      Printer<float>::stream(s, indent + "  ", v.noise_variances[i]);
    }
    s << indent << "bias_variances[]" << std::endl;
    for (size_t i = 0; i < v.bias_variances.size(); ++i)
    {
      s << indent << "  bias_variances[" << i << "]: ";
      Printer<float>::stream(s, indent + "  ", v.bias_variances[i]);
    }
  }
};

} // namespace message_operations
} // namespace rs2rosinternal

#endif // REALSENSE_MSGS_MESSAGE_IMUINTRINSIC_H
