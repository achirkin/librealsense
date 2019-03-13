﻿using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;

namespace Intel.RealSense
{
    public class DisparityTransform : ProcessingBlock
    {
        static IntPtr Create(bool transform_to_disparity)
        {
            object error;
            byte transform_direction = transform_to_disparity ? (byte)1 : (byte)0;
            return NativeMethods.rs2_create_disparity_transform_block(transform_direction, out error);
        }

        public DisparityTransform(bool transform_to_disparity = true) : base(Create(transform_to_disparity))
        {
            object error;
            NativeMethods.rs2_start_processing_queue(Handle, queue.Handle, out error);
        }

        [Obsolete("This method is obsolete. Use Process method instead")]
        public VideoFrame ApplyFilter(Frame original, FramesReleaser releaser = null)
        {
            return Process(original).As<VideoFrame>().DisposeWith(releaser);
        }
    }
}